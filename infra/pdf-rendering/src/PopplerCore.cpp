/**
 * @file infra/pdf-rendering/src/PopplerCore.cpp
 * @brief Implements Poppler-based PDF rendering and metadata extraction.
 */

#include "pdf-rendering/pch.h"
#include "pdf-rendering/PopplerCore.h"
#include "debug/IDebugger.h"
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-image.h>
#include <poppler-page-renderer.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

using json = nlohmann::json;
namespace ports = core::ports::pdf_rendering;

static void safeWriteText(const std::shared_ptr<IDebugger>& debugger, const std::string& path, const std::string& text) {
    if (!debugger || !debugger->enabled()) return;
    try {
        debugger->writeText(path, text);
    } catch (...) {
        return;
    }
}

static void safeWriteBytes(const std::shared_ptr<IDebugger>& debugger, const std::string& path, const std::vector<uint8_t>& data) {
    if (!debugger || !debugger->enabled()) return;
    try {
        debugger->writeBytes(path, data);
    } catch (...) {
        return;
    }
}

static ports::RenderedPage extractPageMeta(poppler::page* page, int pageIndex, double dpi, std::shared_ptr<IDebugger> debugger) {
    ports::RenderedPage rp;
    double pageWidthPts = 0.0, pageHeightPts = 0.0;
    try {
        auto rect = page->page_rect();
        pageWidthPts = rect.width();
        pageHeightPts = rect.height();
    } catch (...) {
        pageWidthPts = 595.2; pageHeightPts = 841.8;
    }

    rp.pageWidthPts = pageWidthPts;
    rp.pageHeightPts = pageHeightPts;
    rp.dpiX = dpi; rp.dpiY = dpi;

    try {
        auto tbl = page->text_list();
        for (const auto& tb : tbl) {
            ports::TextElement te;
            try {
                auto u = tb.text();
                auto ba = u.to_utf8();
                if (ba.size() > 0 && ba.data() != nullptr)
                    te.text.assign(reinterpret_cast<const char*>(ba.data()), ba.size());
            } catch (...) { te.text.clear(); }

            try {
                auto b = tb.bbox();
                try { te.x = b.x(); te.y = b.y(); te.width = b.width(); te.height = b.height(); }
                catch (...) {
                    try { te.x = b.left(); te.y = b.top(); te.width = b.right() - b.left(); te.height = b.bottom() - b.top(); }
                    catch (...) { te.x = te.y = te.width = te.height = 0.0; }
                }
            } catch (...) { te.x = te.y = te.width = te.height = 0.0; }

            rp.textElements.push_back(te);
        }
    } catch (...) { rp.textElements.clear(); }

    json j;
    j["dpi_x"] = dpi; j["dpi_y"] = dpi;
    j["page_width_pts"] = pageWidthPts; j["page_height_pts"] = pageHeightPts;
    double scaleX = dpi / 72.0; double scaleY = dpi / 72.0;
    json tj = json::array();
    for (const auto& te : rp.textElements) {
        json it;
        it["text"] = te.text;
        it["bbox_pts"] = { te.x, te.y, te.width, te.height };
        it["bbox_px"] = { static_cast<int>(std::round(te.x * scaleX)), static_cast<int>(std::round(te.y * scaleY)), static_cast<int>(std::round(te.width * scaleX)), static_cast<int>(std::round(te.height * scaleY)) };
        tj.push_back(it);
    }
    j["text_elements"] = tj;
    j["embedded_images"] = json::array();
    rp.metadataJson = j.dump();

    if (debugger && debugger->enabled()) {
        std::string stem = std::to_string(pageIndex + 1);
        safeWriteText(debugger, std::string("poppler/meta/") + stem + ".json", rp.metadataJson);
    }

    return rp;
}

static std::filesystem::path resolveOutputDir(const std::filesystem::path& outputDir) {
    if (outputDir.empty()) return {};
    std::filesystem::create_directories(outputDir);
    return outputDir;
}

static std::vector<uint8_t> encodePngBytes(poppler::image& img) {
    std::vector<uint8_t> out;
    if (!img.is_valid()) return out;

    const int w = img.width();
    const int h = img.height();
    if (w <= 0 || h <= 0) return out;

    const int stride = img.bytes_per_row();
    unsigned char* data = reinterpret_cast<unsigned char*>(img.data());
    if (!data) return out;

    cv::Mat rgb(h, w, CV_8UC3, data, static_cast<size_t>(stride));
    cv::Mat bgr;
    cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);

    std::vector<int> params;
    params.push_back(cv::IMWRITE_PNG_COMPRESSION);
    params.push_back(3);

    cv::imencode(".png", bgr, out, params);
    return out;
}

static std::string prefix_base(const std::string& uniqIdPrefix, const std::string& filePrefix)
{
    if (!uniqIdPrefix.empty() && !filePrefix.empty()) return uniqIdPrefix + "_" + filePrefix;
    if (!uniqIdPrefix.empty()) return uniqIdPrefix;
    if (!filePrefix.empty()) return filePrefix;
    return "poppler";
}

std::vector<ports::RenderedPage> PopplerCore::extractDocumentMeta(const std::string& pdfPath,
                                                                           double dpi,
                                                                           const std::filesystem::path& outputDir,
                                                                           const std::string& uniqIdPrefix,
                                                                           const std::string& filePrefix,
                                                                           std::shared_ptr<IDebugger> debugger,
                                                                           std::shared_ptr<std::atomic<bool>> cancelFlag) {
    std::vector<ports::RenderedPage> result;
    if (!std::filesystem::exists(pdfPath)) throw std::runtime_error("PDF file does not exist: " + pdfPath);

    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdfPath));
    if (!doc) throw std::runtime_error("Failed to open PDF: " + pdfPath);

    (void)uniqIdPrefix;
    (void)filePrefix;

    int numPages = doc->pages();
    for (int i = 0; i < numPages; ++i) {
        if (cancelFlag && cancelFlag->load()) break;
        std::unique_ptr<poppler::page> page(doc->create_page(i));
        if (!page) continue;
        result.push_back(extractPageMeta(page.get(), i, dpi, debugger));
    }
    return result;
}

std::vector<ports::RenderedPage> PopplerCore::renderDocument(const std::string& pdfPath,
                                                                      double dpi,
                                                                      const std::filesystem::path& outputDir,
                                                                      const std::string& uniqIdPrefix,
                                                                      const std::string& filePrefix,
                                                                      std::shared_ptr<IDebugger> debugger,
                                                                      std::shared_ptr<std::atomic<bool>> cancelFlag) {
    std::vector<ports::RenderedPage> result;
    if (!std::filesystem::exists(pdfPath)) throw std::runtime_error("PDF file does not exist: " + pdfPath);

    std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdfPath));
    if (!doc) throw std::runtime_error("Failed to open PDF: " + pdfPath);

    poppler::page_renderer renderer;
    renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
    renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);
    renderer.set_image_format(poppler::image::format_enum::format_rgb24);

    int numPages = doc->pages();

    std::filesystem::path outDir = resolveOutputDir(outputDir);
    const bool writeFiles = !outDir.empty();
    const std::string base = prefix_base(uniqIdPrefix, filePrefix.empty() ? std::string("poppler_render") : filePrefix);

    for (int i = 0; i < numPages; ++i) {
        if (cancelFlag && cancelFlag->load()) break;
        std::unique_ptr<poppler::page> page(doc->create_page(i));
        if (!page) continue;

        poppler::image img = renderer.render_page(page.get(), dpi, dpi);
        if (!img.is_valid()) continue;

        ports::RenderedPage rp;
        rp.dpiX = dpi;
        rp.dpiY = dpi;
        rp.imageWidthPx = img.width();
        rp.imageHeightPx = img.height();
        rp.imageBytes = encodePngBytes(img);

        if (writeFiles) {
            std::ostringstream fname;
            fname << outDir.string() << "/" << base << "_page" << (i + 1) << ".png";
            rp.imagePath = fname.str();
            if (!img.save(rp.imagePath, "png")) throw std::runtime_error("Failed to save image: " + rp.imagePath);
        }

        if (debugger && debugger->enabled() && !rp.imageBytes.empty()) {
            std::string stem = base + std::string("_page") + std::to_string(i + 1);
            safeWriteBytes(debugger, std::string("poppler/output/") + stem + std::string(".png"), rp.imageBytes);
        }

        result.push_back(std::move(rp));
    }
    return result;
}
