/**
 * @file core/tests/domain/values/TestExportFormat.cpp
 * @brief Tests for export format normalization and validation.
 */

#include <gtest/gtest.h>

#include "core/domain/values/ExportFormat.h"

namespace core::domain {

TEST(ExportFormatTest, TrimsWhitespaceAndKeepsStableValue) {
    const ExportFormat format{"  xlsx  "};

    EXPECT_EQ(format.value(), "xlsx");
    EXPECT_FALSE(format.empty());
    EXPECT_TRUE(ExportFormat::isValid("  xlsx  "));
}

TEST(ExportFormatTest, RejectsBlankInput) {
    EXPECT_FALSE(ExportFormat::isValid("   "));
    EXPECT_TRUE(ExportFormat{}.empty());
}

} // namespace core::domain
