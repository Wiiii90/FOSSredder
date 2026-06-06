/**
 * @file app/src/Environment.cpp
 * @brief Implements application-local environment bootstrap helpers.
 */

#include "app/Environment.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace app::runtime {

namespace {

void trimInPlace(std::string& value) {
  const auto isNotSpace = [](unsigned char c) {
    return !std::isspace(c);
  };
  const auto first = std::find_if(value.begin(), value.end(), isNotSpace);
  const auto last =
      std::find_if(value.rbegin(), value.rend(), isNotSpace).base();
  if (last <= first) {
    value.clear();
    return;
  }

  value = std::string(first, last);
}

std::string upperAscii(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) {
                   return static_cast<char>(std::toupper(c));
                 });
  return value;
}

#ifdef _WIN32
std::string readEnvironmentValue(const std::string& key) {
  const DWORD length = GetEnvironmentVariableA(key.c_str(), nullptr, 0);
  if (length == 0) {
    return {};
  }

  std::string buffer(length, '\0');
  GetEnvironmentVariableA(key.c_str(), buffer.data(), length);
  if (!buffer.empty() && buffer.back() == '\0') {
    buffer.pop_back();
  }
  return buffer;
}

void setEnvironmentValue(const std::string& key, const std::string& value) {
  SetEnvironmentVariableA(key.c_str(), value.c_str());
}
#else
std::string readEnvironmentValue(const std::string& key) {
  const char* value = std::getenv(key.c_str());
  return value ? std::string(value) : std::string();
}

void setEnvironmentValue(const std::string& key, const std::string& value) {
  setenv(key.c_str(), value.c_str(), 1);
}
#endif

void mergePathValue(const std::string& key, const std::string& value) {
  if (value.empty()) {
    return;
  }

  const std::string existing = readEnvironmentValue(key);
  if (existing.empty()) {
    setEnvironmentValue(key, value);
    return;
  }

  if (existing.find(value) != std::string::npos) {
    return;
  }

#ifdef _WIN32
  std::string merged = value;
  if (merged.back() != ';') {
    merged.push_back(';');
  }
#else
  std::string merged = value;
  if (merged.back() != ':') {
    merged.push_back(':');
  }
#endif
  merged += existing;
  setEnvironmentValue(key, merged);
}

} // namespace

bool loadDotEnv(const std::string& path, bool overwrite) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return false;
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) {
      continue;
    }
    if (line[0] == '#' || line.rfind("//", 0) == 0) {
      continue;
    }

    const auto separator = line.find('=');
    if (separator == std::string::npos) {
      continue;
    }

    std::string key = line.substr(0, separator);
    std::string value = line.substr(separator + 1);
    trimInPlace(key);
    trimInPlace(value);
    if (key.empty()) {
      continue;
    }

    if (!overwrite) {
      const std::string existing = readEnvironmentValue(key);
      if (!existing.empty()) {
        if (upperAscii(key) == "PATH") {
          mergePathValue(key, value);
        }
        continue;
      }
    }

    setEnvironmentValue(key, value);
  }

  return true;
}

} // namespace app::runtime
