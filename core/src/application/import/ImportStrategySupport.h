/**
 * @file core/src/application/import/ImportStrategySupport.h
 * @brief Declares private support types and helpers for the default import strategy.
 */

#pragma once

#include "ImportPipelineHelpers.h"

#include "core/ports/infra/image-processing/IImageProcessor.h"
#include "core/ports/infra/pdf-rendering/PdfRenderingRequest.h"
#include "core/ports/infra/pdf-rendering/PdfRenderingResult.h"
#include "core/ports/infra/text-recognition/ITextRecognizer.h"
#include "core/ports/diagnostics/IErrorReporter.h"
#include "core/jobs/Scheduler.h"

#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <vector>

namespace core::application::importing {

using ImportClock = std::chrono::steady_clock;

struct ImportRunTimings {
    ImportClock::time_point startedAt = ImportClock::now();
    double renderSec = 0.0;
    double extractSec = 0.0;
    double finalizeSec = 0.0;
};

struct SchedulerResources {
    core::jobs::Scheduler localScheduler;
    core::jobs::SlotLimiter localOcrLimiter;
    core::jobs::Scheduler* scheduler = nullptr;
    core::jobs::SlotLimiter* ocrLimiter = nullptr;

    SchedulerResources(const ImportRequest& req,
                       std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter);
};

void ensureDirectoryExists(const std::filesystem::path& path,
                          core::ports::diagnostics::IErrorReporter* errorReporter,
                          const char* origin);

internal::ProgressReporter makeProgressReporter(const ImportRequest& req,
                                                core::ports::diagnostics::IErrorReporter* errorReporter);

core::ports::pdf_rendering::RenderRequest makeRenderRequest(const ImportRequest& req);
core::ports::pdf_rendering::ExtractRequest makeExtractRequest(const core::ports::pdf_rendering::RenderRequest& renderRequest,
                                                const ImportRequest& req);

std::vector<internal::PageWork> collectPageWork(const ImportRequest& req,
                                              const core::ports::pdf_rendering::RenderResult& renderResult,
                                              const core::ports::pdf_rendering::ExtractResult& extractResult,
                                              const std::shared_ptr<core::ports::image_processing::IImageProcessor>& opencv,
                                              const std::shared_ptr<core::ports::text_recognition::ITextRecognizer>& tesseract,
                                              SchedulerResources& resources,
                                              const internal::ProgressReporter& report,
                                              core::ports::diagnostics::IErrorReporter* errorReporter,
                                              ImportResult& out,
                                              std::mutex& artifactsMutex);

void attachMetricsArtifact(ImportResult& out,
                           const ImportRequest& req,
                           const std::vector<internal::PageWork>& pages,
                           size_t totalPages,
                           const internal::FinalizeStats& finalizeStats,
                           const ImportRunTimings& timings,
                           core::ports::diagnostics::IErrorReporter* errorReporter);

}
