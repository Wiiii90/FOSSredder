/**
 * @file core/include/core/domain/policies/AnalysisPolicy.h
 * @brief Shared analysis validation and normalization helpers.
 */

#pragma once

#include <string>
#include <utility>

#include "core/constants/analysis.h"
#include "core/domain/values/AnalysisType.h"
#include "core/domain/values/ExportFormat.h"
#include "core/domain/values/FilterSpec.h"

namespace core::domain::policies::analysis {

/**
 * @brief Checks whether an analysis type is supported at all.
 * @param type Raw analysis type.
 * @return `true` when the normalized type is non-empty.
 */
bool supportsResultType(const std::string& type);

/**
 * @brief Checks whether an analysis type produces tabular output.
 * @param type Raw analysis type.
 * @return `true` when the normalized type equals the tabular type key.
 */
bool isTabularType(const std::string& type);

/**
 * @brief Checks whether an analysis type produces chart-like output.
 * @param type Raw analysis type.
 * @return `true` for supported chart-like types.
 */
bool isChartLikeType(const std::string& type);

/**
 * @brief Checks whether an analysis type can be exported.
 * @param type Raw analysis type.
 * @param exportFormat Raw export format.
 * @return `true` when both type and export format are set.
 */
bool isExportable(const std::string& type, const std::string& exportFormat);

/**
 * @brief Normalizes filter keys used by analysis adjustments.
 * @param key Raw adjustment key.
 * @return Normalized key text.
 */
std::string normalizeKey(std::string key);

/**
 * @brief Checks whether an analysis is sufficiently configured.
 * @param type Raw analysis type.
 * @param configJson Raw configuration payload.
 * @param filterSpec Raw filter specification.
 * @param exportFormat Raw export format.
 * @return `true` when any meaningful analysis configuration is present.
 */
bool isConfigured(const std::string& type,
                  const std::string& configJson,
                  const std::string& filterSpec,
                  const std::string& exportFormat);

/**
 * @brief Resolves the effective output type for an analysis.
 * @param type Raw analysis type.
 * @param configJson Raw configuration payload.
 * @return Resolved output type key.
 */
std::string resolveOutputType(const std::string& type, const std::string& configJson);

/**
 * @brief Resolves the effective execution type for an analysis.
 * @param type Raw analysis type.
 * @return Effective execution type key.
 */
std::string resolveExecutionType(const std::string& type);

} // namespace core::domain::policies::analysis
