/**
 * @file core/src/application/import/ImportPageRequests.h
 * @brief Declares private request-building helpers for per-page import processing.
 */

#pragma once

#include "core/ports/infra/document-image-processing/DocumentImageProcessingRequest.h"
#include "core/ports/infra/document-image-processing/DocumentImageProcessingResult.h"
#include "core/ports/infra/pdf-rendering/PdfRenderingResult.h"
#include "core/ports/infra/text-recognition/TextRecognitionRequest.h"
#include "ImportConstants.h"
#include "core/application/import/ImportRequest.h"
#include "../../utils/UniqId.h"

#include <cmath>
#include <string>
#include <vector>

namespace core::application::importing::internal {

inline core::ports::text_recognition::Settings buildRecognitionSettings(int defaultPsm)
{
    core::ports::text_recognition::Settings settings;
    settings.psm = defaultPsm;
    return settings;
}

inline core::ports::document_image_processing::MaskRequest buildMaskRequest(const std::vector<uint8_t>& pageBytes,
                                                 size_t pageIndex,
                                                 const ImportRequest& req,
                                                 const core::ports::pdf_rendering::ExtractResult& extractRes)
{
    core::ports::document_image_processing::MaskRequest request;
    request.imageBytes = pageBytes;
    request.uniqIdPrefix = core::utils::makeUniqId();
    request.filePrefix = std::string(constants::kDocumentImageProcessingMaskPrefix) + std::to_string(pageIndex + 1);
    request.usePoppler = true;
    request.useMorphology = true;
    request.useTesseract = false;
    request.cancelFlag = req.cancelFlag;

    if (pageIndex < extractRes.pages.size()) {
        const auto& pageExtract = extractRes.pages[pageIndex];
        const double scaleX = pageExtract.dpiX / 72.0;
        const double scaleY = pageExtract.dpiY / 72.0;
        for (const auto& textElement : pageExtract.textElements) {
            core::ports::document_image_processing::Rect rect;
            rect.x = static_cast<int>(std::round(textElement.x * scaleX));
            rect.y = static_cast<int>(std::round(textElement.y * scaleY));
            rect.width = static_cast<int>(std::round(textElement.width * scaleX));
            rect.height = static_cast<int>(std::round(textElement.height * scaleY));
            if (rect.width <= 1 || rect.height <= 1) continue;
            request.textElements.push_back(rect);
        }
    }

    if (request.textElements.empty()) request.useTesseract = true;
    return request;
}

inline core::ports::text_recognition::ExtractRequest buildMaskOcrRequest(const std::vector<uint8_t>& pageBytes,
                                                          const ImportRequest& req)
{
    core::ports::text_recognition::ExtractRequest request;
    request.imageBytes.assign(pageBytes.begin(), pageBytes.end());
    return request;
}

inline core::ports::document_image_processing::DetectRequest buildDetectRequest(const std::vector<uint8_t>& maskedBytes,
                                                     size_t pageIndex,
                                                     const ImportRequest& req)
{
    core::ports::document_image_processing::DetectRequest request;
    request.imageBytes = maskedBytes;
    request.uniqIdPrefix = core::utils::makeUniqId();
    request.filePrefix = std::string(constants::kDocumentImageProcessingDetectPrefix) + std::to_string(pageIndex + 1);
    request.kind = core::ports::document_image_processing::DetectRequest::DetectKind::Tables;
    request.cancelFlag = req.cancelFlag;
    return request;
}

inline core::ports::document_image_processing::CropRequest buildCropRequest(const std::vector<uint8_t>& pageBytes,
                                                 size_t pageIndex,
                                                 const ImportRequest& req,
                                                 const core::ports::document_image_processing::DetectResult& detectResponse)
{
    core::ports::document_image_processing::CropRequest request;
    request.imageBytes = pageBytes;
    request.uniqIdPrefix = core::utils::makeUniqId();
    request.filePrefix = std::string(constants::kDocumentImageProcessingCropPrefix) + std::to_string(pageIndex + 1);
    request.bbox = detectResponse.table.bbox;
    request.cancelFlag = req.cancelFlag;
    return request;
}

inline core::ports::text_recognition::ExtractRequest buildTableOcrRequest(const std::vector<uint8_t>& croppedBytes,
                                                           size_t pageIndex,
                                                           const ImportRequest& req,
                                                           const core::ports::document_image_processing::DetectResult& detectResponse)
{
    core::ports::text_recognition::ExtractRequest request;
    request.imageBytes = croppedBytes;
    request.cancelFlag = req.cancelFlag;

    return request;
}

}
