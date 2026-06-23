/**
 * @file core/src/application/import/ImportStrategySupport.cpp
 * @brief Implements private support types and helpers for the default import strategy.
 */

#include "ImportStrategySupport.h"

#include "ImportConstants.h"
#include "core/errors/ErrorReporting.h"
#include "core/jobs/Scheduler.h"
#include "../../utils/TransientId.h"

#include <chrono>
#include <future>
#include <nlohmann/json.hpp>
#include <thread>
#include <utility>

namespace core::application::importing {

namespace {

constexpr auto kPausePollInterval = std::chrono::milliseconds(50);

void waitWhilePaused(const ImportRequest& req)
{
    while (req.pauseFlag && req.pauseFlag->load()) {
        if (req.cancelFlag && req.cancelFlag->load()) return;
        std::this_thread::sleep_for(kPausePollInterval);
    }
}

}

SchedulerResources::SchedulerResources(
    const ImportRequest& req,
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter)
    : localScheduler(constants::kLocalSchedulerWorkers,
                     constants::kLocalSchedulerQueueCapacity,
                     std::move(errorReporter))
    , localOcrLimiter(constants::kLocalOcrSlots)
    , scheduler(req.scheduler ? req.scheduler : &localScheduler)
    , ocrLimiter(req.ocrLimiter ? req.ocrLimiter : &localOcrLimiter)
{
}

void ensureDirectoryExists(const std::filesystem::path& path,
                          core::ports::diagnostics::IErrorReporter* errorReporter,
                          const char* origin)
{
    try {
        std::filesystem::create_directories(path);
    } catch (...) {
        core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, origin, std::current_exception());
    }
}

internal::ProgressReporter makeProgressReporter(const ImportRequest& req,
                                                core::ports::diagnostics::IErrorReporter* errorReporter)
{
    return [&](double progress, const std::string& phase) {
        if (!req.progressCallback) return;
        waitWhilePaused(req);

        try {
            req.progressCallback(progress, phase);
        } catch (...) {
            core::errors::reportException(errorReporter,
                                          core::errors::ErrorSeverity::Warning,
                                          "core::import::DefaultImportStatementStrategy::progressCallback",
                                          std::current_exception());
        }
    };
}

core::ports::pdf_rendering::RenderRequest makeRenderRequest(const ImportRequest& req)
{
    core::ports::pdf_rendering::RenderRequest request;
    request.pdfPath = std::filesystem::path(req.sourcePath);
    request.dpi = constants::kRenderDpi;
    request.outputDir = std::filesystem::path();
    request.uniqIdPrefix = core::utils::makeTransientId();
    request.filePrefix = std::string(constants::kPopplerRenderPrefix);
    request.cancelFlag = req.cancelFlag;
    return request;
}

core::ports::pdf_rendering::ExtractRequest makeExtractRequest(const core::ports::pdf_rendering::RenderRequest& renderRequest,
                                                const ImportRequest& req)
{
    core::ports::pdf_rendering::ExtractRequest request;
    request.pdfPath = renderRequest.pdfPath;
    request.dpi = renderRequest.dpi;
    request.outputDir = std::filesystem::path();
    request.uniqIdPrefix = core::utils::makeTransientId();
    request.filePrefix = std::string(constants::kPopplerExtractPrefix);
    request.cancelFlag = req.cancelFlag;
    return request;
}

std::vector<internal::PageWork> collectPageWork(const ImportRequest& req,
                                              const core::ports::pdf_rendering::RenderResult& renderResult,
                                              const core::ports::pdf_rendering::ExtractResult& extractResult,
                                              const std::shared_ptr<core::ports::document_image_processing::IDocumentImageProcessor>& documentImageProcessor,
                                              const std::shared_ptr<core::ports::text_recognition::ITextRecognizer>& tesseract,
                                              SchedulerResources& resources,
                                              const internal::ProgressReporter& report,
                                              core::ports::diagnostics::IErrorReporter* errorReporter,
                                              ImportResult& out,
                                              std::mutex& artifactsMutex)
{
    const size_t totalPages = std::max(renderResult.images.size(), renderResult.imageBytes.size());
    static constexpr size_t unitsPerPage = constants::kUnitsPerPage;
    const size_t totalUnits = std::max<size_t>(1, totalPages * unitsPerPage);

    std::atomic<size_t> doneUnits{0};
    std::vector<internal::PageWork> pages(totalPages);
    std::vector<std::future<internal::PageWork>> futures;
    futures.reserve(totalPages);

    for (size_t pageIndex = 0; pageIndex < totalPages; ++pageIndex) {
        waitWhilePaused(req);
        if (req.cancelFlag && req.cancelFlag->load()) break;

        auto promise = std::make_shared<std::promise<internal::PageWork>>();
        futures.push_back(promise->get_future());

        resources.scheduler->enqueue([&, pageIndex, promise]() mutable {
            promise->set_value(internal::processImportPage(pageIndex,
                                                         totalPages,
                                                         req,
                                                         renderResult,
                                                         extractResult,
                                                         documentImageProcessor,
                                                         tesseract,
                                                         resources.ocrLimiter,
                                                         report,
                                                         doneUnits,
                                                         totalUnits,
                                                         errorReporter,
                                                         out,
                                                         artifactsMutex));
        });
    }

    for (auto& future : futures) {
        try {
            auto pageWork = future.get();
            if (pageWork.pageIndex < pages.size()) pages[pageWork.pageIndex] = std::move(pageWork);
        } catch (...) {
            core::errors::reportException(errorReporter,
                                          core::errors::ErrorSeverity::Warning,
                                          "core::import::DefaultImportStatementStrategy::pageFutureGet",
                                          std::current_exception());
        }
    }

    return pages;
}

void attachMetricsArtifact(ImportResult& out,
                           const ImportRequest& req,
                           const std::vector<internal::PageWork>& pages,
                           size_t totalPages,
                           const internal::FinalizeStats& finalizeStats,
                           const ImportRunTimings& timings,
                           core::ports::diagnostics::IErrorReporter* errorReporter)
{
    try {
        nlohmann::json metrics;
        metrics[constants::metrics::kJobId] = req.jobId;
        metrics[constants::metrics::kSourcePath] = req.sourcePath;
        metrics[constants::metrics::kPagesTotal] = totalPages;
        metrics[constants::metrics::kPagesWithTable] = finalizeStats.pagesWithTable;
        metrics[constants::metrics::kRenderSeconds] = timings.renderSec;
        metrics[constants::metrics::kExtractSeconds] = timings.extractSec;
        metrics[constants::metrics::kFinalizeSeconds] = timings.finalizeSec;
        metrics[constants::metrics::kPageWorkSecondsMax] = finalizeStats.maxPageSec;
        metrics[constants::metrics::kPageWorkSecondsSum] = finalizeStats.sumPageSec;
        metrics[constants::metrics::kOcrSecondsSum] = finalizeStats.sumOcrSec;
        metrics[constants::metrics::kOcrWordsTotal] = finalizeStats.totalOcrWords;
        metrics[constants::metrics::kTotalSeconds] = std::chrono::duration<double>(ImportClock::now() - timings.startedAt).count();

        auto& pageArray = metrics[constants::metrics::kPages];
        pageArray = nlohmann::json::array();
        for (const auto& page : pages) {
            nlohmann::json pageJson;
            pageJson[constants::metrics::kIndex] = page.pageIndex;
            pageJson[constants::metrics::kHasTable] = page.hasTable;
            pageJson[constants::metrics::kTotalSeconds] = page.totalSec;
            pageJson[constants::metrics::kOcrSecondsSum] = page.ocrSec;
            pageJson[constants::metrics::kOcrWords] = page.ocrWords;
            pageArray.push_back(std::move(pageJson));
        }

        const auto serialized = metrics.dump(2);
        out.artifacts[std::string(constants::kMetricsArtifactName)] =
            std::vector<uint8_t>(serialized.begin(), serialized.end());
    } catch (...) {
        core::errors::reportException(errorReporter,
                                      core::errors::ErrorSeverity::Warning,
                                      "core::import::DefaultImportStatementStrategy::metricsArtifact",
                                      std::current_exception());
    }
}

}
