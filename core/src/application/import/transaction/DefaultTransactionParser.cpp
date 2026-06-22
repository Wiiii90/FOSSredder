/**
 * @file core/src/application/import/transaction/DefaultTransactionParser.cpp
 * @brief Implements transaction parsing and amount resolution logic.
 */

#include "core/application/import/transaction/DefaultTransactionParser.h"
#include "../../../utils/Util.h"
#include "core/application/import/internal/ParserHelpers.h"

#include <algorithm>
#include <regex>
#include <string>

using core::utils::collapseWhitespace;
using core::utils::splitWhitespace;
using core::utils::trim;

namespace core::application::importing::transaction {
using internal::OcrLine;
using internal::TransactionBlock;
namespace helpers = core::application::importing::internal;
namespace amount_parser = core::application::importing::transaction;

namespace {

bool tightContinuation(const OcrLine& prev, const OcrLine& next) {
    const bool closeY = (next.minY - prev.maxY) <= 18;
    const bool leftAligned = std::abs(next.minX - prev.minX) <= 14;
    return closeY && leftAligned;
}

bool hasSingleTokenBeforeColon(const std::string& s, size_t colonPos) {
    if (colonPos == std::string::npos || colonPos == 0) return false;
    const auto left = trim(s.substr(0, colonPos));
    if (left.empty()) return false;
    return splitWhitespace(left).size() == 1;
}

bool looksLikeLabelStart(const std::string& line) {
    const auto pos = line.find(':');
    if (pos == std::string::npos || pos == 0 || pos > 40) return false;
    return hasSingleTokenBeforeColon(line, pos);
}

std::optional<std::string> parseValutaToken(const std::string& line) {
    static const std::regex reJoined(R"((\d{2}\.\d{2}))");
    static const std::regex reSplit(R"((\d{2})\.)");
    std::smatch m;

    if (std::regex_search(line, m, reJoined)) return m.str(1);

    {
        std::smatch md;
        if (std::regex_search(line, md, reSplit)) {
            const auto day = md.str(1);
            const auto rest = md.suffix().str();
            static const std::regex reMonth(R"(\b(\d{2})\b)");
            std::smatch mm;
            if (std::regex_search(rest, mm, reMonth)) {
                return day + "." + mm.str(1);
            }
        }
    }

    return std::nullopt;
}

}

int textRightEdge(const OcrLine& l) {
    if (!l.wordSpans.empty()) return core::utils::rightEdgeFromWordSpans(l.wordSpans);
    return l.maxX;
}

DefaultTransactionParser DefaultTransactionParser::parseTransaction(const TransactionBlock& block, std::vector<std::string>* debugOut) {
    DefaultTransactionParser tx;
    tx.bookingDate = block.bookingDateGroup;

    if (!block.main.valuta.empty()) {
        if (auto v = parseValutaToken(block.main.valuta.line.text)) tx.valuta = *v;
    }

    bool sourceDebit = false;
    bool sourceCredit = false;

    auto tokenIndicatesNegative = [&](const std::string &txt)->bool{
        std::string s = txt;
        try { s = core::utils::trim(s); } catch (...) {  }
        if (s.empty()) return false;
        if (s.front() == '(' && s.back() == ')') return true;
        if (s.front() == '-') return true;
        if (s.back() == '-') return true;
        return false;
    };

    auto centerXOf = [&](const OcrLine& l)->int{
        if (!l.wordSpans.empty()) return (l.wordSpans.front().first + l.wordSpans.back().second) / 2;
        return (l.minX + l.maxX) / 2;
    };

    std::optional<double> creditVal;
    std::optional<double> debitVal;
    bool creditExplicitNeg = false;
    bool debitExplicitNeg = false;

    if (!block.main.credit.empty()) {
        creditExplicitNeg = tokenIndicatesNegative(block.main.credit.line.text);
        if (auto v = helpers::findAndParseAmountInLine(block.main.credit.line, -1, debugOut)) {
            creditVal = v;
            if (debugOut) debugOut->push_back(std::string("initial.credit.helperUsed\t") + block.main.credit.line.text + std::string(" -> ") + std::to_string(*creditVal));
        } else {
            creditVal = amount_parser::parseAmountString(block.main.credit.line.text);
            if (debugOut) debugOut->push_back(std::string("initial.credit.parse\t") + block.main.credit.line.text + std::string(" -> ") + (creditVal ? std::to_string(*creditVal) : std::string("(none)")));
        }
    }
    if (!block.main.debit.empty()) {
        debitExplicitNeg = tokenIndicatesNegative(block.main.debit.line.text);
        if (auto v = helpers::findAndParseAmountInLine(block.main.debit.line, -1, debugOut)) {
            debitVal = v;
            if (debugOut) debugOut->push_back(std::string("initial.debit.helperUsed\t") + block.main.debit.line.text + std::string(" -> ") + std::to_string(*debitVal));
        } else {
            debitVal = amount_parser::parseAmountString(block.main.debit.line.text);
            if (debugOut) debugOut->push_back(std::string("initial.debit.parse\t") + block.main.debit.line.text + std::string(" -> ") + (debitVal ? std::to_string(*debitVal) : std::string("(none)")));
        }
    }

    try {
        const double suspiciousThreshold = 1.0;
        bool needFallback = (!creditVal && !debitVal);
        if (!needFallback) {
            if (creditVal && std::abs(*creditVal) < suspiciousThreshold) needFallback = true;
            if (debitVal && std::abs(*debitVal) < suspiciousThreshold) needFallback = true;
        }
        if (needFallback) {
            if (debugOut) debugOut->push_back(std::string("recovery.scan.start\tcreditVal=") + (creditVal?"1":"0") + std::string(" debitVal=") + (debitVal?"1":"0"));
            if (!block.main.credit.empty()) {
                if (auto v = helpers::findAndParseAmountInLine(block.main.credit.line, -1, debugOut)) { creditVal = v; if (debugOut) debugOut->push_back(std::string("recovery.credit.helper\t") + block.main.credit.line.text + std::string(" -> ") + std::to_string(*v)); }
            }
            if (!creditVal && !block.main.debit.empty()) {
                if (auto v = helpers::findAndParseAmountInLine(block.main.debit.line, -1, debugOut)) { debitVal = v; if (debugOut) debugOut->push_back(std::string("recovery.debit.helper\t") + block.main.debit.line.text + std::string(" -> ") + std::to_string(*v)); }
            }
            if (!creditVal && !debitVal) {
                if (!block.main.left.empty()) {
                    if (auto v = helpers::findAndParseAmountInLine(block.main.left.line, -1, debugOut)) {
                        creditVal = v;
                        if (debugOut) debugOut->push_back(std::string("recovery.left.helper\t") + block.main.left.line.text + std::string(" -> ") + std::to_string(*v));
                    }
                }
                for (const auto& dl : block.detailLines) {
                    if (creditVal || debitVal) break;
                    if (auto v = helpers::findAndParseAmountInLine(dl, -1, debugOut)) { creditVal = v; if (debugOut) debugOut->push_back(std::string("recovery.detail.helper\t") + dl.text + std::string(" -> ") + std::to_string(*v)); break; }
                }
            }
        }
    } catch (...) {  }

    if (creditVal && debitVal) {
        if (creditExplicitNeg && !debitExplicitNeg) {
            tx.amount = *creditVal;
            if (debugOut) debugOut->push_back(std::string("resolve.both\tchoose.creditNeg->") + std::to_string(tx.amount));
        } else if (debitExplicitNeg && !creditExplicitNeg) {
            tx.amount = *debitVal;
            if (debugOut) debugOut->push_back(std::string("resolve.both\tchoose.debitNeg->") + std::to_string(tx.amount));
        } else {
            if (std::abs(*debitVal) >= std::abs(*creditVal)) tx.amount = *debitVal; else tx.amount = *creditVal;
            if (debugOut) debugOut->push_back(std::string("resolve.both\tchoose.larger->") + std::to_string(tx.amount));
        }
    } else if (creditVal) {
        tx.amount = *creditVal;
        if (debugOut) debugOut->push_back(std::string("resolve.credit->") + std::to_string(tx.amount));
    } else if (debitVal) {
        bool treatAsCredit = false;
        if (!block.main.credit.empty()) {
            try {
                int cxDebit = centerXOf(block.main.debit.line);
                int cxCredit = centerXOf(block.main.credit.line);
                if (cxDebit > cxCredit) treatAsCredit = true;
            } catch (...) {  }
        }
        if (tokenIndicatesNegative(block.main.debit.line.text)) {
            tx.amount = *debitVal;
            if (debugOut) debugOut->push_back(std::string("resolve.debitExpNeg->") + std::to_string(tx.amount));
        } else if (treatAsCredit) {
            tx.amount = *debitVal;
            if (debugOut) debugOut->push_back(std::string("resolve.debit.treatAsCredit->") + std::to_string(tx.amount));
        } else {
            tx.amount = -std::abs(*debitVal);
            if (debugOut) debugOut->push_back(std::string("resolve.debit.defaultNegative->") + std::to_string(tx.amount));
        }
    }

    if (tx.amount == 0.0 && !block.main.debit.empty() && !block.main.credit.empty()) {
        auto tryCombineParse = [&](const std::string& a, const std::string& b)->std::optional<double> {
            try {
                if (a.empty() || b.empty()) return std::nullopt;
                if (auto p = amount_parser::parseAmountString(a + b)) return p;
                if (auto p2 = amount_parser::parseAmountString(a + " " + b)) return p2;
            } catch (...) {  }
            return std::nullopt;
        };
        if (auto p = tryCombineParse(block.main.debit.line.text, block.main.credit.line.text)) {
            bool creditRightOfDebit = false;
            try { creditRightOfDebit = centerXOf(block.main.credit.line) > centerXOf(block.main.debit.line); } catch (...) {  }
            if (creditRightOfDebit) tx.amount = *p; else tx.amount = -std::abs(*p);
            if (debugOut) debugOut->push_back(std::string("combine.debit+credit->") + std::to_string(tx.amount));
        } else if (auto p = tryCombineParse(block.main.credit.line.text, block.main.debit.line.text)) {
            tx.amount = *p;
            if (debugOut) debugOut->push_back(std::string("combine.credit+debit->") + std::to_string(tx.amount));
        }
    }

    try {
        auto hasNegMarker = [&](const std::string &t)->bool{
            if (t.empty()) return false;
            for (char c : t) if (c == '-' || c == '(' || c == ')') return true;
            return false;
        };
        if (tx.amount > 0.0 && (hasNegMarker(block.main.debit.line.text) || hasNegMarker(block.main.credit.line.text))) {
            tx.amount = -std::abs(tx.amount);
            if (debugOut) debugOut->push_back(std::string("enforce.negMarker->") + std::to_string(tx.amount));
        }
    } catch (...) {  }

    try {
        if (sourceDebit && sourceCredit) {
        } else if (sourceDebit) {
            if (tx.amount > 0.0) tx.amount = -std::abs(tx.amount);
        } else if (sourceCredit) {
            if (tx.amount < 0.0) tx.amount = std::abs(tx.amount);
        }
    } catch (...) {  }

    std::vector<OcrLine> lines;
    if (!block.main.left.empty()) lines.push_back(block.main.left.line);
    for (const auto& l : block.detailLines) {
        if (!trim(l.text).empty()) lines.push_back(l);
    }
    if (lines.empty()) return tx;

    int rightEdge = 0;
    for (const auto& l : lines) rightEdge = std::max(rightEdge, textRightEdge(l));

    std::vector<std::string> nameLines;
    nameLines.push_back(collapseWhitespace(lines.front().text));

    for (size_t i = 1; i < lines.size(); ++i) {
        const auto& prev = lines[i - 1];
        const auto& cur = lines[i];

        const int prevRight = textRightEdge(prev);
        const int gap = std::max(0, rightEdge - prevRight);
        const int w = std::max(1, rightEdge - lines.front().minX);

        const int allowAbs = std::max(24, w / 40);
        const int allowRel = w / 25;
        const int allow = std::min(std::max(1, allowAbs), std::max(1, allowRel));

        if (gap > allow) break;
        if (!tightContinuation(prev, cur)) break;

        nameLines.push_back(collapseWhitespace(cur.text));
    }

    std::string nm;
    for (const auto& l : nameLines) nm += l;
    tx.name = collapseWhitespace(std::move(nm));
    if (tx.name.size() > 120) tx.name.resize(120);

    std::string meta;
    bool lastWasLabel = false;
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string t = collapseWhitespace(lines[i].text);
        t = trim(std::move(t));
        if (t.empty()) continue;
        const bool isLabel = looksLikeLabelStart(t);

        if (i == nameLines.size()) {
            meta.push_back('\n');
        } else {
            if (!meta.empty()) {
                if (isLabel || lastWasLabel) meta.push_back('\n');
            }
        }

        meta += t;
        lastWasLabel = isLabel;
    }
    tx.metadata = trim(std::move(meta));

    return tx;
}

}
