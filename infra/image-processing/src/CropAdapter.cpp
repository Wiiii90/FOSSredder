/**
 * @file infra/image-processing/src/CropAdapter.cpp
 * @brief Implements OpenCV crop extraction for the import workflow.
 */

#include "image-processing/pch.h"
#include "image-processing/CropAdapter.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace opencv {
namespace ports = core::ports::image_processing;

std::vector<std::filesystem::path> CropAdapter::cropImages(const std::string& imagePath,
                                                         const std::vector<ports::Rect>& rects,
                                                         const std::filesystem::path& outputDir,
                                                         ports::CropRequest::OutputFormat fmt,
                                                         int jpegQuality,
                                                         std::shared_ptr<IDebugger> debugger,
                                                         const std::string& filePrefix) {
    try {
        cv::Mat img = cv::imread(imagePath, cv::IMREAD_COLOR);
        if (img.empty()) {
            if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "Failed to load image for cropping: " + imagePath);
            return {};
        }
        return cropImages(img, rects, outputDir, fmt, jpegQuality, nullptr, std::move(debugger), filePrefix);
    } catch (...) {
        return {};
    }
}

std::vector<std::filesystem::path> CropAdapter::cropImages(const cv::Mat& img,
                                                         const std::vector<ports::Rect>& rects,
                                                         const std::filesystem::path& outputDir,
                                                         ports::CropRequest::OutputFormat fmt,
                                                         int jpegQuality,
                                                         std::vector<std::vector<uint8_t>>* outBytes,
                                                         std::shared_ptr<IDebugger> debugger,
                                                         const std::string& filePrefix) {
    std::vector<std::filesystem::path> outPaths;
    if (rects.empty()) return outPaths;
    if (img.empty()) return outPaths;

    if (outBytes) outBytes->clear();

    const char* ext = (fmt == ports::CropRequest::OutputFormat::Jpg) ? ".jpg" : ".png";
    std::vector<int> encodeParams;
    if (fmt == ports::CropRequest::OutputFormat::Jpg) {
        encodeParams.push_back(cv::IMWRITE_JPEG_QUALITY);
        encodeParams.push_back(std::max(1, std::min(100, jpegQuality)));
    }

    if (!outputDir.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(outputDir, ec);
    }

    int idx = 0;
    for (const auto& rct : rects) {
        try {
            cv::Rect r(rct.x, rct.y, rct.width, rct.height);
            r &= cv::Rect(0, 0, img.cols, img.rows);
            if (r.width <= 0 || r.height <= 0) { ++idx; continue; }
            cv::Mat crop = img(r).clone();

            if (outBytes) {
                std::vector<uint8_t> buf;
                if (cv::imencode(ext, crop, buf, encodeParams)) outBytes->push_back(std::move(buf));
                else outBytes->push_back({});
            }

            if (!outputDir.empty()) {
                std::ostringstream fname;
                fname << (outputDir.string()) << "/";
                if (!filePrefix.empty()) fname << filePrefix << "_";
                fname << (idx + 1) << ext;
                std::string filename = fname.str();
                if (!cv::imwrite(filename, crop, encodeParams)) {
                    if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "Failed to write crop: " + filename);
                } else {
                    outPaths.emplace_back(filename);
                    if (debugger && debugger->enabled()) {
                        std::ifstream ifs(filename, std::ios::binary);
                        if (ifs) {
                            std::vector<uint8_t> buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                            debugger->writeBytes(std::string("opencv_cropped"), buf);
                        }
                    }
                }
            }
        } catch (...) {
            if (outBytes) outBytes->push_back({});
        }
        ++idx;
    }

    return outPaths;
}

} // namespace opencv
