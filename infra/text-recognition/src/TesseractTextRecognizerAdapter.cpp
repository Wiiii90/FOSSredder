/**
 * @file infra/text-recognition/src/TesseractTextRecognizerAdapter.cpp
 * @brief Implements the Tesseract adapter used by the import workflow.
 */

#include "text-recognition/pch.h"
#include "core/ports/infra/text-recognition/ITextRecognizer.h"
#include "core/ports/infra/text-recognition/TextRecognitionRequest.h"
#include "core/ports/infra/text-recognition/TextRecognitionResult.h"
#include "core/ports/infra/text-recognition/TextRecognitionTypes.h"
#include "text-recognition/TesseractCore.h"
#include "core/ports/diagnostics/IDiagnostics.h"
#include <sstream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <cctype>

class TesseractTextRecognizerAdapter : public core::ports::text_recognition::ITextRecognizer {
public:
    explicit TesseractTextRecognizerAdapter(std::shared_ptr<core::ports::diagnostics::IDiagnostics> dbg = nullptr) : debugger(std::move(dbg)) {}

    core::ports::text_recognition::ExtractResult extract(const core::ports::text_recognition::ExtractRequest& req) override {
        core::ports::text_recognition::ExtractResult out;
        if (req.cancelFlag && req.cancelFlag->load()) return out;

        std::vector<uint8_t> bytes;
        if (!req.imageBytes.empty()) {
            bytes = req.imageBytes;
        } else if (!req.imagePath.empty()) {
            std::ifstream ifs(req.imagePath, std::ios::binary);
            if (ifs) bytes.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        }

        if (req.cancelFlag && req.cancelFlag->load()) return out;

        auto [textDto, words] = TesseractCore::extractFromBytes(bytes, req.tessdataPath.string(), req.recognition, debugger);
        out.text = textDto.text;
        std::ostringstream oss;
        for (const auto &w : words) {
            oss << w.text << "\t" << w.bbox.x << "\t" << w.bbox.y << "\t" << w.bbox.width << "\t" << w.bbox.height << "\t" << w.confidence << "\n";
        }
        out.tsv = oss.str();

        out.words.reserve(words.size());
        for (const auto& w : words) {
            core::ports::text_recognition::Word wr;
            wr.bbox.x = w.bbox.x;
            wr.bbox.y = w.bbox.y;
            wr.bbox.width = w.bbox.width;
            wr.bbox.height = w.bbox.height;
            wr.text = w.text;
            wr.confidence = w.confidence;
            out.words.push_back(wr);
        }

        if (req.cancelFlag && req.cancelFlag->load()) return out;

        if (req.kind == core::ports::text_recognition::ExtractRequest::Kind::Table && !req.cells.empty()) {
            core::ports::text_recognition::Table t;
            t.cells = req.cells;

            auto inCell = [](const core::ports::text_recognition::Rect& w, const core::ports::text_recognition::Rect& c) {
                int cx = w.x + w.width / 2;
                int cy = w.y + w.height / 2;
                return cx >= c.x && cx < (c.x + c.width) && cy >= c.y && cy < (c.y + c.height);
            };

            struct WRef {
                const core::ports::text_recognition::Word* w;
                int cy;
            };

            for (auto& cell : t.cells) {
                if (req.cancelFlag && req.cancelFlag->load()) break;
                std::vector<WRef> cellWords;
                cellWords.reserve(out.words.size() / 4);
                for (const auto& w : out.words) {
                    if (!inCell(w.bbox, cell.bbox)) continue;
                    cellWords.push_back(WRef{ &w, w.bbox.y + w.bbox.height / 2 });
                }

                if (cellWords.empty()) {
                    cell.text.clear();
                    cell.confidence = 0;
                    continue;
                }

                std::sort(cellWords.begin(), cellWords.end(), [](const WRef& a, const WRef& b) {
                    if (a.cy != b.cy) return a.cy < b.cy;
                    return a.w->bbox.x < b.w->bbox.x;
                });

                int avgH = 0;
                for (const auto& cw : cellWords) avgH += cw.w->bbox.height;
                avgH = std::max(1, avgH / static_cast<int>(cellWords.size()));
                int lineTol = std::max(4, avgH / 2);

                    std::vector<std::vector<const core::ports::text_recognition::Word*>> lines;
                std::vector<int> lineY;

                for (const auto& cw : cellWords) {
                    if (lines.empty() || std::abs(cw.cy - lineY.back()) > lineTol) {
                        lines.push_back({ cw.w });
                        lineY.push_back(cw.cy);
                    } else {
                        lines.back().push_back(cw.w);
                        lineY.back() = (lineY.back() + cw.cy) / 2;
                    }
                }

                std::ostringstream text;
                int sumConf = 0;
                int n = 0;

                for (size_t li = 0; li < lines.size(); ++li) {
                    auto& ln = lines[li];
                    std::sort(ln.begin(), ln.end(), [](const auto* a, const auto* b) { return a->bbox.x < b->bbox.x; });
                    for (size_t wi = 0; wi < ln.size(); ++wi) {
                        if (wi) text << ' ';
                        text << ln[wi]->text;
                        sumConf += ln[wi]->confidence;
                        ++n;
                    }
                    if (li + 1 < lines.size()) text << "\n";
                }

                cell.text = text.str();
                cell.confidence = n > 0 ? (sumConf / n) : 0;
            }

            out.tables.push_back(std::move(t));
        }

        return out;
    }

private:
    std::shared_ptr<core::ports::diagnostics::IDiagnostics> debugger;
};

std::shared_ptr<core::ports::text_recognition::ITextRecognizer> createTextRecognizerAdapter() {
    return std::make_shared<TesseractTextRecognizerAdapter>();
}

std::shared_ptr<core::ports::text_recognition::ITextRecognizer> createTextRecognizerAdapter(std::shared_ptr<core::ports::diagnostics::IDiagnostics> debugger) {
    return std::make_shared<TesseractTextRecognizerAdapter>(std::move(debugger));
}
