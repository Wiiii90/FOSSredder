/**
 * @file core/include/core/ports/infra/text-recognition/TextRecognitionTypes.h
 * @brief Shared text recognition data types.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core::ports::text_recognition {

struct Text {
    std::string text;
    int meanConfidence = 0;
};

enum class EngineMode {
    Default,
    LegacyOnly,
    LstmOnly,
    LegacyAndLstm,
};

struct Settings {
    std::string language;
    int psm = 3;
    bool preserveInterwordSpaces = false;
    std::string charWhitelist;
    EngineMode engineMode = EngineMode::Default;
};

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

struct Word {
    Rect bbox;
    std::string text;
    int confidence = 0;
};

struct Cell {
    Rect bbox;
    int row = -1;
    int col = -1;
    std::string text;
    int confidence = 0;
};

struct Table {
    Rect bbox;
    int rows = 0;
    int cols = 0;
    std::vector<Cell> cells;
    std::vector<Word> words;
};

} // namespace core::ports::text_recognition
