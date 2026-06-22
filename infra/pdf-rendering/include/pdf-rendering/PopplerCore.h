/**
 * @file infra/pdf-rendering/include/pdf-rendering/PopplerCore.h
 * @brief Declares the Poppler implementation helpers used by the import workflow.
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <atomic>
#include "core/ports/infra/pdf-rendering/PdfRenderingTypes.h"

namespace core::ports::diagnostics { class IDiagnostics; }

class PopplerCore {
public:
    /**
     * @brief Extracts document metadata and page text elements from a PDF file.
     * @param path The source PDF file path.
     * @param dpi The logical render resolution used for coordinate conversion.
     * @param outputDir Optional debug output directory.
     * @param uniqIdPrefix Optional unique prefix for generated artifacts.
     * @param filePrefix Optional file name prefix for generated artifacts.
     * @param dbg Optional debugger used for trace output.
     * @param cancelFlag Optional cancellation flag for early exit.
     * @return The extracted page metadata.
     */
    static std::vector<core::ports::pdf_rendering::RenderedPage> renderDocument(const std::string& path,
                                                                 double dpi,
                                                                 const std::filesystem::path& outputDir = std::filesystem::path(),
                                                                 const std::string& uniqIdPrefix = "",
                                                                 const std::string& filePrefix = "",
                                                                 std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg = nullptr,
                                                                 std::shared_ptr<std::atomic<bool>> cancelFlag = nullptr);

    /**
     * @brief Extracts page metadata without rendering the page bitmap.
     * @param path The source PDF file path.
     * @param dpi The logical render resolution used for coordinate conversion.
     * @param outputDir Optional debug output directory.
     * @param uniqIdPrefix Optional unique prefix for generated artifacts.
     * @param filePrefix Optional file name prefix for generated artifacts.
     * @param dbg Optional debugger used for trace output.
     * @param cancelFlag Optional cancellation flag for early exit.
     * @return The extracted page metadata.
     */
    static std::vector<core::ports::pdf_rendering::RenderedPage> extractDocumentMeta(const std::string& path,
                                                                      double dpi,
                                                                      const std::filesystem::path& outputDir = std::filesystem::path(),
                                                                      const std::string& uniqIdPrefix = "",
                                                                      const std::string& filePrefix = "",
                                                                      std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg = nullptr,
                                                                      std::shared_ptr<std::atomic<bool>> cancelFlag = nullptr);
};
