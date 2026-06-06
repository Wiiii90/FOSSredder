/**
 * @file infra/image-processing/src/MaskAdapter.cpp
 * @brief Implements OpenCV image masking for the import workflow.
 */

#include "image-processing/pch.h"
#include "image-processing/MaskAdapter.h"
#include "image-processing/DetectAdapter.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <sstream>

namespace opencv {
namespace ports = core::ports::image_processing;

cv::Mat MaskAdapter::makeLineMask(const cv::Mat& grayIn, bool horizontal) {
    if (grayIn.empty()) return grayIn;
    cv::Mat gray;
    if (grayIn.type() == CV_8UC1) gray = grayIn;
    else if (grayIn.channels() == 3) cv::cvtColor(grayIn, gray, cv::COLOR_BGR2GRAY);
    else grayIn.convertTo(gray, CV_8U);

    cv::Mat bin;
    try {
        cv::adaptiveThreshold(gray, bin, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 31, 10);
    } catch (...) {
        cv::threshold(gray, bin, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    }

    const int span = horizontal ? std::max(40, gray.cols / 15) : std::max(40, gray.rows / 15);
    const int thickness = 2;

    cv::Mat k = horizontal
        ? cv::getStructuringElement(cv::MORPH_RECT, cv::Size(span, thickness))
        : cv::getStructuringElement(cv::MORPH_RECT, cv::Size(thickness, span));

    cv::Mat m;
    try { cv::erode(bin, m, k); } catch (...) { m = bin.clone(); }
    try { cv::dilate(m, m, k); } catch (...) { m = m.clone(); }

    try {
        cv::Mat closeK = horizontal
            ? cv::getStructuringElement(cv::MORPH_RECT, cv::Size(std::max(5, gray.cols / 120), 1))
            : cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, std::max(5, gray.rows / 120)));
        cv::morphologyEx(m, m, cv::MORPH_CLOSE, closeK);
    } catch (...) { m = m.clone(); }

    return m;
}

ports::MaskResult MaskAdapter::mask(const ports::MaskRequest& req, std::shared_ptr<IDebugger> debugger) {
    ports::MaskResult res;
    try {
        cv::Mat img;
        std::string imgLabel;
        const auto path = std::filesystem::path(req.imagePath);
        if (!req.imageBytes.empty()) {
            img = cv::imdecode(req.imageBytes, cv::IMREAD_COLOR);
            imgLabel = std::string("<bytes>");
        }

        if (img.empty() && !req.imagePath.empty()) {
            if (!std::filesystem::exists(path)) return res;
            img = cv::imread(path.string(), cv::IMREAD_COLOR);
            imgLabel = path.string();
        }
        if (img.empty()) return res;

        const double imgArea = static_cast<double>(img.cols) * static_cast<double>(img.rows);

        std::vector<cv::Rect> boxes;
        boxes.reserve(req.textElements.size());
        for (const auto& e : req.textElements) boxes.emplace_back(e.x, e.y, e.width, e.height);

        if (req.useMorphology) {
            cv::Mat gray; cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            auto blocks = opencv::DetectAdapter::detectTextBlocks(gray, debugger);
            for (const auto &b : blocks) {
                double area = static_cast<double>(b.width) * static_cast<double>(b.height);
                if (area <= imgArea * 0.15) boxes.emplace_back(b.x, b.y, b.width, b.height);
            }
        }

        if (!req.tesseractTsv.empty()) {
            std::istringstream iss(req.tesseractTsv);
            std::string line;
            while (std::getline(iss, line)) {
                if (line.empty()) continue;
                std::istringstream ls(line);
                std::string text; int left, top, width, height; double conf;
                if (!(ls >> text >> left >> top >> width >> height >> conf)) continue;
                double area = static_cast<double>(width) * static_cast<double>(height);
                if (area <= imgArea * 0.25) boxes.emplace_back(left, top, width, height);
            }
        }

        cv::Mat mask(img.size(), CV_8UC1, cv::Scalar(0));

        // Detect tables in the page and exclude those regions from masking
        std::vector<cv::Rect> tableBboxes;
        try {
            auto tables = opencv::DetectAdapter::detectTables(img, imgLabel, debugger);
            for (const auto &t : tables) {
                if (t.bbox.width > 0 && t.bbox.height > 0) tableBboxes.emplace_back(t.bbox.x, t.bbox.y, t.bbox.width, t.bbox.height);
            }
        } catch (...) { tableBboxes.clear(); }

        // helper to check if a box center lies inside any detected table bbox
        auto isInTable = [&](const cv::Rect &r)->bool{
            const int cx = r.x + r.width/2;
            const int cy = r.y + r.height/2;
            for (const auto &tb : tableBboxes) {
                if (cx >= tb.x && cx < tb.x + tb.width && cy >= tb.y && cy < tb.y + tb.height) return true;
            }
            return false;
        };

        // pad around detected text boxes when forming mask
        // use zero pad but inset rectangles slightly to avoid touching table grid lines
        int padPx = 0;
        const int insetPx = 1; // shrink each mask rect by this amount on all sides
        for (const auto &r : boxes) {
            // skip masking text boxes that belong to detected tables
            if (isInTable(r)) continue;
            cv::Rect rr(r.x - padPx, r.y - padPx, r.width + 2 * padPx, r.height + 2 * padPx);
            // inset to avoid bridging lines
            rr.x += insetPx; rr.y += insetPx; rr.width -= 2 * insetPx; rr.height -= 2 * insetPx;
            rr &= cv::Rect(0,0,img.cols,img.rows);
            if (rr.width > 0 && rr.height > 0) cv::rectangle(mask, rr, cv::Scalar(255), cv::FILLED);
        }

        cv::Mat cleaned;
        // keep dilation minimal to avoid merging neighbouring boxes across table lines
        int dilateK = 1;
        try { cv::dilate(mask, cleaned, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(dilateK, dilateK))); } catch(...) { cleaned = mask.clone(); }

        // avoid additional morphology to keep mask minimal
        (void)cleaned;
        try { cv::threshold(cleaned, cleaned, 128, 255, cv::THRESH_BINARY); } catch(...) { cleaned = mask.clone(); }

        // Clear mask inside detected table regions to preserve table grid lines
        for (const auto &tb : tableBboxes) {
            cv::Rect rr = tb & cv::Rect(0,0,img.cols,img.rows);
            if (rr.width > 0 && rr.height > 0) cv::rectangle(cleaned, rr, cv::Scalar(0), cv::FILLED);
        }

        double nonZero = static_cast<double>(cv::countNonZero(cleaned));
        // only fallback to raw mask if a very large portion is masked
        if (nonZero > imgArea * 0.6) cleaned = mask;

        cv::Mat out;
        // Perform whiteout (fill masked areas with white) but mask is less aggressive now
        out = img.clone();
        out.setTo(cv::Scalar(255,255,255), cleaned);

        std::string base;
        if (!req.uniqIdPrefix.empty()) {
            base = req.uniqIdPrefix;
            if (!req.filePrefix.empty()) base += "_" + req.filePrefix;
        } else {
            base = req.filePrefix.empty() ? std::string("opencv_mask") : req.filePrefix;
        }

        try {
            cv::imencode(".png", cleaned, res.maskImageBytes);
        } catch (...) { res.maskImageBytes.clear(); }
        try {
            cv::imencode(".png", out, res.maskedImageBytes);
        } catch (...) { res.maskedImageBytes.clear(); }

        if (!req.outputDir.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(req.outputDir, ec);
            const auto outPath = std::filesystem::path(req.outputDir) / (base + ".png");
            if (cv::imwrite(outPath.string(), out)) {
                res.maskedImagePath = outPath;
            }
        }

        if (debugger && debugger->enabled()) {
            if (!res.maskImageBytes.empty()) debugger->writeBytes(std::string("opencv_mask_binary"), res.maskImageBytes);
            if (!res.maskedImageBytes.empty()) debugger->writeBytes(std::string("opencv_mask_whiteout"), res.maskedImageBytes);
        }
    } catch (...) {
        if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "MaskAdapter::mask failed");
    }
    return res;
}

} // namespace opencv
