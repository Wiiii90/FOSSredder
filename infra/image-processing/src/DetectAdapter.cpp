/**
 * @file infra/image-processing/src/DetectAdapter.cpp
 * @brief Implements OpenCV text and table detection for the import workflow.
 */

#include "image-processing/pch.h"
#include "image-processing/DetectAdapter.h"
#include "image-processing/MaskAdapter.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace opencv {
namespace ports = core::ports::image_processing;
namespace {

void writeImageViaDebugger(std::shared_ptr<IDebugger> debugger, const std::string& relPath, const cv::Mat& img) {
    if (!debugger) return;
    std::vector<uint8_t> buf;
    try {
        if (img.empty()) return;
        cv::Mat out;
        if (img.type() == CV_8UC1) cv::cvtColor(img, out, cv::COLOR_GRAY2BGR);
        else if (img.type() == CV_8UC3) out = img;
        else { img.convertTo(out, CV_8U, 255.0); if (out.channels() == 1) cv::cvtColor(out, out, cv::COLOR_GRAY2BGR); }
        bool ok = cv::imencode(".png", out, buf);
        if (!ok || buf.empty()) return;
        debugger->writeBytes(relPath + ".png", buf);
    } catch (...) {
        debugger->writeText("opencv/error.txt", std::string("writeImageViaDebugger failed for ") + relPath);
    }
}

static cv::Mat ensureGrayLocal(const cv::Mat& in) {
    if (in.empty()) return in;
    if (in.type() == CV_8UC1) return in.clone();
    cv::Mat out;
    if (in.channels() == 4) cv::cvtColor(in, out, cv::COLOR_BGRA2GRAY);
    else if (in.channels() >= 3) cv::cvtColor(in, out, cv::COLOR_BGR2GRAY);
    else in.convertTo(out, CV_8U);
    return out;
}

std::vector<cv::Rect> findTextBlocksUsingMorphology(const cv::Mat& gray) {
    cv::Mat normalizedGray = ensureGrayLocal(gray);
    cv::Mat bin;
    cv::adaptiveThreshold(normalizedGray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 15, 10);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 3));
    cv::Mat morph;
    cv::morphologyEx(bin, morph, cv::MORPH_CLOSE, kernel);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(morph, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Rect> blocks;
    for (const auto& contour : contours) {
        cv::Rect rect = cv::boundingRect(contour);
        if (rect.width > 50 && rect.height > 10) blocks.push_back(rect);
    }
    return blocks;
}

static int snapYToEdge(const cv::Mat& gray, int y, int window) {
    if (gray.empty()) return y;
    int y0 = std::max(1, y - window);
    int y1 = std::min(gray.rows - 2, y + window);
    int bestY = y;
    int bestScore = -1;
    for (int yy = y0; yy <= y1; ++yy) {
        const uint8_t* a = gray.ptr<uint8_t>(yy - 1);
        const uint8_t* b = gray.ptr<uint8_t>(yy + 1);
        int score = 0;
        for (int x = 0; x < gray.cols; ++x) score += std::abs(int(b[x]) - int(a[x]));
        if (score > bestScore) { bestScore = score; bestY = yy; }
    }
    return bestY;
}

static int snapXToEdge(const cv::Mat& gray, int x, int window) {
    if (gray.empty()) return x;
    int x0 = std::max(1, x - window);
    int x1 = std::min(gray.cols - 2, x + window);
    int bestX = x;
    int bestScore = -1;
    for (int xx = x0; xx <= x1; ++xx) {
        int score = 0;
        for (int y = 0; y < gray.rows; ++y) score += std::abs(int(gray.at<uint8_t>(y, xx + 1)) - int(gray.at<uint8_t>(y, xx - 1)));
        if (score > bestScore) { bestScore = score; bestX = xx; }
    }
    return bestX;
}

static int edgeScoreY(const cv::Mat& gray, int y) {
    if (gray.empty() || y <= 0 || y >= gray.rows - 1) return 0;
    const uint8_t* a = gray.ptr<uint8_t>(y - 1);
    const uint8_t* b = gray.ptr<uint8_t>(y + 1);
    int score = 0;
    for (int x = 0; x < gray.cols; ++x) score += std::abs(int(b[x]) - int(a[x]));
    return score;
}

static int edgeScoreX(const cv::Mat& gray, int x) {
    if (gray.empty() || x <= 0 || x >= gray.cols - 1) return 0;
    int score = 0;
    for (int y = 0; y < gray.rows; ++y) score += std::abs(int(gray.at<uint8_t>(y, x + 1)) - int(gray.at<uint8_t>(y, x - 1)));
    return score;
}

static std::vector<int> dedupeByScore(const std::vector<int>& in, int minDist, const cv::Mat& gray, bool horizontal) {
    if (in.empty()) return {};
    std::vector<int> s = in;
    std::sort(s.begin(), s.end());

    std::vector<int> out;
    int best = s[0];
    int bestScore = horizontal ? edgeScoreY(gray, best) : edgeScoreX(gray, best);

    for (size_t i = 1; i < s.size(); ++i) {
        if (std::abs(s[i] - best) < minDist) {
            int sc = horizontal ? edgeScoreY(gray, s[i]) : edgeScoreX(gray, s[i]);
            if (sc > bestScore) { best = s[i]; bestScore = sc; }
        } else {
            out.push_back(best);
            best = s[i];
            bestScore = horizontal ? edgeScoreY(gray, best) : edgeScoreX(gray, best);
        }
    }
    out.push_back(best);
    return out;
}

static ports::Rect bboxFromCells(const std::vector<ports::Cell>& cells, const cv::Rect& fallback) {
    ports::Rect out;
    out.x = fallback.x; out.y = fallback.y; out.width = fallback.width; out.height = fallback.height;
    if (cells.empty()) return out;

    int minx = INT_MAX, miny = INT_MAX, maxx = 0, maxy = 0;
    for (const auto& c : cells) {
        minx = std::min(minx, c.bbox.x);
        miny = std::min(miny, c.bbox.y);
        maxx = std::max(maxx, c.bbox.x + c.bbox.width);
        maxy = std::max(maxy, c.bbox.y + c.bbox.height);
    }

    if (minx < maxx && miny < maxy) {
        out.x = minx;
        out.y = miny;
        out.width = maxx - minx;
        out.height = maxy - miny;
    }

    return out;
}

struct LineSeg {
    bool horizontal = true;
    int a = 0;
    int b0 = 0;
    int b1 = 0;
    int len = 0;
};

static std::vector<LineSeg> extractLineSegsFromMask(const cv::Mat& mask, bool horizontal, int minLen, int maxThickness) {
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<LineSeg> segs;
    segs.reserve(contours.size());

    for (const auto& c : contours) {
        cv::Rect r = cv::boundingRect(c);
        if (horizontal) {
            if (r.width >= minLen && r.height <= maxThickness)
                segs.push_back({ true, r.y + r.height / 2, r.x, r.x + r.width, r.width });
        } else {
            if (r.height >= minLen && r.width <= maxThickness)
                segs.push_back({ false, r.x + r.width / 2, r.y, r.y + r.height, r.height });
        }
    }

    return segs;
}

static std::vector<int> collectAxisFromSegs(const std::vector<LineSeg>& segs) {
    std::vector<int> out;
    out.reserve(segs.size());
    for (const auto& s : segs) out.push_back(s.a);
    return out;
}

static bool overlaps(int a0, int a1, int b0, int b1, int minOverlap) {
    int lo = std::max(a0, b0);
    int hi = std::min(a1, b1);
    return (hi - lo) >= minOverlap;
}

static bool isAnchoredToBorder(int v0, int border, int tol) {
    return std::abs(v0 - border) <= tol;
}

static int nearestValue(const std::vector<int>& s, int v) {
    if (s.empty()) return v;
    auto it = std::lower_bound(s.begin(), s.end(), v);
    if (it == s.begin()) return *it;
    if (it == s.end()) return s.back();
    int a = *(it - 1);
    int b = *it;
    return (std::abs(v - a) <= std::abs(v - b)) ? a : b;
}

static std::vector<int> completeAxisLinesFromSegments(
    const std::vector<int>& primaryLines,
    const std::vector<LineSeg>& partialSegs,
    const std::vector<int>& orthogonalLines,
    int primaryMinDist,
    int orthoSnapTol,
    int borderTol,
    int minPartialLen,
    const cv::Mat& gray,
    bool horizontal) {

    std::vector<int> out = primaryLines;

    if (partialSegs.empty() || orthogonalLines.size() < 2) {
        std::sort(out.begin(), out.end());
        out = dedupeByScore(out, primaryMinDist, gray, horizontal);
        std::sort(out.begin(), out.end());
        return out;
    }

    const int borderStart = 0;

    for (const auto& seg : partialSegs) {
        if (seg.len < minPartialLen) continue;
        if (!isAnchoredToBorder(seg.b0, borderStart, borderTol)) continue;

        int snappedPrimary = seg.a;
        snappedPrimary = horizontal ? snapYToEdge(gray, snappedPrimary, 4) : snapXToEdge(gray, snappedPrimary, 4);

        int snappedEnd = nearestValue(orthogonalLines, seg.b1);
        if (std::abs(snappedEnd - seg.b1) > orthoSnapTol) continue;

        bool coversInterval = false;
        for (size_t i = 0; i + 1 < orthogonalLines.size(); ++i) {
            int o0 = orthogonalLines[i];
            int o1 = orthogonalLines[i + 1];
            if (o1 <= 0) continue;
            if (overlaps(seg.b0, snappedEnd, o0, o1, std::max(12, (o1 - o0) / 2))) { coversInterval = true; break; }
        }
        if (!coversInterval) continue;

        out.push_back(snappedPrimary);
    }

    std::sort(out.begin(), out.end());
    out = dedupeByScore(out, primaryMinDist, gray, horizontal);
    std::sort(out.begin(), out.end());

    return out;
}

ports::Table detectTableGridHough(const cv::Mat& roiGrayIn, const cv::Rect& roiOffset, const std::vector<cv::Rect>&, int& outCells, double& outQuality, const std::string&, std::shared_ptr<IDebugger> debugger) {
    ports::Table table;
    outCells = 0; outQuality = 0.0;
    if (roiGrayIn.empty()) return table;

    cv::Mat roiGray = ensureGrayLocal(roiGrayIn);

    cv::Mat hMask = MaskAdapter::makeLineMask(roiGray, true);
    cv::Mat vMask = MaskAdapter::makeLineMask(roiGray, false);

    const int minHorizW = std::max((roiGray.cols * 8) / 10, 160);
    const int minVertH = std::max((roiGray.rows * 5) / 10, 100);
    const int maxThickness = 12;

    auto hFullSegs = extractLineSegsFromMask(hMask, true, minHorizW, maxThickness);
    auto vFullSegs = extractLineSegsFromMask(vMask, false, minVertH, maxThickness);

    auto yLines = collectAxisFromSegs(hFullSegs);
    auto xLines = collectAxisFromSegs(vFullSegs);

    auto hPartialSegs = extractLineSegsFromMask(hMask, true, std::max(roiGray.cols / 5, 80), maxThickness);
    auto vPartialSegs = extractLineSegsFromMask(vMask, false, std::max(roiGray.rows / 6, 60), maxThickness);

    for (auto& x : xLines) x = snapXToEdge(roiGray, x, 4);
    for (auto& y : yLines) y = snapYToEdge(roiGray, y, 4);

    const int xMinDist = std::max(4, roiGray.cols / 120);
    const int yMinDist = std::max(4, roiGray.rows / 200);

    xLines = dedupeByScore(xLines, xMinDist, roiGray, false);
    yLines = dedupeByScore(yLines, yMinDist, roiGray, true);

    std::sort(xLines.begin(), xLines.end());
    std::sort(yLines.begin(), yLines.end());

    if (!xLines.empty()) {
        xLines.push_back(0);
        xLines.push_back(roiGray.cols - 1);
    }
    if (!yLines.empty()) {
        yLines.push_back(0);
        yLines.push_back(roiGray.rows - 1);
    }

    std::sort(xLines.begin(), xLines.end());
    std::sort(yLines.begin(), yLines.end());

    xLines = dedupeByScore(xLines, xMinDist, roiGray, false);
    yLines = dedupeByScore(yLines, yMinDist, roiGray, true);

    std::sort(xLines.begin(), xLines.end());
    std::sort(yLines.begin(), yLines.end());

    yLines = completeAxisLinesFromSegments(
        yLines,
        hPartialSegs,
        xLines,
        yMinDist,
        std::max(12, roiGray.cols / 50),
        std::max(12, roiGray.cols / 50),
        std::max(40, roiGray.cols / 10),
        roiGray,
        true);

    xLines = completeAxisLinesFromSegments(
        xLines,
        vPartialSegs,
        yLines,
        xMinDist,
        std::max(12, roiGray.rows / 50),
        std::max(12, roiGray.rows / 50),
        std::max(40, roiGray.rows / 10),
        roiGray,
        false);

    if (xLines.size() < 2 || yLines.size() < 2) {
        outQuality = 0.0;
        return table;
    }

    for (size_t r = 0; r + 1 < yLines.size(); ++r) {
        for (size_t c = 0; c + 1 < xLines.size(); ++c) {
            int x0 = xLines[c], x1 = xLines[c + 1];
            int y0 = yLines[r], y1 = yLines[r + 1];
            int w = x1 - x0, h = y1 - y0;
            if (w <= 6 || h <= 6) continue;
            cv::Rect crel(x0, y0, w, h);
            crel &= cv::Rect(0, 0, roiGray.cols, roiGray.rows);
            if (crel.width <= 0 || crel.height <= 0) continue;
            ports::Cell cell;
            cell.bbox.x = crel.x + roiOffset.x; cell.bbox.y = crel.y + roiOffset.y; cell.bbox.width = crel.width; cell.bbox.height = crel.height;
            cell.row = static_cast<int>(r);
            cell.col = static_cast<int>(c);
            table.cells.push_back(cell);
        }
    }

    outCells = static_cast<int>(table.cells.size());
    if (outCells == 0) { outQuality = 0.0; return table; }

    double avgW = 0.0, avgH = 0.0;
    for (const auto& c : table.cells) { avgW += c.bbox.width; avgH += c.bbox.height; }
    avgW /= table.cells.size(); avgH /= table.cells.size();
    double sizeScore = std::clamp((avgW * avgH) / double(roiGray.cols * roiGray.rows), 0.0, 1.0);
    double lineScore = std::clamp((double)(xLines.size() + yLines.size()) / 12.0, 0.0, 1.0);
    outQuality = 0.6 * lineScore + 0.4 * sizeScore;

    table.bbox = bboxFromCells(table.cells, roiOffset);
    if (!table.cells.empty()) {
        table.cols = static_cast<int>(xLines.size() > 0 ? xLines.size() - 1 : 0);
        table.rows = static_cast<int>(yLines.size() > 0 ? yLines.size() - 1 : 0);
    }

    return table;
}

} // anonymous namespace

// Expose DetectTextBlocks implementation
std::vector<cv::Rect> DetectAdapter::detectTextBlocks(const cv::Mat& img, std::shared_ptr<IDebugger> debugger) {
    return findTextBlocksUsingMorphology(img);
}

std::vector<ports::Table> DetectAdapter::detectTables(const cv::Mat& img, const std::string& imagePath, std::shared_ptr<IDebugger> debugger) {
    std::vector<ports::Table> result;
    if (img.empty()) {
        if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "Empty image: " + imagePath);
        return result;
    }

    auto blocks = findTextBlocksUsingMorphology(img);

    struct Candidate { ports::Table t; int cells; double q; };
    std::vector<Candidate> candidates;

    for (const auto& b : blocks) {
        if (b.width < img.cols * 0.12) continue;
        cv::Mat roi = img(b);
        std::vector<cv::Rect> emptyBoxes;

        int cells = 0; double q = 0.0;
        auto tab = detectTableGridHough(roi, cv::Rect(b.x, b.y, b.width, b.height), emptyBoxes, cells, q, std::string(), debugger);
        if (cells > 0 && q > 0.05) candidates.push_back({ tab, cells, q });
    }

    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        if (a.cells != b.cells) return a.cells > b.cells;
        return a.q > b.q;
    });

    if (!candidates.empty()) result.push_back(candidates[0].t);

    if (debugger && debugger->enabled()) {
        cv::Mat vis;
        if (img.channels() == 1) cv::cvtColor(img, vis, cv::COLOR_GRAY2BGR);
        else vis = img.clone();

        for (const auto &t : result)
            cv::rectangle(vis, cv::Rect(t.bbox.x, t.bbox.y, t.bbox.width, t.bbox.height), cv::Scalar(0,255,0), 2);

        if (!result.empty()) {
            const auto &t = result[0];
            for (const auto &c : t.cells)
                cv::rectangle(vis, cv::Rect(c.bbox.x, c.bbox.y, c.bbox.width, c.bbox.height), cv::Scalar(255,0,0), 2);
        }

        writeImageViaDebugger(debugger, "opencv/detect_table_with_cells", vis);
    }

    return result;
}

} // namespace opencv
