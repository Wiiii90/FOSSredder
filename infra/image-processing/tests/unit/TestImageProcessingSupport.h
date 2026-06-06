/**
 * @file infra/image-processing/tests/unit/TestImageProcessingSupport.h
 * @brief Shared helpers for image-processing tests.
 */

#pragma once

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace infra::image_processing::tests {
namespace support {

inline std::filesystem::path makeTempDir(const std::string& stem) {
    auto dir = std::filesystem::temp_directory_path() / stem;
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

inline cv::Mat makeCanvas(int width, int height, const cv::Scalar& background = cv::Scalar(255, 255, 255)) {
    return cv::Mat(height, width, CV_8UC3, background);
}

inline void drawTextBlock(cv::Mat& image, const cv::Rect& rect, const cv::Scalar& color = cv::Scalar(0, 0, 0)) {
    cv::rectangle(image, rect, color, cv::FILLED, cv::LINE_AA);
}

inline void drawGrid(cv::Mat& image, int rows, int cols, int margin = 10, int lineWidth = 3) {
    const int cellWidth = (image.cols - 2 * margin) / cols;
    const int cellHeight = (image.rows - 2 * margin) / rows;
    for (int c = 0; c <= cols; ++c) {
        const int x = margin + c * cellWidth;
        cv::line(image, cv::Point(x, margin), cv::Point(x, margin + rows * cellHeight), cv::Scalar(0, 0, 0), lineWidth);
    }
    for (int r = 0; r <= rows; ++r) {
        const int y = margin + r * cellHeight;
        cv::line(image, cv::Point(margin, y), cv::Point(margin + cols * cellWidth, y), cv::Scalar(0, 0, 0), lineWidth);
    }
}

inline std::vector<uint8_t> encodePng(const cv::Mat& image) {
    std::vector<uint8_t> bytes;
    cv::imencode(".png", image, bytes);
    return bytes;
}

inline std::filesystem::path writePng(const std::filesystem::path& path, const cv::Mat& image) {
    std::filesystem::create_directories(path.parent_path());
    cv::imwrite(path.string(), image);
    return path;
}

} // namespace support
} // namespace infra::image_processing::tests
