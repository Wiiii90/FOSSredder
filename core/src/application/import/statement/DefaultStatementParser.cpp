/**
 * @file core/src/application/import/statement/DefaultStatementParser.cpp
 * @brief Implements statement import parsing and transaction block extraction.
 */

#include "core/pch.h"

#include "core/application/import/statement/DefaultStatementParser.h"

#include "core/application/import/internal/ParserConfig.h"
#include "core/application/import/transaction/DefaultTransactionParser.h"
#include "core/application/import/internal/ParserHeuristics.h"
#include "core/application/import/internal/ParserHelpers.h"
#include "core/application/import/statement/StatementParseHelpers.h"
#include "../../../utils/Util.h"

#include <algorithm>
#include <optional>
#include <regex>

namespace core::application::importing::statement {

namespace helpers = core::application::importing::internal;

using core::application::importing::transaction::DefaultTransactionParser;
using core::utils::lowerAscii;
using core::utils::trim;

namespace {

static bool isValutaHeaderLine(const std::string& line) {
    const auto n = helpers::normalizeAlnumLower(line);
    return n.find("valuta") != std::string::npos;
}

struct RowModel {
    bool inSection = false;
};

using helpers::isFooterLine;
using helpers::isHeaderNoiseLine;
using helpers::isPostTransactionFootnote;
using helpers::isTransactionsSectionHeader;
using helpers::isDebitCreditHeaderLine;

static bool isLikelyTransactionMainRowText(const std::string& line) {
    static const std::regex re(R"((\d{2}\.\s*\d{2})\s+\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?(?:\s+.*)?\s*$)");
    return std::regex_search(line, re);
}

}

DefaultStatementParser::ParseResult DefaultStatementParser::parse([[maybe_unused]] const core::ports::document_image_processing::Table& table,
                                                                  const core::ports::text_recognition::ExtractResult& ocr,
                                                                  const std::string& pageCropImagePath,
                                                                  std::shared_ptr<core::ports::document_image_processing::IDocumentImageProcessor> documentImageProcessor,
                                                                  const std::vector<uint8_t>& pageCropImageBytes,
                                                                  std::string initialBookingDate,
                                                                  int initialTransactionIndex) {
    (void)table;
    ParseResult out;

    out.debugLines.push_back(std::string("pageCropImagePath\t") + pageCropImagePath);
    out.debugLines.push_back(std::string("pageCropImageBytes\t") + std::to_string(pageCropImageBytes.size()));
    out.debugLines.push_back(std::string("initialBookingDate\t") + initialBookingDate);
    out.debugLines.push_back(std::string("initialTransactionIndex\t") + std::to_string(initialTransactionIndex));

    const auto ocrLines = helpers::buildOcrLinesFromWords(ocr.words);
    out.debugLines.push_back(std::string("ocr.words\t") + std::to_string(ocr.words.size()));
    out.debugLines.push_back(std::string("ocr.lines.raw\t") + std::to_string(ocrLines.size()));

    const auto seed = helpers::inferColumnModelFromLines(ocrLines);
    core::application::importing::statement::internal::ColumnModel seedCols{ seed.valutaX, seed.debitX, seed.creditX, -1, -1, -1 };

    std::vector<core::application::importing::internal::RawLineLite> rawLite;
    rawLite.reserve(ocrLines.size());
    for (const auto& l : ocrLines) rawLite.push_back({ l.minX, l.maxX, l.minY, l.maxY, l.wordSpans, l.text });

    const auto merged = core::application::importing::statement::internal::selectiveGroupMergeLinesRaw(rawLite, 8, core::application::importing::internal::ColumnGuess{ seedCols.valutaX, seedCols.debitX, seedCols.creditX });
    std::vector<core::application::importing::statement::internal::RawLine> lines;
    lines.reserve(merged.size());
    for (const auto& ml : merged) lines.push_back(core::application::importing::statement::internal::RawLine{0, ml.minX, ml.maxX, ml.minY, ml.maxY, ml.wordSpans, ml.text});
    out.debugLines.push_back(std::string("ocr.lines\t") + std::to_string(lines.size()));

    std::vector<core::application::importing::transaction::internal::OcrLine> ocrFromRaw;
    ocrFromRaw.reserve(lines.size());
    for (const auto& rl : lines) ocrFromRaw.push_back(core::application::importing::statement::internal::rawToOcrLine(rl));

    std::vector<std::pair<size_t,std::string>> pageHeaderDates;
    try {
        for (size_t i = 0; i < lines.size(); ++i) {
            try {
                if (auto bd = core::application::importing::statement::internal::findBookingDateInHeader(lines[i].text)) {
                    pageHeaderDates.emplace_back(i, *bd);
                }
            } catch (...) {  }
        }
    } catch (...) {  }

    std::string currentBookingDate = std::move(initialBookingDate);
    int txIndex = std::max(1, initialTransactionIndex);

    size_t headerScanPtr = 0;

    {
        std::string dbg;
        if (core::application::importing::statement::internal::detectEarlyEmptyPage(ocrLines, dbg)) {
            out.debugLines.push_back(dbg);
            out.debugLines.push_back(std::string("page.emptyDetected\tlines=") + std::to_string(12));
            out.lastBookingDate = currentBookingDate;
            out.nextTransactionIndex = txIndex;
            return out;
        }
        if (!dbg.empty()) out.debugLines.push_back(dbg);
    }

    std::vector<core::application::importing::transaction::internal::TransactionBlock> blocks;
    core::application::importing::transaction::internal::TransactionBlock cur;
    cur.bookingDateGroup = currentBookingDate;

    bool inTransactions = !currentBookingDate.empty();
    core::application::importing::statement::internal::ColumnModel cols = seedCols;
    RowModel rows;

    const core::application::importing::statement::internal::HeaderAnalysis headerAnalysis = core::application::importing::statement::internal::analyzeHeaderWindow(ocrLines, lines, seedCols, out);
    static constexpr size_t headerScanLines = 12;
    int headerBottomY = headerAnalysis.headerBottomY;
    const int headerMarginPx = headerAnalysis.headerMarginPx;
    const int pageMaxY = headerAnalysis.pageMaxY;
    const bool headerFound = headerAnalysis.headerFound;

    if (currentBookingDate.empty()) {
        if (auto d = core::application::importing::statement::internal::findDefaultBookingDate(ocrLines, 30)) {
            currentBookingDate = *d;
            out.debugLines.push_back(std::string("header.defaultBookingDate\t") + currentBookingDate);
            cur.bookingDateGroup = currentBookingDate;
        }
    }

    const auto flush = [&]() {
         if (cur.main.left.empty() && cur.detailLines.empty()) return;
         blocks.push_back(std::move(cur));
         cur = core::application::importing::transaction::internal::TransactionBlock{};
         cur.bookingDateGroup = currentBookingDate;
     };

    std::string prevLine;
    int txStartLooseCount = 0;

    std::vector<core::application::importing::transaction::internal::OcrLine> orphanLines;

    for (size_t li = 0; li < lines.size(); ++li) {
        const auto& l = lines[li];
        const auto txt = l.text;
        const auto combined = prevLine.empty() ? txt : (prevLine + " " + txt);
        if (txt.empty()) continue;

        if (headerScanPtr < pageHeaderDates.size() && pageHeaderDates[headerScanPtr].first == li) {
            currentBookingDate = pageHeaderDates[headerScanPtr].second;
            out.debugLines.push_back(std::string("header.foundOnPage\tline=") + std::to_string(li) + std::string("\t") + currentBookingDate);
            ++headerScanPtr;
            inTransactions = true;
            flush();
            cur.bookingDateGroup = currentBookingDate;
            prevLine = txt;
            continue;
        }
        while (headerScanPtr < pageHeaderDates.size() && pageHeaderDates[headerScanPtr].first < li) ++headerScanPtr;

        if (isFooterLine(txt)) { out.debugLines.push_back(std::string("stop.footer\t") + txt + "\tline=" + std::to_string(li)); break; }
        if (isPostTransactionFootnote(txt)) {
            bool nearBottom = false;
            try { if (pageMaxY >= 0 && l.maxY >= static_cast<int>(pageMaxY * 3 / 4)) nearBottom = true; } catch (...) {  }
            try {
                out.debugLines.push_back(std::string("footnote.check\tpageMaxY=") + std::to_string(pageMaxY) + std::string("\tlineY=") + std::to_string(l.maxY) + std::string("\tnearBottom=") + (nearBottom ? "1" : "0") + std::string("\tblocks=") + std::to_string(blocks.size()));
            } catch (...) {  }

            bool reallyTerminal = false;
            try { if (!blocks.empty() && nearBottom) reallyTerminal = true; } catch (...) {  }
            if (reallyTerminal) {
                out.debugLines.push_back(std::string("stop.footnote\t") + txt + "\tline=" + std::to_string(li));
                break;
            }
            out.debugLines.push_back(std::string("footnote.ignored\t") + txt + "\tline=" + std::to_string(li));
            prevLine = txt;
            continue;
        }

        if (headerBottomY >= 0 && l.maxY <= headerBottomY + headerMarginPx && !inTransactions) {
            try {
                if (seedCols.valutaX >= 0) {
                    auto anchorOl = core::application::importing::statement::internal::rawToOcrLine(l);
                    if (helpers::hasTokenNearX(anchorOl, seedCols.valutaX, helpers::parserConfig.tokenNearBandForMainRow) || helpers::hasAmountNearValuta(anchorOl, seedCols.valutaX, helpers::parserConfig.amountNearValutaBandPx)) {
                        out.debugLines.push_back(std::string("header.overrideByValutaAnchor\tline=") + std::to_string(li) + "\ttext=" + txt);
                    } else {
                        continue;
                    }
                }
            } catch (...) {  }

            bool hasHeaderSignal = false;
            try {
                if (core::application::importing::statement::internal::findBookingDateInHeader(txt).has_value()) hasHeaderSignal = true;
            } catch (...) {  }
            try {
                if (core::application::importing::statement::internal::findBookingDateInHeader(combined).has_value()) hasHeaderSignal = true;
            } catch (...) {  }
            try {
                if (isValutaHeaderLine(txt)) hasHeaderSignal = true;
            } catch (...) {  }

            if (!hasHeaderSignal && !(rows.inSection && cols.hasValuta())) {
                bool combinedStarted = false;
                try {
                    std::string combPrev = prevLine.empty() ? txt : (prevLine + std::string(" ") + txt);
                    std::string combNext = txt;
                    if (li + 1 < lines.size()) combNext = txt + std::string(" ") + lines[li+1].text;
                    try {
                        if (auto cs = core::application::importing::statement::internal::tryCombinedStart(ocrFromRaw, li, core::application::importing::internal::ColumnGuess{ cols.valutaX, cols.debitX, cols.creditX })) {
                            inTransactions = true;
                            out.debugLines.push_back(std::string("tx.start.combined.aboveHeader\tline=") + std::to_string(li) + "\ttext=" + (cs->second ? combNext : combPrev));
                            combinedStarted = true;
                        }
                    } catch (...) {  }
                } catch (...) {  }
                if (combinedStarted) {
                } else {
                    try {
                    auto ol = core::application::importing::statement::internal::rawToOcrLine(l);
                        bool allowAboveHeader = false;
                        try { if (helpers::hasAmountNearValuta(ol, cols.valutaX, helpers::parserConfig.amountNearValutaBandPx)) allowAboveHeader = true; } catch (...) {  }
                        try { if (!allowAboveHeader && helpers::isLooseTransactionLine(ol, cols.valutaX)) allowAboveHeader = true; } catch (...) {  }
                        if (allowAboveHeader) {
                            inTransactions = true;
                            ++txStartLooseCount;
                            out.debugLines.push_back(std::string("tx.start.aboveHeaderDetected\tline=") + std::to_string(li) + "\ttext=" + txt);
                        } else {
                            out.debugLines.push_back(std::string("line.reason\taboveHeaderSkip\t") + txt + "\tline=" + std::to_string(li));
                            prevLine = txt;
                            continue;
                        }
                    } catch (...) {

                        out.debugLines.push_back(std::string("line.reason\taboveHeaderSkip\t") + txt + "\tline=" + std::to_string(li));
                        prevLine = txt;
                        continue;
                    }
                }
            }
        }

        if ((!cols.hasDebit() || !cols.hasCredit()) && core::application::importing::statement::internal::isLikelyTransactionHeaderLine(core::application::importing::statement::internal::rawToOcrLine(l), cols)) {
            if (!cols.hasDebit()) {
                if (auto dx = helpers::findPhraseCenterX(core::application::importing::statement::internal::rawToOcrLine(l), {"zu","ihren","lasten"})) cols.debitX = *dx;
            }
            if (!cols.hasCredit()) {
                if (auto cx = helpers::findPhraseCenterX(core::application::importing::statement::internal::rawToOcrLine(l), {"zu","ihren","gunsten"})) cols.creditX = *cx;
            }
            out.debugLines.push_back(std::string("header.debitcredit\t") + txt + "\tline=" + std::to_string(li));
            out.debugLines.push_back(std::string("cols.debitX\t") + std::to_string(cols.debitX) + "\tcols.creditX\t" + std::to_string(cols.creditX));
            try { headerBottomY = std::max(headerBottomY, l.maxY); } catch (...) {  }
            prevLine = txt;
            continue;
        }

        if (!rows.inSection && (helpers::isTransactionsSectionHeader(txt) || isValutaHeaderLine(txt) || helpers::isTransactionsSectionHeader(combined))) {
            rows.inSection = true;
            if (!cols.hasValuta()) {
                if (auto vx = helpers::findTokenCenterX(core::application::importing::statement::internal::rawToOcrLine(l), "valuta")) cols.valutaX = *vx;
            }
            out.debugLines.push_back(std::string("header.section\t") + combined + "\tline=" + std::to_string(li));
            out.debugLines.push_back(std::string("cols.valutaX\t") + std::to_string(cols.valutaX));
            try { headerBottomY = std::max(headerBottomY, l.maxY); } catch (...) {  }
            prevLine = txt;
            continue;
        }

        if (auto bd = core::application::importing::statement::internal::findBookingDateInHeader(txt)) {
            currentBookingDate = *bd;
            out.debugLines.push_back(std::string("header.bookingDate\t") + currentBookingDate + "\tline=" + std::to_string(li));
            flush();
            cur.bookingDateGroup = currentBookingDate;
            try { headerBottomY = std::max(headerBottomY, l.maxY); } catch (...) {  }
            prevLine = txt;
            continue;
        }

        bool headerNoiseSkipped = false;
        try {
            if (!inTransactions && headerFound && helpers::isHeaderNoiseLine(txt)) {
                bool anchoredToValuta = false;
                try { if (seedCols.valutaX >= 0) { core::application::importing::transaction::internal::OcrLine _ol = core::application::importing::statement::internal::rawToOcrLine(l); if (helpers::hasTokenNearX(_ol, seedCols.valutaX, helpers::parserConfig.tokenNearBandForMainRow) || helpers::hasAmountLikeTokenInLine(_ol, seedCols.valutaX)) anchoredToValuta = true; } } catch (...) {  }

                bool looksMain = false;
                try {
                    core::application::importing::transaction::internal::OcrLine _ol = core::application::importing::statement::internal::rawToOcrLine(l);
                    if (helpers::isLooseTransactionLine(_ol, seedCols.valutaX)) looksMain = true;
                    if (helpers::hasAmountLikeTokenInLine(_ol, seedCols.valutaX)) looksMain = true;
                    if (helpers::hasShortDateToken(_ol.text) && helpers::hasLeftDescriptiveText(_ol, seedCols.valutaX)) looksMain = true;
                } catch (...) {  }

                if (!anchoredToValuta && !looksMain && !isTransactionsSectionHeader(txt) && !isDebitCreditHeaderLine(txt) && !isLikelyTransactionMainRowText(txt)) {
                    out.debugLines.push_back(std::string("line.skip.headerNoise\t") + txt + std::string("\tline=") + std::to_string(li));
                    prevLine = txt;
                    headerNoiseSkipped = true;
                } else {
                    out.debugLines.push_back(std::string("header.noise_filter.suppressed\tline=") + std::to_string(li));
                }
            }
        } catch (...) {  }
        if (headerNoiseSkipped) continue;

        if (!inTransactions) {
            const bool canStart = (!currentBookingDate.empty()) || (rows.inSection && cols.hasValuta());
            bool didVerticalStart = false;
            try {
                if (canStart && rows.inSection && cols.hasValuta()) {
                if (auto vs = core::application::importing::statement::internal::tryVerticalStart(ocrFromRaw, li, { cols.valutaX, cols.debitX, cols.creditX })) {
                        inTransactions = true; didVerticalStart = true;
                        if (!cur.main.left.empty() || !cur.detailLines.empty()) flush();
                        cur.bookingDateGroup = currentBookingDate;
                        cur.main = vs->first;
                        if (vs->second > 0) {
                            out.debugLines.push_back(std::string("tx.main.vertical.helper\tformed\tline=") + std::to_string(li));
                            ++li;
                        } else {
                            out.debugLines.push_back(std::string("tx.main.vertical.helper\tformed\tline=") + std::to_string(li));
                        }
                    }
                }
            } catch (...) {  }

            if (canStart && (isLikelyTransactionMainRowText(txt) || isLikelyTransactionMainRowGeom(l, cols))) {
                 inTransactions = true;
                 out.debugLines.push_back(std::string("tx.start\tline=") + std::to_string(li) + "\ttext=" + txt);
            } else if (didVerticalStart) {
            } else {
                bool startedCombined = false;
                try {
                    std::string combPrev = prevLine.empty() ? txt : (prevLine + std::string(" ") + txt);
                    std::string combNext = txt;
                    if (li + 1 < lines.size()) combNext = txt + std::string(" ") + lines[li+1].text;
                    try {
                        if (auto cs = core::application::importing::statement::internal::tryCombinedStart(ocrFromRaw, li, { cols.valutaX, cols.debitX, cols.creditX })) {
                            inTransactions = true; startedCombined = true;
                            out.debugLines.push_back(std::string("tx.start.combined\tline=") + std::to_string(li) + "\ttext=" + (cs->second ? combNext : combPrev));
                        }
                    } catch (...) {  }
                } catch (...) {  }
                if (startedCombined) {
                } else {
                    bool relaxedStart = false;
                    try {
                        core::application::importing::transaction::internal::OcrLine ol = core::application::importing::statement::internal::rawToOcrLine(l);
                        bool hasDate = false;
                         try { if (helpers::hasShortDateToken(ol.text)) hasDate = true; } catch (...) {  }
                        bool hasAmount = false;
                         try { if (helpers::hasAmountNearValuta(ol, cols.valutaX, helpers::parserConfig.amountNearValutaBandPx)) hasAmount = true; } catch (...) {  }
                        try {
                             if (!hasAmount && li > 0) { auto prevOl = core::application::importing::statement::internal::rawToOcrLine(lines[li-1]); if (helpers::hasAmountNearValuta(prevOl, cols.valutaX, helpers::parserConfig.amountNearValutaBandPx)) hasAmount = true; }
                             if (!hasAmount && li + 1 < lines.size()) { auto nextOl = core::application::importing::statement::internal::rawToOcrLine(lines[li+1]); if (helpers::hasAmountNearValuta(nextOl, cols.valutaX, helpers::parserConfig.amountNearValutaBandPx)) hasAmount = true; }
                        } catch (...) {  }
                        bool hasLeftDesc = false;
                         try { if (helpers::hasLeftDescriptiveText(ol, cols.valutaX)) hasLeftDesc = true; } catch (...) {  }
                        if (hasDate && (hasAmount || hasLeftDesc)) relaxedStart = true;
                    } catch (...) {  }
                    if (relaxedStart) {
                        inTransactions = true;
                        out.debugLines.push_back(std::string("tx.start.relaxed\tline=") + std::to_string(li) + "\ttext=" + txt);
                    } else {
                    inTransactions = true;
                    out.debugLines.push_back(std::string("tx.start.forced\tline=") + std::to_string(li) + "\ttext=" + txt);
                    if (!cur.main.left.empty() || !cur.detailLines.empty()) flush();
                    cur.bookingDateGroup = currentBookingDate;
                    core::application::importing::internal::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
                    cur.main = core::application::importing::statement::internal::handleMainRow(core::application::importing::statement::internal::rawToOcrLine(l), cg, false, out.debugLines);
                    prevLine = txt;
                    continue;
                    }
                }
            }
        }

        const bool mainByRegex = isLikelyTransactionMainRowText(txt);
        core::application::importing::statement::internal::ColumnModel effectiveCols = cols.hasValuta() ? cols : seedCols;
        const bool mainByGeom = (!mainByRegex) && ((li > headerScanLines) || effectiveCols.hasValuta()) && core::application::importing::statement::internal::isLikelyTransactionMainRowGeom(l, effectiveCols);

        if (mainByRegex || mainByGeom) {
            if (!cur.main.left.empty() || !cur.detailLines.empty()) flush();
            cur.bookingDateGroup = currentBookingDate;
            core::application::importing::internal::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
            cur.main = core::application::importing::statement::internal::handleMainRow(core::application::importing::statement::internal::rawToOcrLine(l), cg, mainByGeom, out.debugLines);
            prevLine = txt;
            continue;
        }

        if (!cur.main.left.empty()) {
            core::application::importing::internal::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
            core::application::importing::statement::internal::appendDetailLine(cur, core::application::importing::statement::internal::rawToOcrLine(l), cg, &out.debugLines);
        }

        prevLine = txt;
    }

    flush();

    try {
        for (const auto& ol : orphanLines) {
            try {
                bool looksMain = false;
                try { if (core::application::importing::internal::hasAmountLikeTokenInLine(ol, cols.valutaX)) looksMain = true; } catch (...) {  }
                try { if (!looksMain && core::application::importing::internal::isLooseTransactionLine(ol, cols.valutaX)) looksMain = true; } catch (...) {  }
                try { if (!looksMain && core::application::importing::internal::hasShortDateToken(ol.text) && core::application::importing::internal::hasLeftDescriptiveText(ol, cols.valutaX)) looksMain = true; } catch (...) {  }
                if (looksMain) {
                    core::application::importing::transaction::internal::TransactionBlock nb;
                    nb.bookingDateGroup = currentBookingDate;
                    core::application::importing::internal::ColumnGuess cg{ cols.valutaX, cols.debitX, cols.creditX };
                    nb.main = core::application::importing::statement::internal::handleMainRow(ol, cg, false, out.debugLines);
                    blocks.push_back(std::move(nb));
                    out.debugLines.push_back(std::string("tx.start.rescued\ttext=") + ol.text);
                }
            } catch (...) {  }
        }
    } catch (...) {  }

    core::application::importing::statement::internal::attachOrphansToBlocks(blocks,
                          orphanLines,
                          helpers::parserConfig.orphanAttachMaxGapPx,
                          cols.valutaX,
                          &out.debugLines);

    out.debugLines.push_back(std::string("blocks\t") + std::to_string(blocks.size()));

    core::application::importing::statement::internal::appendPageSummary(lines, headerBottomY, blocks, txStartLooseCount, out);
    core::application::importing::statement::internal::appendTransactionsFromBlocks(blocks, cols, ocr, documentImageProcessor, pageCropImagePath, pageCropImageBytes, txIndex, out);

    out.debugLines.push_back(std::string("transactions\t") + std::to_string(out.transactions.size()));

    out.lastBookingDate = currentBookingDate;
    out.nextTransactionIndex = txIndex;
    return out;
}

}
