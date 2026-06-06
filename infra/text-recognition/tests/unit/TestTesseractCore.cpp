/**
 * @file infra/text-recognition/tests/unit/TestTesseractCore.cpp
 * @brief Tests for the Tesseract OCR core helpers.
 */

#include <gtest/gtest.h>

#include "text-recognition/TesseractCore.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cctype>

namespace infra::text_recognition::tests {
namespace {

std::vector<uint8_t> encodeTextImage(const std::string& text) {
    cv::Mat image(220, 800, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::putText(image, text, cv::Point(30, 140), cv::FONT_HERSHEY_SIMPLEX, 3.0, cv::Scalar(0, 0, 0), 6, cv::LINE_AA);
    std::vector<uint8_t> bytes;
    cv::imencode(".png", image, bytes);
    return bytes;
}

std::string trimCopy(std::string value) {
    const auto notSpace = [](unsigned char ch) {
        return !std::isspace(ch);
    };

    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

TEST(TesseractCoreTest, ReturnsEmptyOutputForEmptyInput) {
    const core::ports::text_recognition::Settings settings;
    const auto [text, words] = TesseractCore::extractFromBytes({}, "", settings);

    EXPECT_TRUE(text.text.empty());
    EXPECT_TRUE(words.empty());
}

TEST(TesseractCoreTest, RecognizesSimpleTextAndWordsFromGeneratedImage) {
    const auto bytes = encodeTextImage("12345");
    core::ports::text_recognition::Settings settings;
    settings.language = "deu";
    settings.psm = 7;
    settings.engineMode = core::ports::text_recognition::EngineMode::Default;
    settings.preserveInterwordSpaces = true;
    settings.charWhitelist = "0123456789";

    const auto [text, words] = TesseractCore::extractFromBytes(bytes, "", settings);

    EXPECT_FALSE(trimCopy(text.text).empty());
    EXPECT_FALSE(words.empty());
}

} // namespace
} // namespace infra::text_recognition::tests
