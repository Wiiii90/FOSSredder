/**
 * @file core/include/core/ports/infra/pdf-rendering/PdfRenderingRequest.h
 * @brief Request types for PDF rendering operations.
 */
#pragma once

#include <atomic>
#include <filesystem>
#include <memory>
#include <string>

namespace core::ports::pdf_rendering {

struct RenderRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
    std::filesystem::path outputDir;
    std::string uniqIdPrefix;
    std::string filePrefix;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

struct ExtractRequest {
    std::filesystem::path pdfPath;
    double dpi = 300.0;
    int fromPage = 0;
    int toPage = -1;
    std::filesystem::path outputDir;
    std::string uniqIdPrefix;
    std::string filePrefix;
    std::shared_ptr<std::atomic<bool>> cancelFlag;
};

} // namespace core::ports::pdf_rendering
