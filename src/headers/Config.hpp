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
  float animationIdleWaveSpeed = 1.0f;
  float animationTalkWaveSpeed = 1.0f;
  float animationIdleWaveAmpX = 4.0f;
  float animationTalkWaveAmpX = 4.0f;
  float animationIdleWaveAmpY = 3.0f;
  float animationTalkWaveAmpY = 3.0f;
  float animationIdleShakeSpeed = 1.0f;
  float animationTalkShakeSpeed = 1.0f;
  float animationIdleShakeAmpX = 4.0f;
  float animationTalkShakeAmpX = 4.0f;
  float animationIdleShakeAmpY = 3.0f;
  float animationTalkShakeAmpY = 3.0f;
  float applicationFontSize = 12.0f;
  Color background = {255, 255, 255, 255};
  bool autoscaleEnabled = true;
  float offsetX = 0.0f;
  float offsetY = 0.0f;
  float scale = 1.0f;
};

AppConfig DefaultConfig();
bool LoadConfig(const char *path, AppConfig *config);
bool SaveConfig(const char *path, const AppConfig &config);
