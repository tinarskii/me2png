#pragma once

#include "ImagePicker.hpp"
#include <raylib.h>

struct AppConfig {
  ImagePaths images;
  char micName[kPathMax]{};
  float threshold = 500.0f;
  bool blinkEnabled = true;
  float blinkMinSeconds = 2.0f;
  float blinkMaxSeconds = 5.0f;
  float blinkDuration = 0.12f;
  int idleAnimation = 1;
  int talkAnimation = 2;
  Color background = {255, 255, 255, 255};
};

AppConfig DefaultConfig();
bool LoadConfig(const char *path, AppConfig *config);
bool SaveConfig(const char *path, const AppConfig &config);
