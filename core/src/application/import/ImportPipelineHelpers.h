/**
 * @file core/src/application/import/ImportPipelineHelpers.h
 * @brief Declares private helper types and functions for the import pipeline implementation.
 */

#pragma once

#include "core/ports/infra/image-processing/ImageProcessingResult.h"
#include "core/ports/infra/pdf-rendering/PdfRenderingResult.h"
#include "core/ports/infra/text-recognition/TextRecognitionResult.h"
#include "core/ports/diagnostics/IErrorReporter.h"
#include "core/application/import/ImportRequest.h"
#include "core/application/import/ImportResult.h"
#include "core/jobs/Scheduler.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/ports/infra/image-processing/IImageProcessor.h"
#include "core/ports/infra/text-recognition/ITextRecognizer.h"

#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace core::application::importing::internal {

using ProgressReporter = std::function<void(double, const std::string&)>;

struct PageWork {
    bool hasTable = false;
    core::ports::image_processing::Table table;
    core::ports::text_recognition::ExtractResult ocr;
    std::vector<uint8_t> cropBytes;
    size_t pageIndex = 0;
    double totalSec = 0.0;
    double ocrSec = 0.0;
    size_t ocrWords = 0;
};

struct FinalizeStats {
    size_t pagesWithTable = 0;
    size_t totalOcrWords = 0;
    double maxPageSec = 0.0;
    double sumPageSec = 0.0;
    double sumOcrSec = 0.0;
};

std::vector<uint8_t> readImportBytes(const std::filesystem::path& path);
std::vector<uint8_t> readImportBytes(const std::filesystem::path& path, core::ports::diagnostics::IErrorReporter* errorReporter);

PageWork processImportPage(size_t pageIndex,
                           size_t totalPages,
                           const ImportRequest& req,
                           const core::ports::pdf_rendering::RenderResult& renderRes,
                           const core::ports::pdf_rendering::ExtractResult& extractRes,
                           const std::shared_ptr<core::ports::image_processing::IImageProcessor>& opencv,
                                              const std::shared_ptr<core::ports::text_recognition::ITextRecognizer>& tesseract,
                           core::jobs::SlotLimiter* ocrLimiter,
                           const ProgressReporter& report,
                           std::atomic<size_t>& doneUnits,
                           size_t totalUnits,
                           core::ports::diagnostics::IErrorReporter* errorReporter,
                           ImportResult& out,
                           std::mutex& artifactsMutex);

FinalizeStats finalizeParsedPages(const ImportRequest& req,
                                  const std::vector<PageWork>& pages,
                                  const std::shared_ptr<core::ports::image_processing::IImageProcessor>& opencv,
                                  ImportResult& out,
                                  std::vector<core::application::importing::draft::TransactionDraft>& all,
                                  std::string& carriedBookingDate,
                                  int& nextTxIndex,
                                  core::ports::diagnostics::IErrorReporter* errorReporter,
                                  const ProgressReporter& report,
                                  std::mutex& artifactsMutex);

}
