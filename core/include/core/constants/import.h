#pragma once

#include <cstddef>
#include <string_view>

namespace core::constants::importing {

inline constexpr double kRenderDpi = 300.0;
inline constexpr double kProgressPreparing = 0.02;
inline constexpr double kProgressRendering = 0.05;
inline constexpr double kProgressRendered = 0.15;
inline constexpr double kProgressExtracting = 0.16;
inline constexpr double kProgressExtracted = 0.22;
inline constexpr double kProgressPageWorkBase = 0.22;
inline constexpr double kProgressPageWorkSpan = 0.60;
inline constexpr double kProgressFinalizeBase = 0.82;
inline constexpr double kProgressFinalizeSpan = 0.18;
inline constexpr std::size_t kLocalSchedulerWorkers = 4;
inline constexpr std::size_t kLocalSchedulerQueueCapacity = 128;
inline constexpr std::size_t kLocalOcrSlots = 2;
inline constexpr std::size_t kUnitsPerPage = 4;
inline constexpr std::string_view kRunNameImport = "import";

namespace runs {
inline constexpr int kKeepCount = 20;
inline constexpr std::string_view kTimestampFormat = "yyyyMMddHHmmsszzz";
inline constexpr int kFirstSuffix = 1;
} // namespace runs

inline constexpr std::string_view kPopplerRenderPrefix = "poppler_render";
inline constexpr std::string_view kPopplerExtractPrefix = "poppler_extract";
inline constexpr std::string_view kOpenCvMaskPrefix = "opencv_mask_page";
inline constexpr std::string_view kOpenCvDetectPrefix = "opencv_detect_tables_page";
inline constexpr std::string_view kOpenCvCropPrefix = "opencv_crop_table_page";
inline constexpr std::string_view kProgressCanceled = "Canceled";
inline constexpr std::string_view kProgressPreparingMessage = "Preparing import";
inline constexpr std::string_view kProgressRenderingMessage = "Rendering pages";
inline constexpr std::string_view kProgressRenderedMessage = "Rendered pages";
inline constexpr std::string_view kProgressExtractingMessage = "Extracting text";
inline constexpr std::string_view kProgressExtractedMessage = "Extracted text";
inline constexpr std::string_view kProgressDoneMessage = "Done";
inline constexpr std::string_view kErrorSourceMissing = "Source statement file does not exist";
inline constexpr std::string_view kErrorRunRootMissing = "Import run root is not configured";
inline constexpr std::string_view kErrorExtractionFailed = "core::domain::Statement extraction failed";
inline constexpr std::string_view kMetricsArtifactName = "metrics.json";

namespace pageSteps {
inline constexpr std::string_view kNoImage = "No image";
inline constexpr std::string_view kMask = "Mask";
inline constexpr std::string_view kDetect = "Detect";
inline constexpr std::string_view kNoTable = "No table";
inline constexpr std::string_view kCrop = "Crop";
inline constexpr std::string_view kNoCrop = "No crop";
inline constexpr std::string_view kOcr = "OCR";
inline constexpr std::string_view kOcrFailed = "OCR failed";
inline constexpr std::string_view kDone = "Done";
inline constexpr std::string_view kFinalize = "Finalize";
} // namespace pageSteps

namespace metrics {
inline constexpr std::string_view kJobId = "jobId";
inline constexpr std::string_view kSourcePath = "sourcePath";
inline constexpr std::string_view kPagesTotal = "pagesTotal";
inline constexpr std::string_view kPagesWithTable = "pagesWithTable";
inline constexpr std::string_view kRenderSeconds = "renderSec";
inline constexpr std::string_view kExtractSeconds = "extractSec";
inline constexpr std::string_view kFinalizeSeconds = "finalizeSec";
inline constexpr std::string_view kPageWorkSecondsMax = "pageWorkSecMax";
inline constexpr std::string_view kPageWorkSecondsSum = "pageWorkSecSum";
inline constexpr std::string_view kOcrSecondsSum = "ocrSecSum";
inline constexpr std::string_view kOcrWordsTotal = "ocrWordsTotal";
inline constexpr std::string_view kTotalSeconds = "totalSec";
inline constexpr std::string_view kPages = "pages";
inline constexpr std::string_view kIndex = "index";
inline constexpr std::string_view kHasTable = "hasTable";
inline constexpr std::string_view kOcrWords = "ocrWords";
} // namespace metrics

} // namespace core::constants::importing
