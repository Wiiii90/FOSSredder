/**
 * @file infra/text-recognition/src/TesseractCore.cpp
 * @brief Implements Tesseract OCR extraction helpers.
 */

#include "text-recognition/pch.h"
#include "text-recognition/TesseractCore.h"
#include "core/ports/diagnostics/IDiagnostics.h"
#include "core/ports/infra/text-recognition/TextRecognitionTypes.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <array>
#include <filesystem>
#include <cstdlib>
#include <system_error>
#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#endif

using namespace std;
namespace ports = core::ports::text_recognition;

static Pix* pixFromBytes(const std::vector<uint8_t>& data) {
    if (data.empty()) return nullptr;
    Pix* p = pixReadMem(data.data(), static_cast<size_t>(data.size()));
    return p;
}

static std::string readEnvironmentVariable(const char* name) {
#if defined(_MSC_VER)
    char* value = nullptr;
    size_t valueLength = 0;
    if (_dupenv_s(&value, &valueLength, name) != 0 || value == nullptr) {
        return {};
    }

    std::string result(value, valueLength > 0 ? valueLength - 1 : 0);
    free(value);
    return result;
#else
    if (const char* value = std::getenv(name)) {
        return std::string(value);
    }

    return {};
#endif
}

static std::filesystem::path executableDirectory() {
#if defined(_WIN32)
    std::array<wchar_t, MAX_PATH> buffer{};
    const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        return {};
    }

    return std::filesystem::path(buffer.data()).parent_path();
#else
    return {};
#endif
}

static std::string resolveTessdataPath(const std::string& provided) {
    auto existsNoThrow = [](const std::filesystem::path& p) {
        std::error_code ec;
        return !p.empty() && std::filesystem::exists(p, ec) && !ec;
    };

    if (!provided.empty()) {
        std::filesystem::path p(provided);
        if (existsNoThrow(p)) return p.string();
    }

    const std::string envTessdata = readEnvironmentVariable("TESSDATA_PREFIX");
    if (!envTessdata.empty()) {
        if (existsNoThrow(envTessdata)) return envTessdata;
    }

    std::vector<std::filesystem::path> candidates;
    const auto exeDir = executableDirectory();
    if (!exeDir.empty()) {
        candidates.push_back(exeDir / "res" / "tessdata");
        candidates.push_back(exeDir / "tessdata");
        candidates.push_back(exeDir.parent_path() / "res" / "tessdata");
        candidates.push_back(exeDir.parent_path() / "tessdata");
    }
    candidates.push_back(std::filesystem::current_path() / "res" / "tessdata");
    candidates.push_back(std::filesystem::current_path() / "tessdata");
    candidates.push_back(std::filesystem::path(__FILE__).parent_path().parent_path() / "res" / "tessdata");
    candidates.push_back(std::filesystem::path(__FILE__).parent_path().parent_path() / "tessdata");

    for (const auto &c : candidates) {
        if (existsNoThrow(c)) return c.string();
    }

    return std::string();
}

static tesseract::OcrEngineMode toTesseractOem(ports::EngineMode mode) {
    switch (mode) {
    case ports::EngineMode::LegacyOnly:
        return tesseract::OEM_TESSERACT_ONLY;
    case ports::EngineMode::LstmOnly:
        return tesseract::OEM_LSTM_ONLY;
    case ports::EngineMode::LegacyAndLstm:
        return tesseract::OEM_TESSERACT_LSTM_COMBINED;
    case ports::EngineMode::Default:
    default:
        return tesseract::OEM_DEFAULT;
    }
}

static std::string resolveLanguage(const ports::Settings& recognition) {
    return recognition.language.empty() ? std::string("deu") : recognition.language;
}

static void configureForStatements(tesseract::TessBaseAPI& ocr, const ports::Settings& recognition) {
    ocr.SetVariable("preserve_interword_spaces", recognition.preserveInterwordSpaces ? "1" : "0");

    if (!recognition.charWhitelist.empty()) {
        ocr.SetVariable("tessedit_char_whitelist", recognition.charWhitelist.c_str());
    }
}

static void applyPsm(tesseract::TessBaseAPI& ocr, int psm) {
    const int maxPsm = static_cast<int>(tesseract::PSM_COUNT) - 1;
    if (psm < 0 || psm > maxPsm) {
        psm = 3;
    }
    ocr.SetPageSegMode(static_cast<tesseract::PageSegMode>(psm));
}

static ports::Text recognizeTextFromBytes(const std::vector<uint8_t>& data,
                                                   const std::string& tessdataPath,
                                                   const ports::Settings& recognition,
                                                   std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg) {
    ports::Text out;
    tesseract::TessBaseAPI ocr;
    std::string resolved = resolveTessdataPath(tessdataPath);
    const std::string language = resolveLanguage(recognition);
    if (!resolved.empty()) ocr.SetVariable("TESSDATA_PREFIX", resolved.c_str());
    if (resolved.empty() || ocr.Init(resolved.c_str(), language.c_str(), toTesseractOem(recognition.engineMode)) != 0) {
        // last resort: try default Init without explicit path
        if (ocr.Init(nullptr, language.c_str(), toTesseractOem(recognition.engineMode)) != 0) return out;
    }

    configureForStatements(ocr, recognition);
    applyPsm(ocr, recognition.psm);

    Pix* pix = pixFromBytes(data);
    if (!pix) { ocr.End(); return out; }
    ocr.SetImage(pix);
    ocr.Recognize(0);
    char* txt = ocr.GetUTF8Text();
    if (txt) { out.text = string(txt); delete[] txt; }
    out.meanConfidence = ocr.MeanTextConf();
    if (dbg && dbg->enabled()) dbg->writeText(string("tesseract/output/text.txt"), out.text);
    ocr.End(); pixDestroy(&pix);
    return out;
}

static vector<ports::Word> getWordsFromBytes(const std::vector<uint8_t>& data,
                                                      const std::string& tessdataPath,
                                                      const ports::Settings& recognition,
                                                      std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg) {
    std::vector<ports::Word> out;
    Pix* pix = pixFromBytes(data);
    if (!pix) return out;
    tesseract::TessBaseAPI ocr;
    std::string resolved = resolveTessdataPath(tessdataPath);
    const std::string language = resolveLanguage(recognition);
    if (!resolved.empty()) ocr.SetVariable("TESSDATA_PREFIX", resolved.c_str());
    if (resolved.empty() || ocr.Init(resolved.c_str(), language.c_str(), toTesseractOem(recognition.engineMode)) != 0) {
        if (ocr.Init(nullptr, language.c_str(), toTesseractOem(recognition.engineMode)) != 0) { pixDestroy(&pix); return out; }
    }

    configureForStatements(ocr, recognition);
    applyPsm(ocr, recognition.psm);

    ocr.SetImage(pix);
    ocr.Recognize(0);

    // Stable extraction via TSV (word level == 5).
    char* tsv = ocr.GetTSVText(0);
    if (!tsv) { ocr.End(); pixDestroy(&pix); return out; }
    string s(tsv);
    if (dbg && dbg->enabled()) dbg->writeText(string("tesseract/tsv/words.tsv"), s);

    istringstream iss(s);
    string line;
    bool first = true;
    while (getline(iss, line)) {
        if (first) { first = false; continue; }
        if (line.empty()) continue;
        vector<string> cols;
        string cell;
        istringstream ls(line);
        while (getline(ls, cell, '\t')) cols.push_back(cell);
        if (cols.size() < 12) continue;
        int level = 0;
        try { level = stoi(cols[0]); } catch (...) { continue; }
        if (level != 5) continue;
        int left = 0, top = 0, width = 0, height = 0, conf = 0;
        try {
            left = stoi(cols[6]);
            top = stoi(cols[7]);
            width = stoi(cols[8]);
            height = stoi(cols[9]);
            conf = stoi(cols[10]);
        } catch (...) {
            continue;
        }
        string text = cols[11];
        ports::Word w;
        w.bbox.x = left;
        w.bbox.y = top;
        w.bbox.width = width;
        w.bbox.height = height;
        w.text = text;
        w.confidence = conf;
        out.push_back(std::move(w));
    }

    delete[] tsv;
    ocr.End();
    pixDestroy(&pix);
    return out;
}

std::pair<ports::Text, std::vector<ports::Word>> TesseractCore::extractFromBytes(
    const std::vector<uint8_t>& data,
    const std::string& tessdataPath,
    const ports::Settings& recognition,
    std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg) {
    auto t = recognizeTextFromBytes(data, tessdataPath, recognition, dbg);
    auto w = getWordsFromBytes(data, tessdataPath, recognition, dbg);
    return {t, w};
}
