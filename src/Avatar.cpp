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

void Avatar::Animation(AnimationState state) {
  float time = GetTime();
  float bob = sinf(time * 2.0f) * 8.0f;
  // NON UNIFORM SHAKING
  float shakeX = sinf(time * 30.0f) * 4.0f;
  float shakeY = cosf(time * 40.0f) * 3.0f;

  switch (state) {
  case NONE:
    break;
  case SHAKE:
    position = {(GetScreenWidth() - rect.width * scale) / 2.0f + shakeX,
                (GetScreenHeight() - rect.height * scale) / 2.0f + shakeY};
    break;
  case WAVE:
    position = {(GetScreenWidth() - rect.width * scale) / 2.0f,
                (GetScreenHeight() - rect.height * scale) / 2.0f + bob};
    break;
  }
}

void Avatar::StopAnimation() {
  position = {(GetScreenWidth() - rect.width * scale) / 2.0f,
              (GetScreenHeight() - rect.height * scale) / 2.0f};
}