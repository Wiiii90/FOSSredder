/**
 * @file core/src/utils/Util.cpp
 * @brief Implements private string helpers shared across `core` implementation files.
 */

#include "Util.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace core::utils {

std::string trim(std::string s) {
    const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

std::string lowerAscii(std::string s) {
    for (auto& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    return s;
}

std::string collapseWhitespace(std::string s) {
    static const std::regex spacesRe(R"(\s+)");
    s = std::regex_replace(std::move(s), spacesRe, " ");
    return trim(std::move(s));
}

std::vector<std::string> splitWhitespace(const std::string& s) {
    std::istringstream iss(s);
    std::vector<std::string> out;
    std::string t;
    while (iss >> t) out.push_back(std::move(t));
    return out;
}

int rightEdgeFromWordSpans(const std::vector<std::pair<int, int>>& wordSpans) {
    if (wordSpans.empty()) return 0;
    return wordSpans.back().second;
}

}
