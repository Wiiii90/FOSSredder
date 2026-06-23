/**
 * @file infra/document-image-processing/src/OpenCvDocumentImageProcessorAdapter.cpp
 * @brief Implements the OpenCV adapter used by the import workflow.
 */

#include "document-image-processing/pch.h"
#include "core/ports/infra/document-image-processing/IDocumentImageProcessor.h"
#include "core/ports/infra/document-image-processing/DocumentImageProcessingRequest.h"
#include "core/ports/infra/document-image-processing/DocumentImageProcessingResult.h"
#include "document-image-processing/DenoiseAdapter.h"
#include "document-image-processing/MaskAdapter.h"
#include "document-image-processing/DetectAdapter.h"
#include "document-image-processing/CropAdapter.h"
#include "core/ports/diagnostics/IDiagnostics.h"

#include <opencv2/opencv.hpp>
#include <filesystem>

class OpenCvDocumentImageProcessorAdapter : public core::ports::document_image_processing::IDocumentImageProcessor {
public:
    OpenCvDocumentImageProcessorAdapter(std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg = nullptr) : debugger(std::move(dbg)) {}

    core::ports::document_image_processing::DenoiseResult denoise(const core::ports::document_image_processing::DenoiseRequest& req) const override {
        if (req.cancelFlag && req.cancelFlag->load()) return {};
        return opencv::DenoiseAdapter::denoise(req, debugger);
    }

    core::ports::document_image_processing::MaskResult mask(const core::ports::document_image_processing::MaskRequest& req) const override {
        if (req.cancelFlag && req.cancelFlag->load()) return {};
        return opencv::MaskAdapter::mask(req, debugger);
    }

    core::ports::document_image_processing::DetectResult detect(const core::ports::document_image_processing::DetectRequest& req) const override {
        core::ports::document_image_processing::DetectResult res;
        try {
            if (req.cancelFlag && req.cancelFlag->load()) return res;
            cv::Mat img;
            std::string label;
            if (!req.imageBytes.empty()) {
                img = cv::imdecode(req.imageBytes, cv::IMREAD_GRAYSCALE);
                label = std::string("<bytes>");
            }

            std::filesystem::path p = req.imagePath;
            if (img.empty()) {
                if (!std::filesystem::exists(p)) return res;
                img = cv::imread(p.string(), cv::IMREAD_GRAYSCALE);
                label = p.string();
            }
            if (img.empty()) return res;

            if (req.kind == core::ports::document_image_processing::DetectRequest::DetectKind::TextBlocks) {
                auto blocks = opencv::DetectAdapter::detectTextBlocks(img, debugger);
                if (!blocks.empty()) {
                    res.detected = true;
                    res.textBlocks.reserve(blocks.size());
                    for (const auto &b : blocks) {
                        core::ports::document_image_processing::Rect r;
                        r.x = b.x; r.y = b.y; r.width = b.width; r.height = b.height;
                        res.textBlocks.push_back(r);
                    }
                }
                return res;
            }

            auto tables = opencv::DetectAdapter::detectTables(img, label, debugger);
            if (!tables.empty()) {
                res.table = tables[0];
                res.detected = true;
            }
        } catch (...) {
            if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "OpenCvDocumentImageProcessorAdapter::detect failed");
        }
        return res;
    }

    core::ports::document_image_processing::CropResult crop(const core::ports::document_image_processing::CropRequest& req) const override {
        core::ports::document_image_processing::CropResult res;
        if (req.cancelFlag && req.cancelFlag->load()) return res;
        try {
            cv::Mat img;
            if (!req.imageBytes.empty()) {
                img = cv::imdecode(req.imageBytes, cv::IMREAD_COLOR);
            }

            std::filesystem::path p = req.imagePath;
            if (img.empty()) {
                if (!std::filesystem::exists(p)) return res;
                img = cv::imread(p.string(), cv::IMREAD_COLOR);
            }
            if (img.empty()) return res;

            std::vector<core::ports::document_image_processing::Rect> rects; rects.push_back(req.bbox);
            std::string prefix;
            if (!req.uniqIdPrefix.empty()) {
                prefix = req.uniqIdPrefix;
                if (!req.filePrefix.empty()) prefix += "_" + req.filePrefix;
            } else {
                prefix = req.filePrefix.empty() ? std::string("opencv_crop") : req.filePrefix;
            }

            res.croppedImageBytes.clear();
            res.croppedImagePaths = opencv::CropAdapter::cropImages(img, rects, req.outputDir, req.outputFormat, req.jpegQuality, &res.croppedImageBytes, debugger, prefix);
        } catch (...) {
            if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "OpenCvDocumentImageProcessorAdapter::crop failed");
        }
        return res;
    }

private:
    std::shared_ptr<core::ports::diagnostics::IDiagnostics> debugger;
};

std::shared_ptr<core::ports::document_image_processing::IDocumentImageProcessor> createDocumentImageProcessorAdapter(std::shared_ptr<core::ports::diagnostics::IDiagnostics> debugger) {
    return std::make_shared<OpenCvDocumentImageProcessorAdapter>(std::move(debugger));
}

