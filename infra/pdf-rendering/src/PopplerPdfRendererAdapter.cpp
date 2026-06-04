/**
 * @file infra/pdf-rendering/src/PopplerPdfRendererAdapter.cpp
 * @brief Implements the Poppler adapter used by the import workflow.
 */

#include "pdf-rendering/pch.h"
#include "core/ports/infra/pdf-rendering/IPdfRenderer.h"
#include "core/ports/infra/pdf-rendering/PdfRenderingRequest.h"
#include "core/ports/infra/pdf-rendering/PdfRenderingResult.h"
#include "pdf-rendering/PopplerCore.h"
#include "debug/IDebugger.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <fstream>
#include <iterator>
#include <filesystem>
#include <sstream>

class PopplerPdfRendererAdapter : public core::ports::pdf_rendering::IPdfRenderer {
  public:
    PopplerPdfRendererAdapter(std::shared_ptr<IDebugger> dbg = nullptr) : debugger(std::move(dbg)) {}

    void writeDebugTextSafe(const std::string& path, const std::string& text) const {
        if (!debugger || !debugger->enabled()) return;
        try {
            debugger->writeText(path, text);
        } catch (...) {
            return;
        }
    }

    core::ports::pdf_rendering::RenderResult render(const core::ports::pdf_rendering::RenderRequest& req) override {
        core::ports::pdf_rendering::RenderResult out;
        try {
            writeDebugTextSafe("poppler/log", std::string("render:start ") + req.pdfPath.string());

            auto t0 = std::chrono::steady_clock::now();
            auto pages = PopplerCore::renderDocument(req.pdfPath.string(), req.dpi, req.outputDir, req.uniqIdPrefix, req.filePrefix, debugger, req.cancelFlag);
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> dur = t1 - t0;

            out.images.clear();
            out.imageBytes.clear();
            out.images.reserve(pages.size());
            out.imageBytes.reserve(pages.size());
            for (const auto& p : pages) {
                out.images.push_back(p.imagePath.empty() ? std::filesystem::path() : std::filesystem::path(p.imagePath));
                out.imageBytes.push_back(p.imageBytes);
            }

            std::ostringstream oss;
            oss << "render:done " << req.pdfPath.string() << " pages=" << out.images.size() << " duration=" << dur.count();
            writeDebugTextSafe("poppler/log", oss.str());
        } catch (const std::exception& ex) {
            writeDebugTextSafe("poppler/log", std::string("render:error ") + ex.what());
            throw;
        }
        return out;
    }

    core::ports::pdf_rendering::ExtractResult extract(const core::ports::pdf_rendering::ExtractRequest& req) override {
        core::ports::pdf_rendering::ExtractResult out;
        try {
            writeDebugTextSafe("poppler/log", std::string("extract:start ") + req.pdfPath.string());
            auto t0 = std::chrono::steady_clock::now();
            auto pages = PopplerCore::extractDocumentMeta(req.pdfPath.string(), req.dpi, req.outputDir, req.uniqIdPrefix, req.filePrefix, debugger, req.cancelFlag);
            auto t1 = std::chrono::steady_clock::now();
            std::chrono::duration<double> dur = t1 - t0;

            out.pages = std::move(pages);

            std::ostringstream oss;
            oss << "extract:done " << req.pdfPath.string() << " pages=" << out.pages.size() << " duration=" << dur.count();
            writeDebugTextSafe("poppler/log", oss.str());
        } catch (const std::exception& ex) {
            writeDebugTextSafe("poppler/log", std::string("extract:error ") + ex.what());
            throw;
        }
        return out;
    }

  private:
    std::shared_ptr<IDebugger> debugger;
};

std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer> createPdfRendererAdapter(std::shared_ptr<IDebugger> debugger) {
    return std::make_shared<PopplerPdfRendererAdapter>(std::move(debugger));
}

