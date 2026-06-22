/**
 * @file infra/text-recognition/tests/unit/TestTesseractTextRecognizerAdapter.cpp
 * @brief Tests for the Tesseract text recognizer adapter.
 */

#include <gtest/gtest.h>

#include "core/ports/infra/text-recognition/ITextRecognizer.h"
#include "core/ports/infra/text-recognition/TextRecognitionRequest.h"
#include "text-recognition/TesseractCore.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cctype>

namespace core::ports::diagnostics { class IDiagnostics; }

std::shared_ptr<core::ports::text_recognition::ITextRecognizer> createTextRecognizerAdapter();

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

TEST(TesseractTextRecognizerAdapterTest, ReturnsEmptyOutputForCancelledRequests) {
    auto recognizer = createTextRecognizerAdapter();
    auto cancelFlag = std::make_shared<std::atomic<bool>>(true);

    core::ports::text_recognition::ExtractRequest request;
    request.cancelFlag = cancelFlag;

    const auto result = recognizer->extract(request);

    EXPECT_TRUE(result.text.empty());
    EXPECT_TRUE(result.words.empty());
    EXPECT_TRUE(result.tables.empty());
}

TEST(TesseractTextRecognizerAdapterTest, ProducesTextAndTableContentForGeneratedImage) {
    auto recognizer = createTextRecognizerAdapter();
    core::ports::text_recognition::ExtractRequest request;
    request.imageBytes = encodeTextImage("12345");
    request.recognition.language = "deu";
    request.recognition.psm = 7;
    request.recognition.engineMode = core::ports::text_recognition::EngineMode::Default;
    request.recognition.preserveInterwordSpaces = true;
    request.recognition.charWhitelist = "0123456789";
    request.kind = core::ports::text_recognition::ExtractRequest::Kind::Table;
    request.cells.push_back({{0, 0, 800, 220}, 0, 0, {}, 0});

    const auto result = recognizer->extract(request);

    EXPECT_FALSE(trimCopy(result.text).empty());
    EXPECT_FALSE(result.words.empty());
    ASSERT_EQ(result.tables.size(), 1u);
    ASSERT_EQ(result.tables.front().cells.size(), 1u);
    EXPECT_FALSE(trimCopy(result.tables.front().cells.front().text).empty());
}

} // namespace
} // namespace infra::text_recognition::tests
