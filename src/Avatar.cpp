#include "headers/Avatar.hpp"
#include <raylib.h>
#include <raymath.h>

void Avatar::LoadSprite(const char *filename) {
  texture = LoadTexture(filename);

  if (texture.id == 0) {
    TraceLog(LOG_ERROR, "Failed to load texture\n");
    rect = {0, 0, 0, 0};
    return;
  }

  rect = (Rectangle){0, 0, (float)texture.width, (float)texture.height};
}

void Avatar::UnloadSprite() { UnloadTexture(texture); }

void Avatar::DrawSprite() {
  float w = rect.width * scale;
  float h = rect.height * scale;

  DrawTextureEx(texture, position, 0.0f, scale, WHITE);
}

void Avatar::AutoScale() {
  float scaleX = (float)GetScreenWidth() / rect.width;
  float scaleY = (float)GetScreenHeight() / rect.height;

  scale = fminf(scaleX, scaleY);

  if (scale <= 0.0f) {
    TraceLog(LOG_ERROR, "Cannot determine scaling, falling back to 1.0f.\n");
    scale = 1.0f;
  }
}

void Avatar::AnimateWave(const AnimationWaveShakeParams &params) {
  float time = GetTime();
  float bobX = sinf(time * params.speed) * params.ampX;
  float bobY = cosf(time * params.speed) * params.ampY;

  position = {(GetScreenWidth() - rect.width * scale) / 2.0f + bobX,
              (GetScreenHeight() - rect.height * scale) / 2.0f + bobY};
}

void Avatar::AnimationShake(const AnimationWaveShakeParams &params) {
  double time = GetTime();
  double interval = 1.0 / (params.speed * 20.0f);
  if (time >= nextShakeTime) {
    shakeOffsetX = (GetRandomValue(-100, 100) / 100.0f) * params.ampX;
    shakeOffsetY = (GetRandomValue(-100, 100) / 100.0f) * params.ampY;
    nextShakeTime = time + interval;
  }
  position = {(GetScreenWidth() - rect.width * scale) / 2.0f + shakeOffsetX,
              (GetScreenHeight() - rect.height * scale) / 2.0f + shakeOffsetY};
}

void Avatar::StopAnimation() {
  position = {(GetScreenWidth() - rect.width * scale) / 2.0f,
              (GetScreenHeight() - rect.height * scale) / 2.0f};
}

void Avatar::TranslateX(float offset) { position.x += offset; }
void Avatar::TranslateY(float offset) { position.y += offset; }