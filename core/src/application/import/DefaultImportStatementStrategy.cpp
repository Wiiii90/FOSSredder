/**
 * @file core/src/import/DefaultImportStatementStrategy.cpp
 * @brief Implements the default import strategy orchestration for statement extraction.
 */

#include "core/pch.h"

#include "ImportStatementStrategy.h"
#include "core/ports/infra/pdf-rendering/PdfRenderingRequest.h"
#include "core/ports/infra/pdf-rendering/PdfRenderingResult.h"
#include "core/ports/infra/pdf-rendering/IPdfRenderer.h"
#include "core/ports/infra/image-processing/ImageProcessingRequest.h"
#include "core/ports/infra/image-processing/ImageProcessingResult.h"
#include "core/ports/infra/image-processing/IImageProcessor.h"
#include "core/ports/infra/text-recognition/TextRecognitionRequest.h"
#include "core/ports/infra/text-recognition/TextRecognitionResult.h"
#include "core/ports/infra/text-recognition/ITextRecognizer.h"
#include "core/constants/import.h"
#include "core/errors/ErrorReporting.h"
#include "core/application/import/IImportStatement.h"
#include "ImportPipelineHelpers.h"
#include "ImportStrategySupport.h"
#include "core/domain/entities/Statement.h"
#include <filesystem>
#include <memory>
#include <mutex>
#include <chrono>
#include <thread>

using core::application::importing::internal::FinalizeStats;
using core::application::importing::internal::PageWork;
using core::application::importing::internal::finalizeParsedPages;

namespace core::application::importing {
namespace poppler = core::ports::pdf_rendering;
namespace opencv = core::ports::image_processing;
namespace tesseract = core::ports::text_recognition;

namespace {

constexpr auto kPausePollInterval = std::chrono::milliseconds(50);

bool waitWhilePaused(const ImportRequest& req)
{
    while (req.pauseFlag && req.pauseFlag->load()) {
        if (req.cancelFlag && req.cancelFlag->load()) return false;
        std::this_thread::sleep_for(kPausePollInterval);
    }
    return !(req.cancelFlag && req.cancelFlag->load());
}

}

class DefaultImportStatementStrategy : public IImportStatementStrategy {
public:
    DefaultImportStatementStrategy(std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer> poppler,
        std::shared_ptr<core::ports::image_processing::IImageProcessor> opencv,
        std::shared_ptr<core::ports::text_recognition::ITextRecognizer> tesseract,
        std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter)
        : poppler_(std::move(poppler))
        , opencv_(std::move(opencv))
        , tesseract_(std::move(tesseract))
        , errorReporter_(std::move(errorReporter)) {
    }

    ImportResult run(const ImportRequest& req) override {
        ImportResult out;
        if (!poppler_ || !opencv_ || !tesseract_) return out;
        if (req.runRoot.empty()) return out;

        core::application::importing::ImportRunTimings timings;

        const std::filesystem::path runRoot(req.runRoot);
        core::application::importing::ensureDirectoryExists(runRoot, errorReporter_.get(), "core::import::DefaultImportStatementStrategy::createRunRoot");

        auto report = core::application::importing::makeProgressReporter(req, errorReporter_.get());

        report(core::constants::importing::kProgressPreparing, std::string(core::constants::importing::kProgressPreparingMessage));
        if (!waitWhilePaused(req)) { report(0.0, std::string(core::constants::importing::kProgressCanceled)); return out; }

        std::vector<core::application::importing::draft::TransactionDraft> all;

        const auto renderRequest = core::application::importing::makeRenderRequest(req);

        core::errors::report(errorReporter_.get(), {
            core::errors::ErrorSeverity::Info,
            "core::import::DefaultImportStatementStrategy::run",
            std::string("poppler render start: ") + renderRequest.pdfPath.string() + " dpi=" + std::to_string(renderRequest.dpi),
            {}
        });
        if (!waitWhilePaused(req)) { report(0.0, std::string(core::constants::importing::kProgressCanceled)); return out; }
        report(core::constants::importing::kProgressRendering, std::string(core::constants::importing::kProgressRenderingMessage));
        const auto renderStart = core::application::importing::ImportClock::now();
        auto renderRes = poppler_->render(renderRequest);
        timings.renderSec = std::chrono::duration<double>(core::application::importing::ImportClock::now() - renderStart).count();
        report(core::constants::importing::kProgressRendered, std::string(core::constants::importing::kProgressRenderedMessage));

        if (req.cancelFlag && req.cancelFlag->load()) { report(0.0, std::string(core::constants::importing::kProgressCanceled)); return out; }
        if (!waitWhilePaused(req)) { report(0.0, std::string(core::constants::importing::kProgressCanceled)); return out; }

        const auto extractRequest = core::application::importing::makeExtractRequest(renderRequest, req);
        report(core::constants::importing::kProgressExtracting, std::string(core::constants::importing::kProgressExtractingMessage));
        const auto extractStart = core::application::importing::ImportClock::now();
        auto extractRes = poppler_->extract(extractRequest);
        timings.extractSec = std::chrono::duration<double>(core::application::importing::ImportClock::now() - extractStart).count();
        report(core::constants::importing::kProgressExtracted, std::string(core::constants::importing::kProgressExtractedMessage));
        if (!waitWhilePaused(req)) { report(0.0, std::string(core::constants::importing::kProgressCanceled)); return out; }

        std::string carriedBookingDate;
        int nextTxIndex = 1;

        core::application::importing::SchedulerResources schedulerResources(req);
        std::mutex artifactsMutex;
        auto pages = core::application::importing::collectPageWork(req,
                                                      renderRes,
                                                      extractRes,
                                                      opencv_,
                                                      tesseract_,
                                                      schedulerResources,
                                                      report,
                                                      errorReporter_.get(),
                                                      out,
                                                      artifactsMutex);

        const size_t totalPages = pages.size();

        const auto finalizeStart = core::application::importing::ImportClock::now();
        const auto finalizeStats = finalizeParsedPages(req,
                                                       pages,
                                                       opencv_,
                                                       out,
                                                       all,
                                                       carriedBookingDate,
                                                       nextTxIndex,
                                                       errorReporter_.get(),
                                                       report,
                                                       artifactsMutex);
        timings.finalizeSec = std::chrono::duration<double>(core::application::importing::ImportClock::now() - finalizeStart).count();

        if (!all.empty()) {
            out.data = std::make_shared<Statement>();
            auto statementName = std::filesystem::path(req.sourcePath).stem().string();
            if (statementName.empty()) {
                statementName = std::filesystem::path(req.sourcePath).filename().string();
            }
            if (statementName.empty()) {
                statementName = "Imported statement";
            }
            out.data->rename(statementName);
            std::vector<std::string> transactionIds;
            transactionIds.reserve(all.size());
            for (const auto& transaction : all) {
                if (!transaction.id.empty()) {
                    transactionIds.push_back(transaction.id);
                }
            }
            out.data->setTransactionIds(std::move(transactionIds));
            out.transactions = std::move(all);
        }

        core::application::importing::attachMetricsArtifact(out, req, pages, totalPages, finalizeStats, timings, errorReporter_.get());

        report(1.0, std::string(core::constants::importing::kProgressDoneMessage));
        return out;
    }

private:
    std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer> poppler_;
    std::shared_ptr<core::ports::image_processing::IImageProcessor> opencv_;
    std::shared_ptr<core::ports::text_recognition::ITextRecognizer> tesseract_;
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter_;
};

std::unique_ptr<IImportStatementStrategy> createDefaultImportStrategy(std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer> poppler,
    std::shared_ptr<core::ports::image_processing::IImageProcessor> opencv,
    std::shared_ptr<core::ports::text_recognition::ITextRecognizer> tesseract,
    std::shared_ptr<core::ports::diagnostics::IErrorReporter> errorReporter) {
    return std::make_unique<DefaultImportStatementStrategy>(std::move(poppler), std::move(opencv), std::move(tesseract), std::move(errorReporter));
}

}
