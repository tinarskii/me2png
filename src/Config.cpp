#include "headers/Config.hpp"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

namespace {
bool ReadFile(const char *path, std::string *out) {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    return false;
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  *out = ss.str();
  return true;
}

size_t FindKey(const std::string &data, const char *key) {
  std::string token = "\"";
  token += key;
  token += "\"";
  return data.find(token);
}

const char *SkipWhitespace(const char *ptr) {
  while (ptr && *ptr && std::isspace(static_cast<unsigned char>(*ptr))) {
    ++ptr;
  }
  return ptr;
}

bool ReadFloatValue(const std::string &data, const char *key, float *out) {
  size_t keyPos = FindKey(data, key);
  if (keyPos == std::string::npos) {
    return false;
  }
  size_t colon = data.find(':', keyPos);
  if (colon == std::string::npos) {
    return false;
  }
  const char *start = SkipWhitespace(data.c_str() + colon + 1);
  if (start == nullptr || *start == '\0') {
    return false;
  }
  char *end = nullptr;
  float value = std::strtof(start, &end);
  if (start == end) {
    return false;
  }
  *out = value;
  return true;
}

bool ReadIntValue(const std::string &data, const char *key, int *out) {
  float value = 0.0f;
  if (!ReadFloatValue(data, key, &value)) {
    return false;
  }
  *out = static_cast<int>(value);
  return true;
}

bool ReadBoolValue(const std::string &data, const char *key, bool *out) {
  size_t keyPos = FindKey(data, key);
  if (keyPos == std::string::npos) {
    return false;
  }
  size_t colon = data.find(':', keyPos);
  if (colon == std::string::npos) {
    return false;
  }
  const char *start = SkipWhitespace(data.c_str() + colon + 1);
  if (start == nullptr) {
    return false;
  }
  if (std::strncmp(start, "true", 4) == 0) {
    *out = true;
    return true;
  }
  if (std::strncmp(start, "false", 5) == 0) {
    *out = false;
    return true;
  }
  return false;
}

bool ReadStringValue(const std::string &data, const char *key,
                     std::string *out) {
  size_t keyPos = FindKey(data, key);
  if (keyPos == std::string::npos) {
    return false;
  }
  size_t colon = data.find(':', keyPos);
  if (colon == std::string::npos) {
    return false;
  }
  const char *start = SkipWhitespace(data.c_str() + colon + 1);
  if (start == nullptr || *start != '"') {
    return false;
  }
  ++start;
  std::string value;
  while (*start) {
    if (*start == '\\') {
      ++start;
      if (*start == '"' || *start == '\\') {
        value.push_back(*start);
      } else if (*start == 'n') {
        value.push_back('\n');
      } else if (*start == 't') {
        value.push_back('\t');
      } else if (*start == 'r') {
        value.push_back('\r');
      } else if (*start != '\0') {
        value.push_back(*start);
      }
    } else if (*start == '"') {
      *out = value;
      return true;
    } else {
      value.push_back(*start);
    }
    ++start;
  }
  return false;
}

bool ReadColorValue(const std::string &data, const char *key, Color *out) {
  size_t keyPos = FindKey(data, key);
  if (keyPos == std::string::npos) {
    return false;
  }
  size_t colon = data.find(':', keyPos);
  if (colon == std::string::npos) {
    return false;
  }
  const char *start = SkipWhitespace(data.c_str() + colon + 1);
  if (start == nullptr || *start != '[') {
    return false;
  }
  ++start;
  int values[4] = {0, 0, 0, 255};
  for (int i = 0; i < 4; ++i) {
    start = SkipWhitespace(start);
    if (start == nullptr || *start == '\0') {
      return false;
    }
    char *end = nullptr;
    long value = std::strtol(start, &end, 10);
    if (start == end) {
      return false;
    }
    values[i] = static_cast<int>(value);
    start = SkipWhitespace(end);
    if (i < 3) {
      if (*start != ',') {
        return false;
      }
      ++start;
    }
  }
  *out = {static_cast<unsigned char>(values[0]),
          static_cast<unsigned char>(values[1]),
          static_cast<unsigned char>(values[2]),
          static_cast<unsigned char>(values[3])};
  return true;
}

std::string EscapeJsonString(const char *value) {
  if (value == nullptr) {
    return "";
  }
  std::string result;
  for (const char *ptr = value; *ptr; ++ptr) {
    switch (*ptr) {
    case '\\':
      result += "\\\\";
      break;
    case '"':
      result += "\\\"";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\r':
      result += "\\r";
      break;
    case '\t':
      result += "\\t";
      break;
    default:
      result.push_back(*ptr);
      break;
    }
  }
  return result;
}
} // namespace

AppConfig DefaultConfig() {
  AppConfig config;
  std::snprintf(config.images.idle, kPathMax, "%s", "./assets/idle.png");
  std::snprintf(config.images.talk, kPathMax, "%s", "./assets/talk.png");
  std::snprintf(config.images.blinkIdle, kPathMax, "%s",
                "./assets/blink-idle.png");
  std::snprintf(config.images.blinkTalk, kPathMax, "%s",
                "./assets/blink-talk.png");
  return config;
}

bool LoadConfig(const char *path, AppConfig *config) {
  if (config == nullptr || path == nullptr) {
    return false;
  }
  std::string data;
  if (!ReadFile(path, &data)) {
    return false;
  }

  ReadFloatValue(data, "threshold", &config->threshold);
  ReadBoolValue(data, "blinkEnabled", &config->blinkEnabled);
  ReadFloatValue(data, "blinkMinSeconds", &config->blinkMinSeconds);
  ReadFloatValue(data, "blinkMaxSeconds", &config->blinkMaxSeconds);
  ReadFloatValue(data, "blinkDuration", &config->blinkDuration);
  ReadIntValue(data, "idleAnimation", &config->idleAnimation);
  ReadIntValue(data, "talkAnimation", &config->talkAnimation);
  ReadColorValue(data, "backgroundColor", &config->background);

  std::string value;
  if (ReadStringValue(data, "imageIdle", &value)) {
    std::snprintf(config->images.idle, kPathMax, "%s", value.c_str());
  }
  if (ReadStringValue(data, "imageTalk", &value)) {
    std::snprintf(config->images.talk, kPathMax, "%s", value.c_str());
  }
  if (ReadStringValue(data, "imageBlinkIdle", &value)) {
    std::snprintf(config->images.blinkIdle, kPathMax, "%s", value.c_str());
  }
  if (ReadStringValue(data, "imageBlinkTalk", &value)) {
    std::snprintf(config->images.blinkTalk, kPathMax, "%s", value.c_str());
  }
  if (ReadStringValue(data, "micName", &value)) {
    std::snprintf(config->micName, kPathMax, "%s", value.c_str());
  }

  return true;
}

bool SaveConfig(const char *path, const AppConfig &config) {
  if (path == nullptr) {
    return false;
  }
  std::ofstream file(path, std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    return false;
  }

  file << "{\n";
  file << "  \"threshold\": " << config.threshold << ",\n";
  file << "  \"blinkEnabled\": " << (config.blinkEnabled ? "true" : "false")
       << ",\n";
  file << "  \"blinkMinSeconds\": " << config.blinkMinSeconds << ",\n";
  file << "  \"blinkMaxSeconds\": " << config.blinkMaxSeconds << ",\n";
  file << "  \"blinkDuration\": " << config.blinkDuration << ",\n";
  file << "  \"idleAnimation\": " << config.idleAnimation << ",\n";
  file << "  \"talkAnimation\": " << config.talkAnimation << ",\n";
  file << "  \"backgroundColor\": [" << (int)config.background.r << ", "
       << (int)config.background.g << ", " << (int)config.background.b << ", "
       << (int)config.background.a << "],\n";
  file << "  \"micName\": \"" << EscapeJsonString(config.micName) << "\",\n";
  file << "  \"imageIdle\": \"" << EscapeJsonString(config.images.idle)
       << "\",\n";
  file << "  \"imageTalk\": \"" << EscapeJsonString(config.images.talk)
       << "\",\n";
  file << "  \"imageBlinkIdle\": \""
       << EscapeJsonString(config.images.blinkIdle) << "\",\n";
  file << "  \"imageBlinkTalk\": \""
       << EscapeJsonString(config.images.blinkTalk) << "\"\n";
  file << "}\n";
  return true;
}
