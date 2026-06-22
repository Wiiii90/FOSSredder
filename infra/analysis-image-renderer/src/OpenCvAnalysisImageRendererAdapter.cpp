/**
 * @file infra/analysis-image-renderer/src/OpenCvAnalysisImageRendererAdapter.cpp
 * @brief Implements OpenCV analysis image rendering.
 */

#include "analysis-image-renderer/OpenCvAnalysisImageRendererAdapter.h"

#include "core/constants/analysis.h"
#include "core/constants/export.h"
#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporting.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <nlohmann/json.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace {
// Reuse the existing rendering logic as-is.
double parseNumber(const std::string& text)
{
    try {
        return std::stod(text);
    } catch (...) {
        return 0.0;
    }
}

bool tryParseJsonObject(const std::string& text, nlohmann::json& out)
{
    try {
        out = nlohmann::json::parse(text);
        return out.is_object();
    } catch (...) {
        return false;
    }
}

std::int32_t qmlHashString(const std::string& value)
{
    std::int32_t hash = 0;
    for (const unsigned char ch : value) {
        hash = static_cast<std::int32_t>((hash << 5) - hash + ch);
    }
    return hash < 0 ? -hash : hash;
}

const std::vector<cv::Scalar>& qmlChartPalette()
{
    static const std::vector<cv::Scalar> colors = {
        cv::Scalar(199, 211, 141, 255), // #8dd3c7
        cv::Scalar(179, 255, 255, 255), // #ffffb3
        cv::Scalar(218, 186, 190, 255), // #bebada
        cv::Scalar(114, 128, 251, 255), // #fb8072
        cv::Scalar(211, 177, 128, 255), // #80b1d3
        cv::Scalar(98, 180, 253, 255),  // #fdb462
        cv::Scalar(105, 222, 179, 255), // #b3de69
        cv::Scalar(229, 205, 252, 255), // #fccde5
        cv::Scalar(217, 217, 217, 255), // #d9d9d9
        cv::Scalar(189, 128, 188, 255), // #bc80bd
        cv::Scalar(197, 235, 204, 255), // #ccebc5
        cv::Scalar(111, 237, 255, 255), // #ffed6f
    };
    return colors;
}

cv::Scalar colorForIndex(std::size_t index)
{
    const auto& colors = qmlChartPalette();
    if (colors.empty()) return cv::Scalar(119, 170, 221, 255);
    return colors[index % colors.size()];
}

cv::Scalar colorForKey(const std::string& key)
{
    const auto& colors = qmlChartPalette();
    if (colors.empty()) return cv::Scalar(119, 170, 221, 255);
    const auto index = static_cast<std::size_t>(qmlHashString(key)) % colors.size();
    return colorForIndex(index);
}

double renderScale(const cv::Mat& image)
{
    return std::max(1.0, static_cast<double>(image.cols) / 1280.0);
}

int scaled(const cv::Mat& image, int value)
{
    return static_cast<int>(std::round(value * renderScale(image)));
}

double scaledFont(const cv::Mat& image, double value)
{
    return value * renderScale(image);
}

int chartFontFace()
{
    return cv::FONT_HERSHEY_SIMPLEX;
}

void drawRotatedText(cv::Mat& image,
                     const std::string& text,
                     const cv::Point& topLeft,
                     double fontScale,
                     const cv::Scalar& color,
                     int thickness)
{
    if (text.empty()) return;

    int baseline = 0;
    const auto textSize = cv::getTextSize(text, chartFontFace(), fontScale, thickness, &baseline);
    cv::Mat textImage(textSize.height + baseline + 8, textSize.width + 8, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    cv::putText(textImage,
                text,
                cv::Point(4, textSize.height + 2),
                chartFontFace(),
                fontScale,
                color,
                thickness,
                cv::LINE_AA);

    cv::Mat rotated;
    cv::rotate(textImage, rotated, cv::ROTATE_90_CLOCKWISE);

    for (int y = 0; y < rotated.rows; ++y) {
        const int targetY = topLeft.y + y;
        if (targetY < 0 || targetY >= image.rows) continue;
        for (int x = 0; x < rotated.cols; ++x) {
            const int targetX = topLeft.x + x;
            if (targetX < 0 || targetX >= image.cols) continue;

            const cv::Vec4b source = rotated.at<cv::Vec4b>(y, x);
            if (source[3] == 0) continue;
            if (image.channels() == 4) {
                image.at<cv::Vec4b>(targetY, targetX) = source;
            } else {
                image.at<cv::Vec3b>(targetY, targetX) = cv::Vec3b(source[0], source[1], source[2]);
            }
        }
    }
}

std::vector<std::vector<std::string>> normalizedRowsForImage(const core::ports::analysis::AnalysisResult& result)
{
    using core::constants::analysis::resultFields::kAmountAdjusted;
    using core::constants::analysis::resultFields::kAmountOriginal;
    using core::constants::analysis::resultFields::kMonth;
    using core::constants::analysis::resultFields::kTaxFactor;
    using core::constants::analysis::resultFields::kTaxPercent;
    using core::constants::analysis::resultFields::kTotal;
    using core::constants::analysis::resultFields::kTransactionId;

    std::vector<std::vector<std::string>> rows;
    if (result.type == core::constants::analysis::plotTypes::kPie) {
        rows.push_back({"Category", "Value"});
        for (const auto& row : result.table) {
            if (row.empty()) continue;
            if (row.size() == 1) rows.push_back({row[0], "0"});
            else rows.push_back({row[0], row[1]});
        }
        return rows;
    }

    if (result.type == core::constants::analysis::plotTypes::kHistogram) {
        rows.push_back({"Month", "Total"});
        for (const auto& row : result.table) {
            if (row.empty()) continue;
            std::string month = row[0];
            std::string totalValue = "0";
            if (row.size() > 1) {
                nlohmann::json summary;
                if (tryParseJsonObject(row[1], summary)) {
                    if (summary.contains(kMonth) && summary[kMonth].is_string()) {
                        month = summary[kMonth].get<std::string>();
                    }
                    if (summary.contains(kTotal) && summary[kTotal].is_number()) {
                        totalValue = std::to_string(summary[kTotal].get<double>());
                    }
                } else {
                    totalValue = row[1];
                }
            }
            rows.push_back({month, totalValue});
        }
        return rows;
    }

    if (result.type == core::constants::analysis::kTypeCalculation) {
        rows.push_back({"Label", "OriginalAmount", "AdjustedAmount", "TaxPercent", "TaxFactor", "TransactionId"});
        for (const auto& row : result.table) {
            if (row.empty()) continue;

            std::string label = row[0];
            std::string original = "";
            std::string adjusted = "";
            std::string taxPercent = "";
            std::string taxFactor = "";
            std::string txId = "";

            if (row.size() > 1) {
                nlohmann::json summary;
                if (tryParseJsonObject(row[1], summary)) {
                    if (summary.contains(kAmountOriginal) && summary[kAmountOriginal].is_number()) {
                        original = std::to_string(summary[kAmountOriginal].get<double>());
                    }
                    if (summary.contains(kAmountAdjusted) && summary[kAmountAdjusted].is_number()) {
                        adjusted = std::to_string(summary[kAmountAdjusted].get<double>());
                    }
                    if (summary.contains(kTaxPercent) && summary[kTaxPercent].is_number()) {
                        taxPercent = std::to_string(summary[kTaxPercent].get<double>());
                    }
                    if (summary.contains(kTaxFactor) && summary[kTaxFactor].is_number()) {
                        taxFactor = std::to_string(summary[kTaxFactor].get<double>());
                    }
                    if (summary.contains(kTransactionId) && summary[kTransactionId].is_string()) {
                        txId = summary[kTransactionId].get<std::string>();
                    }
                }
            }

            rows.push_back({label, original, adjusted, taxPercent, taxFactor, txId});
        }
        return rows;
    }

    for (const auto& row : result.table) {
        rows.push_back(row);
    }

    return rows;
}

bool drawTableImage(cv::Mat& image, const std::vector<std::vector<std::string>>& rows)
{
    if (rows.empty()) return false;

    int y = scaled(image, 150);
    const int lineHeight = scaled(image, 30);
    const int maxRows = 16;
    for (size_t r = 0; r < rows.size() && static_cast<int>(r) < maxRows; ++r) {
        std::ostringstream line;
        for (size_t c = 0; c < rows[r].size(); ++c) {
            if (c > 0) line << " | ";
            line << rows[r][c];
        }
        cv::putText(image,
                    line.str(),
                    cv::Point(scaled(image, 50), y),
                    chartFontFace(),
                    r == 0 ? scaledFont(image, 0.68) : scaledFont(image, 0.55),
                    r == 0 ? cv::Scalar(30, 30, 30, 255) : cv::Scalar(70, 70, 70, 255),
                    r == 0 ? scaled(image, 2) : std::max(1, scaled(image, 1)),
                    cv::LINE_AA);
        y += lineHeight;
    }
    return true;
}

bool drawPieChartImage(cv::Mat& image, const core::ports::analysis::AnalysisResult& result)
{
    struct Slice {
        std::string label;
        double value = 0.0;
    };

    std::vector<Slice> slices;
    for (const auto& row : result.table) {
        if (row.empty()) continue;
        const std::string label = row[0];
        const double value = row.size() > 1 ? std::fabs(parseNumber(row[1])) : 0.0;
        if (value > 0.0) slices.push_back({label, value});
    }
    if (slices.empty()) return false;

    double total = 0.0;
    for (const auto& slice : slices) total += slice.value;
    if (total <= 0.0) return false;

    const int width = image.cols;
    const int height = image.rows;
    const int margin = scaled(image, 24);
    const int legendWidth = std::max(320, width / 3);
    const int pieAreaWidth = width - legendWidth - (margin * 3);
    const int radiusPx = std::max(scaled(image, 120), std::min(pieAreaWidth / 2, height / 2 - margin));
    const cv::Point center(margin + radiusPx, margin + radiusPx);
    const cv::Size radius(radiusPx, radiusPx);

    double startAngle = -90.0;
    for (size_t i = 0; i < slices.size(); ++i) {
        const double angle = 360.0 * (slices[i].value / total);
        const cv::Scalar color = colorForIndex(i);
        cv::ellipse(image, center, radius, 0.0, startAngle, startAngle + angle, color, cv::FILLED, cv::LINE_AA);
        startAngle += angle;
    }

    const int legendX = margin + (radiusPx * 2) + margin;
    int legendY = margin + scaled(image, 20);
    const int maxLegendRows = std::max(1, (height - margin) / scaled(image, 30));
    for (size_t i = 0; i < slices.size() && static_cast<int>(i) < maxLegendRows; ++i) {
        const cv::Scalar color = colorForIndex(i);
        cv::rectangle(image, cv::Point(legendX, legendY - scaled(image, 16)), cv::Point(legendX + scaled(image, 20), legendY + scaled(image, 4)), color, cv::FILLED, cv::LINE_AA);
        std::ostringstream label;
        label << slices[i].label << "  " << std::fixed << std::setprecision(2) << slices[i].value
              << "  " << std::setprecision(1) << (slices[i].value * 100.0 / total) << "%";
        cv::putText(image, label.str(), cv::Point(legendX + scaled(image, 30), legendY), chartFontFace(), scaledFont(image, 0.55), cv::Scalar(50, 50, 50, 255), std::max(1, scaled(image, 1)), cv::LINE_AA);
        legendY += scaled(image, 30);
    }

    return true;
}

bool drawHistogramImage(cv::Mat& image, const core::ports::analysis::AnalysisResult& result)
{
    struct Bucket {
        std::string label;
        double value = 0.0;
        std::unordered_map<std::string, double> byContract;
        std::unordered_map<std::string, double> byProperty;
    };

    std::vector<Bucket> buckets;
    std::vector<std::string> categories;
    std::vector<std::string> propertyCategories;
    for (const auto& row : result.table) {
        if (row.empty()) continue;

        std::string month = row[0];
        double total = 0.0;
        if (row.size() > 1) {
            nlohmann::json summary;
            if (tryParseJsonObject(row[1], summary)) {
                if (summary.contains(core::constants::analysis::resultFields::kMonth)
                    && summary[core::constants::analysis::resultFields::kMonth].is_string()) {
                    month = summary[core::constants::analysis::resultFields::kMonth].get<std::string>();
                }
                if (summary.contains(core::constants::analysis::resultFields::kTotal)
                    && summary[core::constants::analysis::resultFields::kTotal].is_number()) {
                    total = std::fabs(summary[core::constants::analysis::resultFields::kTotal].get<double>());
                }
                if (summary.contains("byContract") && summary["byContract"].is_object()) {
                    for (const auto& item : summary["byContract"].items()) {
                        const std::string key = item.key();
                        if (std::find(categories.begin(), categories.end(), key) == categories.end()) {
                            categories.push_back(key);
                        }
                    }
                }
                if (summary.contains("byProperty") && summary["byProperty"].is_object()) {
                    for (const auto& item : summary["byProperty"].items()) {
                        const std::string key = item.key();
                        if (std::find(propertyCategories.begin(), propertyCategories.end(), key) == propertyCategories.end()) {
                            propertyCategories.push_back(key);
                        }
                    }
                }
            } else {
                total = std::fabs(parseNumber(row[1]));
            }
        }

        Bucket bucket;
        bucket.label = month;
        bucket.value = total;
        if (row.size() > 1) {
            nlohmann::json summary;
            if (tryParseJsonObject(row[1], summary) && summary.contains("byContract") && summary["byContract"].is_object()) {
                for (const auto& item : summary["byContract"].items()) {
                    if (item.value().is_number()) {
                        bucket.byContract[item.key()] = std::fabs(item.value().get<double>());
                    }
                }
            }
            if (tryParseJsonObject(row[1], summary) && summary.contains("byProperty") && summary["byProperty"].is_object()) {
                for (const auto& item : summary["byProperty"].items()) {
                    if (item.value().is_number()) {
                        bucket.byProperty[item.key()] = std::fabs(item.value().get<double>());
                    }
                }
            }
        }
        if (bucket.value <= 0.0 && !bucket.byContract.empty()) {
            for (const auto& [_, value] : bucket.byContract) {
                bucket.value += value;
            }
        }
        buckets.push_back(std::move(bucket));
    }
    if (buckets.empty()) return false;
    if (categories.empty()) categories.push_back("Total");

    const int width = image.cols;
    const int height = image.rows;
    const int margin = scaled(image, 24);
    std::size_t longestPropertyLabel = 0;
    for (const auto& property : propertyCategories) {
        longestPropertyLabel = std::max(longestPropertyLabel, property.size());
    }
    const int legendTop = height - scaled(image, 132);
    const int labelBandHeight = propertyCategories.size() > 1
        ? std::min(scaled(image, 360), scaled(image, 42) + static_cast<int>(longestPropertyLabel) * scaled(image, 10))
        : scaled(image, 46);
    const int left = margin;
    const int right = width - margin;
    const int top = margin;
    const int bottom = legendTop - labelBandHeight;

    double maxValue = 0.0;
    for (const auto& bucket : buckets) maxValue = std::max(maxValue, bucket.value);
    if (maxValue <= 0.0) maxValue = 1.0;

    const int count = static_cast<int>(buckets.size());
    const int slotWidth = std::max(scaled(image, 20), (right - left - scaled(image, 20)) / std::max(1, count));
    const int barWidth = std::max(scaled(image, 10), slotWidth - scaled(image, 34));
    const cv::Scalar barColor = colorForKey("Total");
    const bool splitByProperty = propertyCategories.size() > 1;
    for (int i = 0; i < count; ++i) {
        const auto& bucket = buckets[static_cast<size_t>(i)];
        const int x1 = left + (i * slotWidth) + ((slotWidth - barWidth) / 2);
        if (splitByProperty && !bucket.byProperty.empty()) {
            const int gap = scaled(image, 5);
            const int propertyCount = static_cast<int>(propertyCategories.size());
            const int propertyBarWidth = std::max(scaled(image, 5), (barWidth - (gap * std::max(0, propertyCount - 1))) / std::max(1, propertyCount));
            double contractTotal = 0.0;
            for (const auto& [_, value] : bucket.byContract) contractTotal += value;
            if (contractTotal <= 0.0) contractTotal = bucket.value;

            for (int pi = 0; pi < propertyCount; ++pi) {
                const auto& property = propertyCategories[static_cast<size_t>(pi)];
                const auto propertyIt = bucket.byProperty.find(property);
                const double propertyValue = propertyIt == bucket.byProperty.end() ? 0.0 : propertyIt->second;
                const int px1 = x1 + (pi * (propertyBarWidth + gap));
                const int px2 = px1 + propertyBarWidth;
                int baseline = bottom;

                if (bucket.byContract.empty()) {
                    const int barHeight = static_cast<int>(((bottom - top - 10) * propertyValue) / maxValue);
                    cv::rectangle(image, cv::Point(px1, bottom - barHeight), cv::Point(px2, bottom - 1), barColor, cv::FILLED, cv::LINE_AA);
                } else {
                    for (const auto& category : categories) {
                        const auto contractIt = bucket.byContract.find(category);
                        if (contractIt == bucket.byContract.end()) continue;
                        const double segmentValue = contractTotal > 0.0 ? propertyValue * (contractIt->second / contractTotal) : 0.0;
                        const int segmentHeight = static_cast<int>(((bottom - top - 10) * segmentValue) / maxValue);
                        const auto colorIndex = static_cast<std::size_t>(std::distance(categories.begin(), std::find(categories.begin(), categories.end(), category)));
                        cv::rectangle(image, cv::Point(px1, baseline - segmentHeight), cv::Point(px2, baseline - 1), colorForIndex(colorIndex), cv::FILLED, cv::LINE_AA);
                        baseline -= segmentHeight;
                    }
                }

                drawRotatedText(image,
                                property,
                                cv::Point(px1 + std::max(0, (propertyBarWidth - scaled(image, 14)) / 2),
                                          bottom + scaled(image, 26)),
                                scaledFont(image, 0.36),
                                cv::Scalar(70, 70, 70, 255),
                                std::max(1, scaled(image, 1)));
            }
        } else if (bucket.byContract.empty()) {
            const int barHeight = static_cast<int>(((bottom - top - 10) * bucket.value) / maxValue);
            cv::rectangle(image, cv::Point(x1, bottom - barHeight), cv::Point(x1 + barWidth, bottom - 1), barColor, cv::FILLED, cv::LINE_AA);
        } else {
            int baseline = bottom;
            for (const auto& category : categories) {
                const auto it = bucket.byContract.find(category);
                if (it == bucket.byContract.end()) continue;
                const int segmentHeight = static_cast<int>(((bottom - top - 10) * it->second) / maxValue);
                const auto colorIndex = static_cast<std::size_t>(std::distance(categories.begin(), std::find(categories.begin(), categories.end(), category)));
                cv::rectangle(image, cv::Point(x1, baseline - segmentHeight), cv::Point(x1 + barWidth, baseline - 1), colorForIndex(colorIndex), cv::FILLED, cv::LINE_AA);
                baseline -= segmentHeight;
            }
        }

        if (i % std::max(1, count / 12) == 0) {
            cv::putText(image, bucket.label, cv::Point(x1, bottom + scaled(image, 18)), chartFontFace(), scaledFont(image, 0.4), cv::Scalar(70, 70, 70, 255), std::max(1, scaled(image, 1)), cv::LINE_AA);
        }
    }

    int legendX = margin;
    int legendY = legendTop + scaled(image, 26);
    const int legendRight = width - margin;
    for (size_t i = 0; i < categories.size(); ++i) {
        const auto& category = categories[i];
        const int textWidth = std::max(scaled(image, 80), static_cast<int>(category.size()) * scaled(image, 9));
        const int itemWidth = textWidth + scaled(image, 48);
        if (legendX + itemWidth > legendRight) {
            legendX = margin;
            legendY += scaled(image, 30);
        }
        cv::rectangle(image, cv::Point(legendX, legendY - scaled(image, 16)), cv::Point(legendX + scaled(image, 20), legendY + scaled(image, 4)), colorForIndex(i), cv::FILLED, cv::LINE_AA);
        cv::putText(image, category, cv::Point(legendX + scaled(image, 30), legendY), chartFontFace(), scaledFont(image, 0.55), cv::Scalar(50, 50, 50, 255), std::max(1, scaled(image, 1)), cv::LINE_AA);
        legendX += itemWidth;
    }

    return true;
}

bool writeImageFromResult(const std::filesystem::path& outputPath,
                          const core::ports::analysis::AnalysisResult& result)
{
    constexpr int width = 2560;
    constexpr int height = 1440;
    const bool asPng = outputPath.has_extension()
        && outputPath.extension().string() == ".png";

    cv::Mat image;
    if (asPng) image = cv::Mat(height, width, CV_8UC4, cv::Scalar(0, 0, 0, 0));
    else image = cv::Mat(height, width, CV_8UC3, cv::Scalar(250, 250, 250));

    bool ok = false;
    if (result.type == core::constants::analysis::plotTypes::kPie) {
        ok = drawPieChartImage(image, result);
    } else if (result.type == core::constants::analysis::plotTypes::kHistogram) {
        ok = drawHistogramImage(image, result);
    } else {
        ok = drawTableImage(image, normalizedRowsForImage(result));
    }

    if (!ok) return false;

    return cv::imwrite(outputPath.string(), image);
}

} // namespace

namespace infra::analysis_image_renderer {

namespace {

void reportImageFailure(core::errors::IErrorReporter* reporter,
                        std::string message,
                        const std::filesystem::path& outputPath)
{
    if (!reporter) return;

    core::errors::ErrorEvent event;
    event.severity = core::errors::ErrorSeverity::Error;
    event.origin = "infra::analysis_image_renderer::OpenCvAnalysisImageRendererAdapter::writeAnalysisImage";
    event.message = std::move(message);
    event.code = core::errors::codes::GenericError;
    event.context = {{"path", outputPath.string()}};
    core::errors::report(reporter, event);
}

} // namespace

OpenCvAnalysisImageRendererAdapter::OpenCvAnalysisImageRendererAdapter(
    std::shared_ptr<core::errors::IErrorReporter> errorReporter)
    : errorReporter_(std::move(errorReporter))
{
}

bool OpenCvAnalysisImageRendererAdapter::writeAnalysisImage(const std::filesystem::path& outputPath,
                                                            const std::string& title,
                                                            const core::ports::analysis::AnalysisResult& result) const
{
    (void)title;
    try {
        const bool ok = writeImageFromResult(outputPath, result);
        if (!ok) {
            reportImageFailure(errorReporter_.get(), "failed to render analysis image", outputPath);
        }
        return ok;
    } catch (...) {
        core::errors::reportException(errorReporter_.get(),
                                      core::errors::ErrorSeverity::Error,
                                      "infra::analysis_image_renderer::OpenCvAnalysisImageRendererAdapter::writeAnalysisImage",
                                      std::current_exception());
        return false;
    }
}

} // namespace infra::analysis_image_renderer
