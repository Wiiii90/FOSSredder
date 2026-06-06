/**
 * @file core/src/import/ImportPipelineHelpers.cpp
 * @brief Implements private helper functions used by the import pipeline.
 */

#include "ImportPipelineHelpers.h"
#include "ImportPageRequests.h"

#include "core/constants/import.h"
#include "core/errors/ErrorReporting.h"
#include "core/application/import/statement/DefaultStatementParser.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <thread>

namespace core::application::importing::internal {
using core::application::importing::statement::DefaultStatementParser;
namespace poppler = core::ports::pdf_rendering;
namespace tesseract = core::ports::text_recognition;

namespace {

constexpr auto kTsvArtifactPrefix = "tesseract/page_";
constexpr auto kTsvArtifactSuffix = "_crop_0.tsv";
constexpr auto kParserLogArtifactPrefix = "parser/page_";
constexpr auto kParserLogArtifactSuffix = ".log";
constexpr auto kPausePollInterval = std::chrono::milliseconds(50);

bool importCanceled(const ImportRequest& req)
{
    return req.cancelFlag && req.cancelFlag->load();
}

bool waitWhilePaused(const ImportRequest& req)
{
    while (req.pauseFlag && req.pauseFlag->load()) {
        if (importCanceled(req)) return false;
        std::this_thread::sleep_for(kPausePollInterval);
    }
    return !importCanceled(req);
}

void safeReleaseLimiter(core::jobs::SlotLimiter* ocrLimiter,
                        core::errors::IErrorReporter* errorReporter,
                        const char* origin)
{
    try {
        if (ocrLimiter) ocrLimiter->release();
    } catch (...) {
        core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, origin, std::current_exception());
    }
}

core::ports::text_recognition::ExtractResult extractWithLimiter(const std::shared_ptr<core::ports::text_recognition::ITextRecognizer>& tesseractService,
                                                 const tesseract::ExtractRequest& request,
                                                 core::jobs::SlotLimiter* ocrLimiter,
                                                 double& accumulatedOcrSec)
{
    if (ocrLimiter) ocrLimiter->acquire();
    const auto ocrStart = std::chrono::steady_clock::now();
    auto result = tesseractService->extract(request);
    accumulatedOcrSec += std::chrono::duration<double>(std::chrono::steady_clock::now() - ocrStart).count();
    if (ocrLimiter) ocrLimiter->release();
    return result;
}

std::string makeTsvArtifactKey(size_t pageIndex)
{
    return std::string(kTsvArtifactPrefix) + std::to_string(pageIndex) + kTsvArtifactSuffix;
}

std::string makeParserLogArtifactKey(size_t pageIndex)
{
    return std::string(kParserLogArtifactPrefix) + std::to_string(pageIndex) + kParserLogArtifactSuffix;
}

void storeTsvArtifact(ImportResult& out,
                      size_t pageIndex,
                      const tesseract::ExtractResult& extractResult,
                      std::mutex& artifactsMutex,
                      core::errors::IErrorReporter* errorReporter)
{
    try {
        std::vector<uint8_t> tsvData(extractResult.tsv.begin(), extractResult.tsv.end());
        std::lock_guard<std::mutex> guard(artifactsMutex);
        out.artifacts[makeTsvArtifactKey(pageIndex)] = std::move(tsvData);
    } catch (...) {
        core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::storeTsvArtifact", std::current_exception());
    }
}

}

std::vector<uint8_t> readImportBytes(const std::filesystem::path& path)
{
    return readImportBytes(path, nullptr);
}

std::vector<uint8_t> readImportBytes(const std::filesystem::path& path, core::errors::IErrorReporter* errorReporter)
{
    std::vector<uint8_t> out;
    try {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs) return out;
        out.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    } catch (...) { core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::readBytes", std::current_exception()); }
    return out;
}

PageWork processImportPage(size_t pageIndex,
                           size_t totalPages,
                           const ImportRequest& req,
                           const poppler::RenderResult& renderRes,
                           const poppler::ExtractResult& extractRes,
                           const std::shared_ptr<core::ports::image_processing::IImageProcessor>& opencv,
                           const std::shared_ptr<core::ports::text_recognition::ITextRecognizer>& tesseract,
                           core::jobs::SlotLimiter* ocrLimiter,
                           const ProgressReporter& report,
                           std::atomic<size_t>& doneUnits,
                           size_t totalUnits,
                           core::errors::IErrorReporter* errorReporter,
                           ImportResult& out,
                           std::mutex& artifactsMutex)
{
    const auto pageStart = std::chrono::steady_clock::now();
    PageWork page;
    page.pageIndex = pageIndex;

    static constexpr size_t unitsPerPage = core::constants::importing::kUnitsPerPage;
    size_t localUnitsDone = 0;
    auto unitDone = [&](size_t inc, const std::string& label) {
        localUnitsDone += inc;
        const size_t d = doneUnits.fetch_add(inc) + inc;
        const double frac = std::min(1.0, static_cast<double>(d) / static_cast<double>(totalUnits));
        const double progress = core::constants::importing::kProgressPageWorkBase + core::constants::importing::kProgressPageWorkSpan * frac;
        report(progress, std::string("[") + std::to_string(pageIndex + 1) + "/" + std::to_string(totalPages) + "] " + label);
    };
    auto finishUnits = [&](const std::string& label) {
        if (localUnitsDone >= unitsPerPage) return;
        unitDone(unitsPerPage - localUnitsDone, label);
    };

    if (req.cancelFlag && req.cancelFlag->load()) {
        finishUnits(std::string(core::constants::importing::kProgressCanceled));
        return page;
    }
    if (!waitWhilePaused(req)) {
        finishUnits(std::string(core::constants::importing::kProgressCanceled));
        return page;
    }

    std::vector<uint8_t> pageBytes;
    if (pageIndex < renderRes.imageBytes.size() && !renderRes.imageBytes[pageIndex].empty()) {
        pageBytes = renderRes.imageBytes[pageIndex];
    } else if (pageIndex < renderRes.images.size() && !renderRes.images[pageIndex].empty()) {
        pageBytes = readImportBytes(renderRes.images[pageIndex]);
    }
    if (pageBytes.empty()) {
        finishUnits(std::string(core::constants::importing::pageSteps::kNoImage));
        return page;
    }
    if (!waitWhilePaused(req)) {
        finishUnits(std::string(core::constants::importing::kProgressCanceled));
        return page;
    }

    auto maskRequest = buildMaskRequest(pageBytes, pageIndex, req, extractRes);

    if (maskRequest.useTesseract) {
        if (!waitWhilePaused(req)) {
            finishUnits(std::string(core::constants::importing::kProgressCanceled));
            return page;
        }
        try {
            const auto tessRequest = buildMaskOcrRequest(pageBytes, req);
            const auto tessResponse = extractWithLimiter(tesseract, tessRequest, ocrLimiter, page.ocrSec);

            maskRequest.tesseractTsv = tessResponse.tsv;
        } catch (...) {
            safeReleaseLimiter(ocrLimiter, errorReporter, "core::import::DefaultImportStatementStrategy::ocrLimiterReleaseMask");
            core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::tesseractMaskExtract", std::current_exception());
        }
    }

    if (!waitWhilePaused(req)) {
        finishUnits(std::string(core::constants::importing::kProgressCanceled));
        return page;
    }
    auto maskResponse = opencv->mask(maskRequest);
    std::vector<uint8_t> maskedBytes = !maskResponse.maskedImageBytes.empty() ? maskResponse.maskedImageBytes : pageBytes;
    unitDone(1, std::string(core::constants::importing::pageSteps::kMask));

    if (!waitWhilePaused(req)) {
        finishUnits(std::string(core::constants::importing::kProgressCanceled));
        return page;
    }
    const auto detectRequest = buildDetectRequest(maskedBytes, pageIndex, req);
    auto detectResponse = opencv->detect(detectRequest);
    unitDone(1, std::string(core::constants::importing::pageSteps::kDetect));

    if (!detectResponse.detected) {
        finishUnits(std::string(core::constants::importing::pageSteps::kNoTable));
        return page;
    }

    if (!waitWhilePaused(req)) {
        finishUnits(std::string(core::constants::importing::kProgressCanceled));
        return page;
    }
    const auto cropRequest = buildCropRequest(pageBytes, pageIndex, req, detectResponse);
    auto cropResponse = opencv->crop(cropRequest);
    unitDone(1, std::string(core::constants::importing::pageSteps::kCrop));

    if (cropResponse.croppedImageBytes.empty() || cropResponse.croppedImageBytes.front().empty()) {
        finishUnits(std::string(core::constants::importing::pageSteps::kNoCrop));
        return page;
    }

    const auto tableRequest = buildTableOcrRequest(cropResponse.croppedImageBytes.front(), pageIndex, req, detectResponse);

    if (!waitWhilePaused(req)) {
        finishUnits(std::string(core::constants::importing::kProgressCanceled));
        return page;
    }
    try {
        page.ocr = extractWithLimiter(tesseract, tableRequest, ocrLimiter, page.ocrSec);
    } catch (...) {
        safeReleaseLimiter(ocrLimiter, errorReporter, "core::import::DefaultImportStatementStrategy::ocrLimiterReleaseTable");
        core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::tesseractTableExtract", std::current_exception());
        finishUnits(std::string(core::constants::importing::pageSteps::kOcrFailed));
        return page;
    }

    unitDone(1, std::string(core::constants::importing::pageSteps::kOcr));

    page.hasTable = true;
    page.table = detectResponse.table;
    page.cropBytes = std::move(cropResponse.croppedImageBytes.front());
    page.ocrWords = page.ocr.words.size();

    storeTsvArtifact(out, pageIndex, page.ocr, artifactsMutex, errorReporter);

    finishUnits(std::string(core::constants::importing::pageSteps::kDone));
    page.totalSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - pageStart).count();
    return page;
}

FinalizeStats finalizeParsedPages(const ImportRequest& req,
                                  const std::vector<PageWork>& pages,
                                  const std::shared_ptr<core::ports::image_processing::IImageProcessor>& opencv,
                                  ImportResult& out,
                                  std::vector<core::application::importing::draft::TransactionDraft>& all,
                                  std::string& carriedBookingDate,
                                  int& nextTxIndex,
                                  core::errors::IErrorReporter* errorReporter,
                                  const ProgressReporter& report,
                                  std::mutex& artifactsMutex)
{
    FinalizeStats stats;
    const size_t finalizePageCount = std::max<size_t>(1, pages.size());
    for (size_t pageIndex = 0; pageIndex < pages.size(); ++pageIndex) {
        if (!waitWhilePaused(req)) {
            report(0.0, std::string(core::constants::importing::kProgressCanceled));
            break;
        }
        if (req.cancelFlag && req.cancelFlag->load()) {
            report(0.0, std::string(core::constants::importing::kProgressCanceled));
            break;
        }

        const auto& page = pages[pageIndex];
        if (!page.hasTable) continue;

        const double fraction = static_cast<double>(pageIndex + 1) / static_cast<double>(finalizePageCount);
        const double progress = core::constants::importing::kProgressFinalizeBase + core::constants::importing::kProgressFinalizeSpan * fraction;
        report(progress, std::string("[") + std::to_string(pageIndex + 1) + "/" + std::to_string(finalizePageCount) + "] " + std::string(core::constants::importing::pageSteps::kFinalize));

        ++stats.pagesWithTable;
        stats.totalOcrWords += page.ocrWords;
        stats.maxPageSec = std::max(stats.maxPageSec, page.totalSec);
        stats.sumPageSec += page.totalSec;
        stats.sumOcrSec += page.ocrSec;

        auto parsed = DefaultStatementParser::parse(page.table,
                                                    page.ocr,
                                                    std::string(),
                                                    opencv,
                                                    page.cropBytes,
                                                    carriedBookingDate,
                                                    nextTxIndex);
        carriedBookingDate = parsed.lastBookingDate;
        nextTxIndex = parsed.nextTransactionIndex;

        if (!parsed.transactions.empty()) {
            for (auto& transaction : parsed.transactions) all.push_back(std::move(transaction));
        }

        if (!parsed.debugLines.empty()) {
            try {
                std::ostringstream outLog;
                for (const auto& line : parsed.debugLines) outLog << line << '\n';
                const auto serialized = outLog.str();
                std::lock_guard<std::mutex> guard(artifactsMutex);
                out.artifacts[makeParserLogArtifactKey(page.pageIndex)] = std::vector<uint8_t>(serialized.begin(), serialized.end());
            } catch (...) {
                core::errors::reportException(errorReporter,
                                              core::errors::ErrorSeverity::Warning,
                                              "core::import::DefaultImportStatementStrategy::storeParserLogArtifact",
                                              std::current_exception());
            }
        }

        if (!parsed.artifacts.empty()) {
            try {
                std::lock_guard<std::mutex> guard(artifactsMutex);
                for (auto& [key, value] : parsed.artifacts) out.artifacts.emplace(key, std::move(value));
            } catch (...) { core::errors::reportException(errorReporter, core::errors::ErrorSeverity::Warning, "core::import::DefaultImportStatementStrategy::mergeParsedArtifacts", std::current_exception()); }
        }
    }

    return stats;
}

}
