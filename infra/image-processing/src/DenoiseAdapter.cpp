/**
 * @file infra/image-processing/src/DenoiseAdapter.cpp
 * @brief Implements OpenCV denoising for the import workflow.
 */

#include "image-processing/pch.h"
#include "image-processing/DenoiseAdapter.h"
#include "debug/IDebugger.h"
#include <opencv2/opencv.hpp>
#include <filesystem>

namespace opencv {
namespace ports = core::ports::image_processing;

namespace {
cv::Mat applyDenoise(const cv::Mat& img, ports::DenoiseRequest::Method method) {
    if (img.empty()) return cv::Mat();
    cv::Mat out;
    switch (method) {
    case ports::DenoiseRequest::Method::Median:
        cv::medianBlur(img, out, 3);
        break;
    case ports::DenoiseRequest::Method::Gaussian:
        cv::GaussianBlur(img, out, cv::Size(3, 3), 0);
        break;
    case ports::DenoiseRequest::Method::Bilateral:
        cv::bilateralFilter(img, out, 5, 50, 50);
        break;
    }
    return out.empty() ? img : out;
}
}

ports::DenoiseResult DenoiseAdapter::denoise(const ports::DenoiseRequest& req, std::shared_ptr<IDebugger> debugger) {
    ports::DenoiseResult res;
    try {
        const auto path = std::filesystem::path(req.imagePath);
        if (!std::filesystem::exists(path)) return res;

        cv::Mat src = cv::imread(path.string(), cv::IMREAD_COLOR);
        if (src.empty()) return res;
        cv::Mat denoised = applyDenoise(src, req.method);

        auto stem = path.stem().string();
        auto parent = path.parent_path();
        auto outPath = parent / (stem + "_denoised.png");
        if (cv::imwrite(outPath.string(), denoised)) {
            res.denoisedImagePath = outPath;
            if (debugger && debugger->enabled()) {
                std::vector<uint8_t> buf;
                if (cv::imencode(".png", denoised, buf)) {
                    debugger->writeBytes("opencv/" + stem + "/denoised.png", buf);
                }
            }
        }
    } catch (...) {
        if (debugger && debugger->enabled()) debugger->writeText("opencv/error.txt", "DenoiseAdapter::denoise failed");
    }
    return res;
}

} // namespace opencv
