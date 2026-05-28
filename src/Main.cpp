#define RAYGUI_IMPLEMENTATION
#include "headers/Avatar.hpp"
#include "headers/Config.hpp"
#include "headers/ImagePicker.hpp"
#include "headers/Microphone.hpp"
#include "headers/Utils.hpp"
#include "headers/raygui.h"
#include "portaudio.h"
#include <algorithm>
#include <raylib.h>
#include <vector>

namespace {
Avatar::AnimationState AnimationFromIndex(int index) {
  switch (index) {
  case 1:
    return Avatar::WAVE;
  case 2:
    return Avatar::SHAKE;
  case 0:
  default:
    return Avatar::NONE;
  }
}
} // namespace

int main(void) {
  Pa_Initialize();
  // -- Window Config --
  SetTraceLogLevel(LOG_ERROR);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1280, 720, "Me2PNG v0.4.1");
  SetTargetFPS(60);
  // GuiSetStyle(DEFAULT, TEXT_SIZE, 12);
  // GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(RED));
  std::string fontPath = AssetPath("DynaPuff.ttf");
  Font customFont = LoadFont(fontPath.c_str());
  GuiSetFont(customFont);

  const char *configPath = "./config.json";
  AppConfig config = DefaultConfig();
  LoadConfig(configPath, &config);

  ImagePicker imagePicker;
  imagePicker.SetWorkingDirectory(GetWorkingDirectory());
  imagePicker.SetPaths(config.images);
  ImagePaths &imagePaths = imagePicker.Paths();

  // -- Microphone Setup --
  std::vector<std::string> microphones = GetMicrophoneList();
  int micActive = 0;
  for (size_t i = 0; i < microphones.size(); ++i) {
    if (microphones[i] == config.micName) {
      micActive = (int)i;
      break;
    }
  }
  std::string micText = "";
  for (size_t i = 0; i < microphones.size(); ++i) {
    micText += microphones[i];
    if (i + 1 < microphones.size()) {
      micText += ";";
    }
  }

  // -- Config State --
  bool dropdownOpen = false;
  float applicationFontSize = config.applicationFontSize;
  float rmsThreshold = config.threshold;
  const float thresholdMin = 0.0f;
  const float thresholdMax = 5000.0f;
  const float rmsBarMax = 5000.0f;
  bool blinkEnabled = config.blinkEnabled;
  float blinkMinSeconds = config.blinkMinSeconds;
  float blinkMaxSeconds = config.blinkMaxSeconds;
  float blinkDuration = config.blinkDuration;
  int idleAnimationIndex = config.idleAnimation;
  int talkAnimationIndex = config.talkAnimation;
  Color backgroundColor = config.background;
  bool autoscaleEnabled = config.autoscaleEnabled;
  Vector3 backgroundHsv = ColorToHSV(backgroundColor);
  bool configVisible = true;
  Vector2 configScroll = {0.0f, 0.0f};
  Rectangle configView = {};
  bool idleAnimDropdownOpen = false;
  bool talkAnimDropdownOpen = false;
  AnimationWaveShakeParams animationIdleWave = {config.animationIdleWaveSpeed,
                                                config.animationIdleWaveAmpX,
                                                config.animationIdleWaveAmpY};
  AnimationWaveShakeParams animationIdleShake = {config.animationIdleShakeSpeed,
                                                 config.animationIdleShakeAmpX,
                                                 config.animationIdleShakeAmpY};
  AnimationWaveShakeParams animationTalkWave = {config.animationTalkWaveSpeed,
                                                config.animationTalkWaveAmpX,
                                                config.animationTalkWaveAmpY};
  AnimationWaveShakeParams animationTalkShake = {config.animationTalkShakeSpeed,
                                                 config.animationTalkShakeAmpX,
                                                 config.animationTalkShakeAmpY};
  bool configDirty = false;
  float offsetX = config.offsetX;
  float offsetY = config.offsetY;
  float scale = config.scale;

  // -- Avatar Setup --
  Avatar avatar;
  avatar.scale = autoscaleEnabled ? 1.0f : scale;
  GuiSetStyle(DEFAULT, TEXT_SIZE, (int)applicationFontSize);

  // -- Clamp values --
  rmsThreshold = std::clamp(rmsThreshold, thresholdMin, thresholdMax);
  idleAnimationIndex = std::clamp(idleAnimationIndex, 0, 2);
  talkAnimationIndex = std::clamp(talkAnimationIndex, 0, 2);
  if (blinkMinSeconds > blinkMaxSeconds) {
    std::swap(blinkMinSeconds, blinkMaxSeconds);
    config.blinkMinSeconds = blinkMinSeconds;
    config.blinkMaxSeconds = blinkMaxSeconds;
    configDirty = true;
  }
  blinkMinSeconds = std::max(0.1f, blinkMinSeconds);
  blinkMaxSeconds = std::max(blinkMinSeconds, blinkMaxSeconds);
  blinkDuration = std::max(0.05f, blinkDuration);
  SpriteMode currentMode = SpriteMode::None;
  double blinkEndTime = 0.0;
  double nextBlinkTime = 0.0;
  double talkEndTime = 0.0;
  const double talkHoldSeconds = 0.15;

  // -- Audio Input Setup --
  PaStream *audio_stream = nullptr;
  if (!microphones.empty()) {
    StartAudioCapture(&audio_stream, microphones[micActive]);
    std::snprintf(config.micName, kPathMax, "%s",
                  microphones[micActive].c_str());
  }
  SetThreshold(rmsThreshold);
  nextBlinkTime = GetTime() + GetRandomValue((int)(blinkMinSeconds * 100.0f),
                                             (int)(blinkMaxSeconds * 100.0f)) /
                                  100.0f;

  avatar.LoadSprite(imagePaths.idle);
  avatar.StopAnimation();
  currentMode = SpriteMode::Idle;

  // -- Main Loop --
  while (!WindowShouldClose()) {
    float delta = GetFrameTime() * 60.0f;
    double now = GetTime();
    float rmsValue = GetRMS();
    if (rmsValue >= rmsThreshold) {
      talkEndTime = now + talkHoldSeconds;
    }
    bool isTalking = now < talkEndTime;
    float blinkMin = std::max(0.1f, blinkMinSeconds);
    float blinkMax = std::max(blinkMin, blinkMaxSeconds);
    float blinkDurationSeconds = std::max(0.05f, blinkDuration);
    bool hasBlinkTextures = blinkEnabled && (FileExists(imagePaths.blinkIdle) ||
                                             FileExists(imagePaths.blinkTalk));
    if (hasBlinkTextures && now >= nextBlinkTime) {
      blinkEndTime = now + blinkDurationSeconds;
      nextBlinkTime = now + GetRandomValue((int)(blinkMin * 100.0f),
                                           (int)(blinkMax * 100.0f)) /
                                100.0f;
    }
    bool isBlinking = hasBlinkTextures && now < blinkEndTime;
    SpriteMode desiredMode = SpriteMode::Idle;

    if (IsWindowResized() && autoscaleEnabled) {
      avatar.AutoScale();
    }

    BeginDrawing();

    ClearBackground(backgroundColor);

    if (isTalking) {
      desiredMode = isBlinking && FileExists(imagePaths.blinkTalk)
                        ? SpriteMode::BlinkTalk
                        : SpriteMode::Talk;
    } else {
      desiredMode = isBlinking && FileExists(imagePaths.blinkIdle)
                        ? SpriteMode::BlinkIdle
                        : SpriteMode::Idle;
    }

    if (desiredMode != currentMode) {
      const char *nextPath = nullptr;
      switch (desiredMode) {
      case SpriteMode::Idle:
        nextPath = imagePaths.idle;
        break;
      case SpriteMode::Talk:
        nextPath = imagePaths.talk;
        break;
      case SpriteMode::BlinkIdle:
        nextPath = imagePaths.blinkIdle;
        break;
      case SpriteMode::BlinkTalk:
        nextPath = imagePaths.blinkTalk;
        break;
      case SpriteMode::None:
        break;
      }

      if (nextPath != nullptr && FileExists(nextPath)) {
        if (avatar.texture.id != 0) {
          avatar.UnloadSprite();
        }
        avatar.LoadSprite(nextPath);
        avatar.StopAnimation();
        currentMode = desiredMode;
      }
    }

    Avatar::AnimationState animationState =
        isTalking ? AnimationFromIndex(talkAnimationIndex)
                  : AnimationFromIndex(idleAnimationIndex);
    switch (animationState) {
    case Avatar::WAVE:
      avatar.AnimateWave(isTalking ? animationTalkWave : animationIdleWave);
      break;
    case Avatar::SHAKE:
      avatar.AnimationShake(isTalking ? animationTalkShake
                                      : animationIdleShake);
      break;
    case Avatar::NONE:
      avatar.StopAnimation();
      break;
    }
    avatar.TranslateX(offsetX);
    avatar.TranslateY(offsetY);
    avatar.DrawSprite();

    // -- Config Panel --
    if (configVisible) {
      DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                    Color{0, 0, 0, 120});

      float panelWidth = std::min(900.0f, (float)GetScreenWidth() - 20.0f);
      float panelHeight = std::min(640.0f, (float)GetScreenHeight() - 20.0f);
      Rectangle configBounds = {10.0f, 10.0f, panelWidth, panelHeight};
      Rectangle hideButtonRect = {configBounds.x + configBounds.width - 28.0f,
                                  configBounds.y + 4.0f, 24.0f, 20.0f};

      const float spacing = 8.0f;
      const float sectionHeight = 18.0f;
      const float rowHeight = 20.0f;
      const float dropdownHeight = 28.0f;
      const float colorPickerHeight = 180.0f;
      float contentHeight = 0.0f;
      auto addHeight = [&](float height) { contentHeight += height + spacing; };

      int dropdownSpacing = GuiGetStyle(DROPDOWNBOX, DROPDOWN_ITEMS_SPACING);
      int animationOptionCount = 3;
      bool idleHasParams =
          AnimationFromIndex(idleAnimationIndex) != Avatar::NONE;
      bool talkHasParams =
          AnimationFromIndex(talkAnimationIndex) != Avatar::NONE;
      float micDropdownOffset = dropdownOpen && !microphones.empty()
                                    ? static_cast<float>(microphones.size()) *
                                          (dropdownHeight + dropdownSpacing)
                                    : 0.0f;
      float idleAnimDropdownOffset =
          idleAnimDropdownOpen ? static_cast<float>(animationOptionCount) *
                                     (dropdownHeight + dropdownSpacing)
                               : 0.0f;
      float talkAnimDropdownOffset =
          talkAnimDropdownOpen ? static_cast<float>(animationOptionCount) *
                                     (dropdownHeight + dropdownSpacing)
                               : 0.0f;

      addHeight(sectionHeight);
      addHeight(dropdownHeight + micDropdownOffset);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(sectionHeight);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(sectionHeight);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(sectionHeight);
      addHeight(dropdownHeight + idleAnimDropdownOffset);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(dropdownHeight + talkAnimDropdownOffset);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(rowHeight);
      addHeight(sectionHeight);
      addHeight(colorPickerHeight);
      addHeight(sectionHeight);
      addHeight(rowHeight);
      contentHeight += 10.0f;

      Rectangle content = {0.0f, 0.0f, configBounds.width - 20.0f,
                           contentHeight};
      int prevScrollbarWidth = GuiGetStyle(SCROLLBAR, SCROLLBAR_WIDTH);
      GuiSetStyle(SCROLLBAR, SCROLLBAR_WIDTH, 0);
      GuiScrollPanel(configBounds, "Config", content, &configScroll,
                     &configView);
      GuiSetStyle(SCROLLBAR, SCROLLBAR_WIDTH, prevScrollbarWidth);

      BeginScissorMode((int)configView.x, (int)configView.y,
                       (int)configView.width, (int)configView.height);
      float contentX = configView.x + configScroll.x + 8.0f;
      float contentY = configView.y + configScroll.y + 8.0f;
      float labelWidth = 110.0f;
      float contentWidth = configView.width - 16.0f;
      int scrollbarWidth = GuiGetStyle(LISTVIEW, SCROLLBAR_WIDTH);
      int scrollbarPad = GuiGetStyle(SCROLLBAR, SCROLL_PADDING);
      float controlWidth =
          std::max(80.0f, contentWidth - labelWidth - 50.0f -
                              (float)scrollbarWidth - (float)scrollbarPad);

      auto nextRow = [&](float height) {
        Rectangle row = {contentX, contentY, contentWidth, height};
        contentY += height + spacing;
        return row;
      };

      auto drawSection = [&](const char *title) {
        Rectangle row = nextRow(sectionHeight);
        GuiLine(row, title);
      };

      auto drawLabeled = [&](const char *label, float height) {
        Rectangle row = nextRow(height);
        Rectangle labelRect = {row.x, row.y, labelWidth, row.height};
        Rectangle controlRect = {row.x + labelWidth + 8.0f, row.y, controlWidth,
                                 row.height};
        GuiLabel(labelRect, label);
        return controlRect;
      };

      auto drawAnimParams = [&](AnimationWaveShakeParams &params) {
        Rectangle speed = drawLabeled("Speed (x)", rowHeight);
        float prevSpeed = params.speed;
        GuiSliderBar(speed, "", TextFormat("%.2f", params.speed), &params.speed,
                     0.1f, 20.0f);
        if (params.speed != prevSpeed) {
          params.speed = std::max(0.1f, params.speed);
          configDirty = true;
        }

        Rectangle ampX = drawLabeled("Amplitude X", rowHeight);
        float prevAmpX = params.ampX;
        GuiSliderBar(ampX, "", TextFormat("%.2f", params.ampX), &params.ampX,
                     0.0f, 20.0f);
        if (params.ampX != prevAmpX) {
          params.ampX = std::max(0.0f, params.ampX);
          configDirty = true;
        }

        Rectangle ampY = drawLabeled("Amplitude Y", rowHeight);
        float prevAmpY = params.ampY;
        GuiSliderBar(ampY, "", TextFormat("%.2f", params.ampY), &params.ampY,
                     0.0f, 20.0f);
        if (params.ampY != prevAmpY) {
          params.ampY = std::max(0.0f, params.ampY);
          configDirty = true;
        }
      };

      drawSection("Audio");
      Rectangle micRect = drawLabeled("Microphone", dropdownHeight);
      if (GuiDropdownBox(micRect, micText.c_str(), &micActive, dropdownOpen)) {
        dropdownOpen = !dropdownOpen;
        if (!dropdownOpen && !microphones.empty() && micActive >= 0 &&
            (size_t)micActive < microphones.size()) {
          StopAudioCapture(audio_stream);
          StartAudioCapture(&audio_stream, microphones[micActive]);
          std::snprintf(config.micName, kPathMax, "%s",
                        microphones[micActive].c_str());
          configDirty = true;
        }
      }
      if (micDropdownOffset > 0.0f) {
        contentY += micDropdownOffset;
      }

      Rectangle thresholdRect = drawLabeled("Threshold", rowHeight);
      float previousThreshold = rmsThreshold;
      GuiSliderBar(thresholdRect, "", TextFormat("%.0f", rmsThreshold),
                   &rmsThreshold, thresholdMin, thresholdMax);
      if (rmsThreshold != previousThreshold) {
        SetThreshold(rmsThreshold);
        config.threshold = rmsThreshold;
        configDirty = true;
      }

      Rectangle rmsRect = drawLabeled("RMS", rowHeight);
      float rmsDisplay = rmsValue;
      if (rmsDisplay < 0.0f) {
        rmsDisplay = 0.0f;
      } else if (rmsDisplay > rmsBarMax) {
        rmsDisplay = rmsBarMax;
      }
      GuiProgressBar(rmsRect, "", TextFormat("%.0f", rmsDisplay), &rmsDisplay,
                     0.0f, rmsBarMax);

      drawSection("Images");
      Rectangle row = nextRow(rowHeight);
      Rectangle labelRect = {row.x, row.y, labelWidth, row.height};
      Rectangle buttonRect = {row.x + labelWidth + 8.0f, row.y, 24.0f,
                              row.height};
      Rectangle valueRect = {buttonRect.x + buttonRect.width + 6.0f, row.y,
                             row.width - (buttonRect.x - row.x) -
                                 buttonRect.width - 6.0f,
                             row.height};
      imagePicker.DrawRow(labelRect, buttonRect, valueRect, "Idle",
                          SpriteMode::Idle);

      row = nextRow(rowHeight);
      labelRect = {row.x, row.y, labelWidth, row.height};
      buttonRect = {row.x + labelWidth + 8.0f, row.y, 24.0f, row.height};
      valueRect = {buttonRect.x + buttonRect.width + 6.0f, row.y,
                   row.width - (buttonRect.x - row.x) - buttonRect.width - 6.0f,
                   row.height};
      imagePicker.DrawRow(labelRect, buttonRect, valueRect, "Talk",
                          SpriteMode::Talk);

      row = nextRow(rowHeight);
      labelRect = {row.x, row.y, labelWidth, row.height};
      buttonRect = {row.x + labelWidth + 8.0f, row.y, 24.0f, row.height};
      valueRect = {buttonRect.x + buttonRect.width + 6.0f, row.y,
                   row.width - (buttonRect.x - row.x) - buttonRect.width - 6.0f,
                   row.height};
      imagePicker.DrawRow(labelRect, buttonRect, valueRect, "Blink Idle",
                          SpriteMode::BlinkIdle);

      row = nextRow(rowHeight);
      labelRect = {row.x, row.y, labelWidth, row.height};
      buttonRect = {row.x + labelWidth + 8.0f, row.y, 24.0f, row.height};
      valueRect = {buttonRect.x + buttonRect.width + 6.0f, row.y,
                   row.width - (buttonRect.x - row.x) - buttonRect.width - 6.0f,
                   row.height};
      imagePicker.DrawRow(labelRect, buttonRect, valueRect, "Blink Talk",
                          SpriteMode::BlinkTalk);

      Rectangle moveXRect = drawLabeled("Move X", rowHeight);
      float prevOffsetX = offsetX;
      GuiSliderBar(moveXRect, "", TextFormat("%.2f", offsetX), &offsetX,
                   -(GetScreenWidth() + avatar.rect.width),
                   GetScreenWidth() + avatar.rect.width);
      if (offsetX != prevOffsetX) {
        config.offsetX = offsetX;
        configDirty = true;
      }
      Rectangle moveYRect = drawLabeled("Move Y", rowHeight);
      float prevOffsetY = offsetY;
      GuiSliderBar(moveYRect, "", TextFormat("%.2f", offsetY), &offsetY,
                   -(GetScreenHeight() + avatar.rect.height),
                   GetScreenHeight() + avatar.rect.height);
      if (offsetY != prevOffsetY) {
        config.offsetY = offsetY;
        configDirty = true;
      }
      Rectangle scaleRect = drawLabeled("Scale", rowHeight);
      float prevScale = scale;
      if (!autoscaleEnabled) {
        GuiSliderBar(scaleRect, "", TextFormat("%.2f", avatar.scale),
                     &avatar.scale, 0.1f, 10.0f);
      } else {
        GuiLabel(scaleRect, "Turn off autoscale to adjust");
      }
      if (avatar.scale != prevScale) {
        config.scale = avatar.scale;
        configDirty = true;
      }

      drawSection("Blink");
      Rectangle blinkEnabledRect = drawLabeled("Enabled", rowHeight);
      bool blinkPrevious = blinkEnabled;
      Rectangle checkboxRect = {blinkEnabledRect.x, blinkEnabledRect.y,
                                rowHeight, rowHeight};
      GuiCheckBox(checkboxRect, "", &blinkEnabled);
      if (blinkEnabled != blinkPrevious) {
        config.blinkEnabled = blinkEnabled;
        configDirty = true;
      }

      Rectangle blinkMinRect = drawLabeled("Min (s)", rowHeight);
      float prevBlinkMin = blinkMinSeconds;
      GuiSliderBar(blinkMinRect, "", TextFormat("%.2f", blinkMinSeconds),
                   &blinkMinSeconds, 0.1f, blinkMaxSeconds);
      if (blinkMinSeconds != prevBlinkMin) {
        config.blinkMinSeconds = blinkMinSeconds;
        configDirty = true;
      }

      Rectangle blinkMaxRect = drawLabeled("Max (s)", rowHeight);
      float prevBlinkMax = blinkMaxSeconds;
      GuiSliderBar(blinkMaxRect, "", TextFormat("%.2f", blinkMaxSeconds),
                   &blinkMaxSeconds, 0.1f, 10.0f);
      if (blinkMaxSeconds != prevBlinkMax) {
        config.blinkMaxSeconds = blinkMaxSeconds;
        configDirty = true;
      }

      Rectangle blinkDurationRect = drawLabeled("Duration", rowHeight);
      float prevBlinkDuration = blinkDuration;
      GuiSliderBar(blinkDurationRect, "", TextFormat("%.2f", blinkDuration),
                   &blinkDuration, 0.05f, 0.5f);
      if (blinkDuration != prevBlinkDuration) {
        config.blinkDuration = blinkDuration;
        configDirty = true;
      }

      drawSection("Animation");
      const char *animationOptions = "None;Wave;Shake";
      Rectangle idleAnimRect = drawLabeled("Idle", dropdownHeight);
      int prevIdleAnim = idleAnimationIndex;
      if (GuiDropdownBox(idleAnimRect, animationOptions, &idleAnimationIndex,
                         idleAnimDropdownOpen)) {
        idleAnimDropdownOpen = !idleAnimDropdownOpen;
      }
      if (idleAnimDropdownOffset > 0.0f) {
        contentY += idleAnimDropdownOffset;
      }
      if (idleAnimationIndex != prevIdleAnim) {
        config.idleAnimation = idleAnimationIndex;
        configDirty = true;
      }

      if (idleHasParams) {
        AnimationWaveShakeParams &idleParams =
            AnimationFromIndex(idleAnimationIndex) == Avatar::WAVE
                ? animationIdleWave
                : animationIdleShake;
        drawAnimParams(idleParams);
        config.animationIdleWaveSpeed = animationIdleWave.speed;
        config.animationIdleWaveAmpX = animationIdleWave.ampX;
        config.animationIdleWaveAmpY = animationIdleWave.ampY;
        config.animationIdleShakeSpeed = animationIdleShake.speed;
        config.animationIdleShakeAmpX = animationIdleShake.ampX;
        config.animationIdleShakeAmpY = animationIdleShake.ampY;
      }

      Rectangle talkAnimRect = drawLabeled("Talk", dropdownHeight);
      int prevTalkAnim = talkAnimationIndex;
      if (GuiDropdownBox(talkAnimRect, animationOptions, &talkAnimationIndex,
                         talkAnimDropdownOpen)) {
        talkAnimDropdownOpen = !talkAnimDropdownOpen;
      }
      if (talkAnimDropdownOffset > 0.0f) {
        contentY += talkAnimDropdownOffset;
      }
      if (talkAnimationIndex != prevTalkAnim) {
        config.talkAnimation = talkAnimationIndex;
        configDirty = true;
      }

      if (talkHasParams) {
        AnimationWaveShakeParams &talkParams =
            AnimationFromIndex(talkAnimationIndex) == Avatar::WAVE
                ? animationTalkWave
                : animationTalkShake;
        drawAnimParams(talkParams);
        config.animationTalkWaveSpeed = animationTalkWave.speed;
        config.animationTalkWaveAmpX = animationTalkWave.ampX;
        config.animationTalkWaveAmpY = animationTalkWave.ampY;
        config.animationTalkShakeSpeed = animationTalkShake.speed;
        config.animationTalkShakeAmpX = animationTalkShake.ampX;
        config.animationTalkShakeAmpY = animationTalkShake.ampY;
      }

      drawSection("Background");
      Rectangle colorRect = nextRow(colorPickerHeight);
      Vector3 prevHsv = backgroundHsv;
      float hueWidth = (float)GuiGetStyle(COLORPICKER, HUEBAR_WIDTH);
      float huePadding = (float)GuiGetStyle(COLORPICKER, HUEBAR_PADDING);
      float colorPanelWidth =
          std::max(60.0f, colorRect.width - hueWidth - huePadding);
      Rectangle panelRect = {colorRect.x, colorRect.y, colorPanelWidth,
                             colorRect.height};
      Rectangle hueRect = {panelRect.x + panelRect.width + huePadding,
                           colorRect.y, hueWidth, colorRect.height};
      GuiColorPanelHSV(panelRect, "", &backgroundHsv);
      float hue = backgroundHsv.x;
      GuiColorBarHue(hueRect, "", &hue);
      backgroundHsv.x = hue;
      if (backgroundHsv.x != prevHsv.x || backgroundHsv.y != prevHsv.y ||
          backgroundHsv.z != prevHsv.z) {
        unsigned char prevAlpha = backgroundColor.a;
        backgroundColor =
            ColorFromHSV(backgroundHsv.x, backgroundHsv.y, backgroundHsv.z);
        backgroundColor.a = prevAlpha;
        config.background = backgroundColor;
        configDirty = true;
      }

      drawSection("Application");
      Rectangle fontSizeRect = drawLabeled("Font Size (px)", rowHeight);
      float prevFontSize = applicationFontSize;
      GuiSliderBar(fontSizeRect, "", TextFormat("%.2f", applicationFontSize),
                   &applicationFontSize, 8.0f, 32.0f);
      if (applicationFontSize != prevFontSize) {
        config.applicationFontSize = applicationFontSize;
        GuiSetStyle(DEFAULT, TEXT_SIZE, (int)applicationFontSize);
        configDirty = true;
      }

      Rectangle autoscaleEnabledRect = drawLabeled("Autoscale", rowHeight);
      bool autoscalePrevious = autoscaleEnabled;
      Rectangle checkboxAutoScaleRect = {
          autoscaleEnabledRect.x, autoscaleEnabledRect.y, rowHeight, rowHeight};
      GuiCheckBox(checkboxAutoScaleRect, "", &autoscaleEnabled);
      if (autoscaleEnabled != autoscalePrevious) {
        config.autoscaleEnabled = autoscaleEnabled;
        configDirty = true;
      }

      EndScissorMode();

      if (GuiButton(hideButtonRect, GuiIconText(ICON_CROSS, ""))) {
        configVisible = false;
        dropdownOpen = false;
        idleAnimDropdownOpen = false;
        talkAnimDropdownOpen = false;
        imagePicker.CloseDialog();
      }

      if (imagePicker.DrawDialog()) {
        config.images = imagePaths;
        configDirty = true;
        currentMode = SpriteMode::None;
      }
    } else {
      if (IsWindowFocused()) {
        Rectangle showButtonRect = {4.0f, 4.0f, 24.0f, 24.0f};
        if (GuiButton(showButtonRect, GuiIconText(ICON_GEAR, ""))) {
          configVisible = true;
        }
      }
    }

    if (configDirty) {
      SaveConfig(configPath, config);
      configDirty = false;
    }

    EndDrawing();
  }

  StopAudioCapture(audio_stream);
  if (avatar.texture.id != 0) {
    avatar.UnloadSprite();
  }
  imagePicker.Cleanup();
  CloseWindow();
  return 0;
}