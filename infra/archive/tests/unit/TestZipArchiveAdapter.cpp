/**
 * @file infra/archive/tests/unit/TestZipArchiveAdapter.cpp
 * @brief Tests for the ZIP archive adapter.
 */

#include <gtest/gtest.h>

#include "archive/ZipArchiveAdapter.h"
#include "core/ports/diagnostics/IErrorReporter.h"

#include <zip.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

namespace infra::archive::tests {
namespace {

class NoopErrorReporter final
    : public core::ports::diagnostics::IErrorReporter {
public:
    void report(const core::errors::ErrorEvent&) override {}
};

std::shared_ptr<core::ports::diagnostics::IErrorReporter> makeErrorReporter()
{
    return std::make_shared<NoopErrorReporter>();
}

std::filesystem::path makeTempDir(const std::string& stem) {
    auto dir = std::filesystem::temp_directory_path() / stem;
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

void writeFile(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << text;
}

} // namespace

TEST(ZipArchiveAdapterTest, CreatesArchiveAndPreservesRelativePaths) {
    const auto tempRoot = makeTempDir("fossredder_zip_archive_test");
    const auto sourceDir = tempRoot / "source";
    const auto nestedDir = sourceDir / "nested";
    const auto archivePath = tempRoot / "out.zip";

    writeFile(sourceDir / "alpha.txt", "alpha");
    writeFile(nestedDir / "beta.txt", "beta");

    ZipArchiveAdapter adapter(makeErrorReporter());
    ASSERT_TRUE(adapter.create(sourceDir, archivePath, core::ports::exporting::PackageFormat::Zip));
    ASSERT_TRUE(std::filesystem::exists(archivePath));
    ASSERT_GT(std::filesystem::file_size(archivePath), 0u);

    int err = 0;
    zip_t* archive = zip_open(archivePath.string().c_str(), ZIP_RDONLY, &err);
    ASSERT_NE(archive, nullptr);

    const zip_int64_t entryCount = zip_get_num_entries(archive, 0);
    EXPECT_EQ(entryCount, 2);
    EXPECT_NE(zip_name_locate(archive, "alpha.txt", 0), -1);
    EXPECT_NE(zip_name_locate(archive, "nested/beta.txt", 0), -1);

    zip_close(archive);
}

TEST(ZipArchiveAdapterTest, RejectsUnsupportedFormat) {
    const auto tempRoot = makeTempDir("fossredder_zip_archive_unsupported");
    const auto sourceDir = tempRoot / "source";
    const auto archivePath = tempRoot / "out.zip";

    writeFile(sourceDir / "alpha.txt", "alpha");

    ZipArchiveAdapter adapter(makeErrorReporter());
    EXPECT_FALSE(adapter.create(sourceDir, archivePath, core::ports::exporting::PackageFormat::None));
    EXPECT_FALSE(std::filesystem::exists(archivePath));
}

} // namespace infra::archive::tests
