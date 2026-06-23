/**
 * @file core/src/application/import/internal/ParserHelpers.cpp
 * @brief Implements low-level OCR helper routines for statement parsing.
 */

#include "core/application/import/internal/ParserHelpers.h"
#include "../../../utils/StringUtils.h"

namespace core::application::importing::internal {
using core::application::importing::transaction::internal::OcrLine;
using core::application::importing::transaction::internal::TransactionMainRow;
ParserConfig parserConfig;
namespace {
static const std::regex
    g_amountRegex(R"(^\(?-?\d{1,3}(?:[\.,]\d{3})*[\.,]\d{1,2}-?$)");
static const std::regex g_amountFallbackRegex(R"(^\(?-?\d+[\.,]\d{2}\)?$)");
static constexpr std::string_view g_narrowNoBreakSpaceUtf8 = "\xE2\x80\xAF";
static constexpr std::string_view g_thinSpaceUtf8 = "\xE2\x80\x89";

bool tokenLooksLikeAmount(const std::string &token) noexcept {
  try {
    return std::regex_match(token, g_amountRegex) ||
           std::regex_match(token, g_amountFallbackRegex);
  } catch (...) {
    return false;
  }
}

void eraseAll(std::string &value, const std::string_view token) {
  size_t position = 0;
  while ((position = value.find(token, position)) != std::string::npos) {
    value.erase(position, token.size());
  }
}

} // namespace

std::vector<size_t> findAmountTokenIndices(const OcrLine &line,
                                           int valutaX, int bandPx) noexcept {
  std::vector<size_t> out;
  try {
    const auto toks = core::utils::splitWhitespace(line.text);
    for (size_t i = 0; i < toks.size(); ++i) {
      try {
        if (!tokenLooksLikeAmount(toks[i]))
          continue;
      } catch (...) {
        continue;
      }
      if (valutaX >= 0 && bandPx > 0) {
        if (i >= line.wordSpans.size())
          continue;
        const auto &sp = line.wordSpans[i];
        const int cx = (sp.first + sp.second) / 2;
        if (std::abs(cx - valutaX) > bandPx)
          continue;
      }
      out.push_back(i);
    }
  } catch (...) {}
  return out;
}

std::string normalizeAlnumLower(const std::string &s) noexcept {
  std::string o;
  o.reserve(s.size());
  for (unsigned char c : core::utils::lowerAscii(s)) {
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
      o.push_back(static_cast<char>(c));
  }
  return o;
}

std::optional<int> findTokenCenterX(const OcrLine &line,
                                    const std::string &tokenLower) noexcept {
  const auto toks = core::utils::splitWhitespace(line.text);
  if (toks.size() != line.wordSpans.size())
    return std::nullopt;

  const auto want = normalizeAlnumLower(tokenLower);
  for (size_t i = 0; i < toks.size(); ++i) {
    if (normalizeAlnumLower(toks[i]) != want)
      continue;
    const auto &sp = line.wordSpans[i];
    return (sp.first + sp.second) / 2;
  }

  for (size_t i = 0; i < toks.size(); ++i) {
    const auto n = normalizeAlnumLower(toks[i]);
    if (n.find(want) == std::string::npos)
      continue;
    const auto &sp = line.wordSpans[i];
    return (sp.first + sp.second) / 2;
  }

  return std::nullopt;
}

std::optional<int>
findPhraseCenterX(const OcrLine &line,
                  std::initializer_list<const char *> phraseLower) noexcept {
  const auto toks = core::utils::splitWhitespace(line.text);
  if (toks.size() != line.wordSpans.size())
    return std::nullopt;

  std::string want;
  for (auto w : phraseLower)
    want += normalizeAlnumLower(std::string(w));
  if (want.empty() || toks.empty())
    return std::nullopt;

  for (size_t i = 0; i < toks.size(); ++i) {
    std::string acc;
    int left = line.wordSpans[i].first;
    int right = line.wordSpans[i].second;

    for (size_t j = i;
         j < toks.size() &&
         (j - i) < static_cast<size_t>(parserConfig.maxPhraseTokens);
         ++j) {
      acc += normalizeAlnumLower(toks[j]);
      right = line.wordSpans[j].second;
      if (acc.find(want) != std::string::npos) {
        return (left + right) / 2;
      }
    }
  }

  return std::nullopt;
}

bool hasTokenNearX(const OcrLine &line, int x,
                   int bandPx) noexcept {
  if (x < 0)
    return false;
  const auto toks = core::utils::splitWhitespace(line.text);
  if (toks.size() != line.wordSpans.size() || toks.empty())
    return false;

  for (size_t i = 0; i < toks.size(); ++i) {
    const auto &sp = line.wordSpans[i];
    const int cx = (sp.first + sp.second) / 2;
    if (std::abs(cx - x) <= bandPx)
      return true;
  }
  return false;
}

bool hasAmountLikeTokenInLine(const OcrLine &line,
                              int valutaX) noexcept {
  try {
    if (valutaX >= 0) {
      return !findAmountTokenIndices(line, valutaX,
                                     parserConfig.amountNearValutaBandPx)
                  .empty();
    }
    return !findAmountTokenIndices(line, -1, 0).empty();
  } catch (...) {}
  return false;
}

bool hasLeftDescriptiveText(const OcrLine &line,
                            int valutaX) noexcept {
  try {
    const auto toks = core::utils::splitWhitespace(line.text);
    if (toks.empty())
      return false;
    for (size_t i = 0; i < toks.size() && i < line.wordSpans.size(); ++i) {
      const auto &sp = line.wordSpans[i];
      const int cx = (sp.first + sp.second) / 2;
      if (cx < valutaX - parserConfig.leftDescriptiveOffsetPx) {
        for (unsigned char c : toks[i])
          if (std::isalpha(c))
            return true;
      }
    }
    for (unsigned char c : toks[0])
      if (std::isalpha(c))
        return true;
  } catch (...) {}
  return false;
}

bool hasAmountNearValuta(const OcrLine &line, int valutaX,
                         int bandPx) noexcept {
  if (valutaX < 0)
    return false;
  try {
    auto idxs = findAmountTokenIndices(line, valutaX, bandPx);
    return !idxs.empty();
  } catch (...) {}
  return false;
}

bool isLooseTransactionLine(const OcrLine &line,
                            int valutaX) noexcept {
  if (valutaX < 0)
    return false;
  try {
    if (!hasShortDateToken(line.text))
      return false;
    std::smatch md;
    if (!std::regex_search(line.text, md, std::regex(R"((\d{2}\.\s*\d{2}))")))
      return false;
    const size_t datePos = static_cast<size_t>(md.position(0));
    const auto toks = core::utils::splitWhitespace(line.text);
    bool foundAmountAfterDate = false;
    auto amtIdxs = findAmountTokenIndices(line, -1, 0);
    for (auto i : amtIdxs) {
      if (i >= toks.size())
        continue;
      const auto pos = line.text.find(toks[i]);
      if (pos != std::string::npos && pos > datePos) {
        foundAmountAfterDate = true;
        break;
      }
    }
    if (!foundAmountAfterDate)
      return false;
    const auto left = line.text.substr(0, datePos);
    for (unsigned char c : left) {
      if (std::isalpha(c))
        return true;
    }
  } catch (...) {}
  return false;
}

std::optional<double>
findAndParseAmountInLine(const OcrLine &line, int valutaX,
                         std::vector<std::string> *debugOut) noexcept {
  try {
    int band = (valutaX >= 0) ? parserConfig.amountNearValutaBandPx : 0;
    auto idxs = findAmountTokenIndices(line, valutaX, band);
    if (idxs.empty()) {
      idxs = findAmountTokenIndices(line, -1, 0);
    }
    const auto toks = core::utils::splitWhitespace(line.text);
    auto tryParse = [&](const std::string &s) -> std::optional<double> {
      try {
        if (s.empty())
          return std::nullopt;
        if (debugOut)
          debugOut->push_back(std::string("candidate.try\t") + s);

        try {
          if (hasShortDateToken(s)) {
            if (debugOut)
              debugOut->push_back(std::string("candidate.skip.shortDate\t") +
                                  s);
            return std::nullopt;
          }
        } catch (...) {}

        try {
          std::string norm = s;
          norm = core::utils::trim(norm);
          bool negative = false;
          if (!norm.empty() && norm.front() == '(' && norm.back() == ')') {
            negative = true;
            norm = norm.substr(1, norm.size() - 2);
          }
          if (!norm.empty() && norm.front() == '-') {
            negative = true;
            norm = norm.substr(1);
          }
          if (!norm.empty() && norm.back() == '-') {
            negative = true;
            norm.pop_back();
          }

          eraseAll(norm, g_narrowNoBreakSpaceUtf8);
          eraseAll(norm, g_thinSpaceUtf8);

          std::string tmp;
          tmp.reserve(norm.size());
          for (unsigned char c : norm) {
            if (c == '.' || c == ' ')
              continue;
            tmp.push_back(static_cast<char>(c));
          }
          std::replace(tmp.begin(), tmp.end(), ',', '.');

          try {
            if (!tmp.empty()) {
              double v = std::stod(tmp);
              if (negative)
                v = -v;
              if (debugOut)
                debugOut->push_back(std::string("candidate.ok.norm\t") + tmp +
                                    std::string(" -> ") + std::to_string(v));
              return v;
            }
          } catch (...) {}
        } catch (...) {}

        if (auto v =
                ::core::application::importing::transaction::parseAmountString(s)) {
          if (debugOut)
            debugOut->push_back(std::string("candidate.ok\t") + s +
                                std::string(" -> ") + std::to_string(*v));
          return v;
        }

        std::string nosp = s;
        nosp.erase(std::remove(nosp.begin(), nosp.end(), ' '), nosp.end());
        if (!nosp.empty()) {
          if (debugOut)
            debugOut->push_back(std::string("candidate.try.nosp\t") + nosp);
          if (auto v2 =
                  ::core::application::importing::transaction::parseAmountString(
                      nosp)) {
            if (debugOut)
              debugOut->push_back(std::string("candidate.ok.nosp\t") + nosp +
                                  std::string(" -> ") + std::to_string(*v2));
            return v2;
          }
        }
      } catch (...) {}
      return std::nullopt;
    };

    if (debugOut)
      debugOut->push_back(
          std::string("helper.findAndParseAmountInLine\tline=") + line.text +
          std::string(" idxs=") + std::to_string(idxs.size()));

    for (auto i : idxs) {
      if (i >= toks.size())
        continue;
      if (auto r = tryParse(toks[i]))
        return r;
      if (i > 0) {
        if (auto r = tryParse(toks[i - 1] + toks[i]))
          return r;
        if (auto r2 = tryParse(toks[i - 1] + " " + toks[i]))
          return r2;
      }
      for (int len = 1; len <= 2; ++len) {
        std::string joined;
        for (int k = 0; k <= len; ++k) {
          int idx = static_cast<int>(i) + k;
          if (idx >= static_cast<int>(toks.size()))
            break;
          if (!joined.empty())
            joined.push_back(' ');
          joined += toks[idx];
        }
        if (!joined.empty()) {
          if (auto r = tryParse(joined))
            return r;
        }
      }
      if (i + 2 < toks.size()) {
        std::string three = toks[i] + toks[i + 1] + toks[i + 2];
        if (auto r = tryParse(three))
          return r;
      }
    }
    if (debugOut)
      debugOut->push_back(std::string("candidate.try.whole\t") + line.text);
    if (auto v = ::core::application::importing::transaction::parseAmountString(
            line.text)) {
      if (debugOut)
        debugOut->push_back(std::string("candidate.ok.whole\t") + line.text +
                            std::string(" -> ") + std::to_string(*v));
      return v;
    }
  } catch (...) {}
  return std::nullopt;
}

std::vector<OcrLine> buildOcrLinesFromWords(
    const std::vector<core::ports::text_recognition::Word> &words) {
  struct W {
    const core::ports::text_recognition::Word *w;
    int cy;
    int top;
    int bottom;
    int left;
  };
  std::vector<W> ws;
  ws.reserve(words.size());
  for (const auto &w : words) {
    bool anyNonSpace = false;
    for (unsigned char c : w.text) {
      if (!std::isspace(c)) {
        anyNonSpace = true;
        break;
      }
    }
    if (!anyNonSpace)
      continue;
    const int cy = w.bbox.y + w.bbox.height / 2;
    ws.push_back(W{&w, cy, w.bbox.y, w.bbox.y + w.bbox.height, w.bbox.x});
  }
  std::sort(ws.begin(), ws.end(), [](const W &a, const W &b) {
    if (a.top != b.top)
      return a.top < b.top;
    if (a.cy != b.cy)
      return a.cy < b.cy;
    return a.left < b.left;
  });
  int avgH = 0;
  for (const auto &ww : ws)
    avgH += (ww.bottom - ww.top);
  avgH = ws.empty() ? 10 : std::max(1, avgH / static_cast<int>(ws.size()));
  const int minOverlap = std::max(2, avgH / 3);
  struct LineAcc {
    int cy = 0;
    int minX = std::numeric_limits<int>::max();
    int maxX = std::numeric_limits<int>::min();
    int minY = std::numeric_limits<int>::max();
    int maxY = std::numeric_limits<int>::min();
    std::vector<const core::ports::text_recognition::Word *> words;
  };
  std::vector<LineAcc> acc;
  acc.reserve(std::max<size_t>(8, words.size() / 8));
  for (const auto &ww : ws) {
    const auto &w = *ww.w;
    const int wMinX = w.bbox.x;
    const int wMaxX = w.bbox.x + w.bbox.width;
    const int wMinY = w.bbox.y;
    const int wMaxY = w.bbox.y + w.bbox.height;
    int bestIdx = -1;
    int bestOverlap = -1;
    for (int i = static_cast<int>(acc.size()) - 1; i >= 0; --i) {
      const auto &l = acc[static_cast<size_t>(i)];
      const int top = std::max(l.minY, wMinY);
      const int bot = std::min(l.maxY, wMaxY);
      const int ov = bot - top;
      if (ov >= minOverlap && ov > bestOverlap) {
        bestOverlap = ov;
        bestIdx = i;
      }
      if (wMinY - l.maxY > avgH)
        break;
    }
    if (bestIdx < 0) {
      LineAcc nl;
      nl.cy = ww.cy;
      nl.minX = wMinX;
      nl.maxX = wMaxX;
      nl.minY = wMinY;
      nl.maxY = wMaxY;
      nl.words.push_back(&w);
      acc.push_back(std::move(nl));
      continue;
    }
    auto &l = acc[static_cast<size_t>(bestIdx)];
    l.cy = (l.cy + ww.cy) / 2;
    l.minX = std::min(l.minX, wMinX);
    l.maxX = std::max(l.maxX, wMaxX);
    l.minY = std::min(l.minY, wMinY);
    l.maxY = std::max(l.maxY, wMaxY);
    l.words.push_back(&w);
  }
  std::vector<OcrLine> out;
  out.reserve(acc.size());
  for (auto &la : acc) {
    std::sort(la.words.begin(), la.words.end(),
              [](const core::ports::text_recognition::Word *a,
                 const core::ports::text_recognition::Word *b) {
                if (a->bbox.x != b->bbox.x)
                  return a->bbox.x < b->bbox.x;
                return a->bbox.y < b->bbox.y;
              });
    OcrLine ol;
    ol.minY = la.minY;
    ol.maxY = la.maxY;
    ol.wordSpans.reserve(la.words.size());
    if (!la.words.empty()) {
      ol.minX = la.words.front()->bbox.x;
      ol.maxX = la.words.front()->bbox.x + la.words.front()->bbox.width;
    } else {
      ol.minX = 0;
      ol.maxX = 0;
    }
    std::string txt;
    for (const auto *w : la.words) {
      const int left = w->bbox.x;
      const int right = w->bbox.x + w->bbox.width;
      ol.wordSpans.emplace_back(left, right);
      ol.minX = std::min(ol.minX, left);
      ol.maxX = std::max(ol.maxX, right);
      if (!txt.empty())
        txt.push_back(' ');
      txt += w->text;
    }
    ol.text = core::utils::trim(std::move(txt));
    out.push_back(std::move(ol));
  }
  return out;
}

ColumnGuess
inferColumnModelFromLines(const std::vector<OcrLine> &lines,
                          size_t scanN) noexcept {
  ColumnGuess out;
  try {
    const size_t n = std::min(lines.size(), scanN);
    for (size_t i = 0; i < n; ++i) {
      const auto &l = lines[i];
      try {
        if (out.valutaX < 0)
          if (auto vx = findTokenCenterX(l, "valuta"))
            out.valutaX = *vx;
      } catch (...) {}
      try {
        if (out.debitX < 0)
          if (auto dx = findPhraseCenterX(l, {"zu", "ihren", "lasten"}))
            out.debitX = *dx;
      } catch (...) {}
      try {
        if (out.creditX < 0)
          if (auto cx = findPhraseCenterX(l, {"zu", "ihren", "gunsten"}))
            out.creditX = *cx;
      } catch (...) {}
      if (out.valutaX >= 0 && out.debitX >= 0 && out.creditX >= 0)
        break;
    }
  } catch (...) {}
  return out;
}

std::vector<RawLineLite>
groupMergeLinesRaw(const std::vector<RawLineLite> &lines, int maxGapPx) {
  if (lines.size() <= 1)
    return lines;
  std::vector<RawLineLite> merged;
  merged.reserve(lines.size());
  RawLineLite cur = lines.front();
  for (size_t i = 1; i < lines.size(); ++i) {
    const auto &next = lines[i];
    if (next.minY - cur.maxY <= maxGapPx) {
      cur.maxY = next.maxY;
      cur.wordSpans.insert(cur.wordSpans.end(), next.wordSpans.begin(),
                           next.wordSpans.end());
      cur.text += " " + next.text;
    } else {
      merged.push_back(std::move(cur));
      cur = next;
    }
  }
  merged.push_back(std::move(cur));
  return merged;
}

core::application::importing::transaction::internal::OcrLine
toOcrLineFromRawWords(const RawLineLite &src, size_t i0, size_t i1) noexcept {
  core::application::importing::transaction::internal::OcrLine out;
  if (i0 >= i1 || i0 >= src.wordSpans.size())
    return out;
  i1 = std::min(i1, src.wordSpans.size());
  out.minY = src.minY;
  out.maxY = src.maxY;
  out.minX = src.wordSpans[i0].first;
  out.maxX = src.wordSpans[i0].second;
  out.wordSpans.reserve(i1 - i0);
  for (size_t i = i0; i < i1; ++i) {
    const auto &sp = src.wordSpans[i];
    out.minX = std::min(out.minX, sp.first);
    out.maxX = std::max(out.maxX, sp.second);
    out.wordSpans.push_back(sp);
  }
  const auto toks = core::utils::splitWhitespace(src.text);
  std::string txt;
  for (size_t i = i0; i < i1 && i < toks.size(); ++i) {
    if (!txt.empty())
      txt.push_back(' ');
    txt += toks[i];
  }
  out.text = core::utils::trim(std::move(txt));
  return out;
}

core::application::importing::transaction::internal::TransactionMainRow
splitMainRowFromRaw(const RawLineLite &src, int valutaX, int debitX,
                    int creditX) noexcept {
  core::application::importing::transaction::internal::TransactionMainRow row;
  const auto toks = core::utils::splitWhitespace(src.text);
  if (toks.size() != src.wordSpans.size() || toks.empty()) {
    row.left.line.text = src.text;
    row.left.line.minX = src.minX;
    row.left.line.maxX = src.maxX;
    row.left.line.minY = src.minY;
    row.left.line.maxY = src.maxY;
    row.left.line.wordSpans = src.wordSpans;
    return row;
  }
  auto cxAt = [&](size_t i) -> int {
    const auto &sp = src.wordSpans[i];
    return (sp.first + sp.second) / 2;
  };
  auto idxNearX = [&](int x, int skipIdx) -> int {
    int bestIdx = -1;
    int bestDist = std::numeric_limits<int>::max();
    for (size_t i = 0; i < toks.size(); ++i) {
      if ((int)i == skipIdx)
        continue;
      int d = std::abs(cxAt(i) - x);
      if (d < bestDist) {
        bestDist = d;
        bestIdx = (int)i;
      }
    }
    return bestIdx;
  };
  auto idxNearXPreferNumeric = [&](int x, int skipIdx) -> int {
    int bestIdx = -1;
    int bestDist = std::numeric_limits<int>::max();
    int bestPriority = -1;
    for (size_t i = 0; i < toks.size(); ++i) {
      if ((int)i == skipIdx)
        continue;
      int d = std::abs(cxAt(i) - x);
      int priority = 0;
      try {
        if (tokenLooksLikeAmount(toks[i]))
          priority = 2;
      } catch (...) {}
      if (priority > bestPriority ||
          (priority == bestPriority && d < bestDist)) {
        bestPriority = priority;
        bestDist = d;
        bestIdx = (int)i;
      }
    }
    return bestIdx;
  };

  int valutaIdx = (valutaX >= 0) ? idxNearX(valutaX, -1) : -1;
  int debitIdx = (debitX >= 0) ? idxNearXPreferNumeric(debitX, -1) : -1;
  int creditIdx = (creditX >= 0) ? idxNearXPreferNumeric(creditX, -1) : -1;
  try {
    if (valutaIdx >= 0 && debitIdx < 0 && creditIdx < 0) {
      int found = -1;
      for (size_t i = 0; i < toks.size(); ++i) {
        if (cxAt(i) <= valutaX - 120)
          continue;
        try {
          if (tokenLooksLikeAmount(toks[i])) {
            found = (int)i;
            break;
          }
        } catch (...) {}
      }
      if (found >= 0) {
        if (creditX >= 0)
          creditIdx = found;
        else if (debitX >= 0)
          debitIdx = found;
        else
          creditIdx = found;
      }
    }
  } catch (...) {}
  if (valutaIdx >= 0) {
    if (debitIdx == valutaIdx && debitX >= 0)
      debitIdx = idxNearX(debitX, valutaIdx);
    if (creditIdx == valutaIdx && creditX >= 0)
      creditIdx = idxNearX(creditX, valutaIdx);
  }
  if (debitIdx >= 0 && creditIdx == debitIdx && creditX >= 0)
    creditIdx = idxNearX(creditX, debitIdx);

  int firstRight = std::numeric_limits<int>::max();
  if (valutaIdx >= 0)
    firstRight = std::min(firstRight, valutaIdx);
  if (debitIdx >= 0)
    firstRight = std::min(firstRight, debitIdx);
  if (creditIdx >= 0)
    firstRight = std::min(firstRight, creditIdx);
  if (firstRight == std::numeric_limits<int>::max())
    firstRight = (int)toks.size();

  row.left.line = toOcrLineFromRawWords(
      src, 0, static_cast<size_t>(std::max(0, firstRight)));
  if (valutaIdx >= 0) {
    size_t v0 = static_cast<size_t>(valutaIdx);
    size_t v1 = v0 + 1;
    if (v1 < toks.size() &&
        std::abs(cxAt(v1) - valutaX) <= parserConfig.valutaNeighborExpandPx)
      v1 = v1 + 1;
    row.valuta.line = toOcrLineFromRawWords(src, v0, v1);
  }
  if (debitIdx >= 0)
    row.debit.line = toOcrLineFromRawWords(src, static_cast<size_t>(debitIdx),
                                           static_cast<size_t>(debitIdx + 1));
  if (creditIdx >= 0)
    row.credit.line = toOcrLineFromRawWords(src, static_cast<size_t>(creditIdx),
                                            static_cast<size_t>(creditIdx + 1));
  return row;
}

core::application::importing::transaction::internal::TransactionMainRow
splitMainRowFromOcrLine(
    const core::application::importing::transaction::internal::OcrLine &src,
    int valutaX, int debitX, int creditX) noexcept {
  RawLineLite rl;
  rl.minX = src.minX;
  rl.maxX = src.maxX;
  rl.minY = src.minY;
  rl.maxY = src.maxY;
  rl.wordSpans = src.wordSpans;
  rl.text = src.text;
  return splitMainRowFromRaw(rl, valutaX, debitX, creditX);
}

} // namespace core::application::importing::internal
