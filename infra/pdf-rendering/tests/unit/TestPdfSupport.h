/**
 * @file infra/pdf-rendering/tests/unit/TestPdfSupport.h
 * @brief Shared helpers for PDF rendering tests.
 */

#pragma once

#include <filesystem>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace infra::pdf_rendering::tests {
namespace support {

inline std::filesystem::path makeTempDir(const std::string& stem) {
    auto dir = std::filesystem::temp_directory_path() / stem;
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

inline std::filesystem::path writeSimplePdf(const std::filesystem::path& path, const std::string& text) {
    std::ostringstream content;
    content << "BT /F1 24 Tf 24 120 Td (" << text << ") Tj ET\n";
    const std::string stream = content.str();

    std::ostringstream pdf;
    std::vector<long> offsets;
    offsets.reserve(6);

    pdf << "%PDF-1.4\n";

    auto appendObject = [&](int id, const std::string& body) {
        offsets.push_back(static_cast<long>(pdf.tellp()));
        pdf << id << " 0 obj\n" << body << "\nendobj\n";
    };

    appendObject(1, "<< /Type /Catalog /Pages 2 0 R >>");
    appendObject(2, "<< /Type /Pages /Kids [3 0 R] /Count 1 >>");
    appendObject(3, "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 200 200] /Contents 4 0 R /Resources << /Font << /F1 5 0 R >> >> >>");
    appendObject(4, "<< /Length " + std::to_string(stream.size()) + " >>\nstream\n" + stream + "endstream");
    appendObject(5, "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>");

    const long xrefOffset = static_cast<long>(pdf.tellp());
    pdf << "xref\n0 6\n";
    pdf << "0000000000 65535 f \n";
    for (const auto offset : offsets) {
        pdf << std::setw(10) << std::setfill('0') << offset << " 00000 n \n";
    }
    pdf << "trailer\n<< /Size 6 /Root 1 0 R >>\n";
    pdf << "startxref\n" << xrefOffset << "\n%%EOF\n";

    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << pdf.str();
    return path;
}

} // namespace support
} // namespace infra::pdf_rendering::tests
