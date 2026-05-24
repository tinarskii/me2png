#include <raylib.h>
#include <raymath.h>

struct Position {
  float x;
  float y;
};

struct AnimationWaveShakeParams {
  float speed;
  float ampX;
  float ampY;
};

class Avatar {
public:
  float scale = 1.0f;
  Texture2D texture;
  Rectangle rect;
  Vector2 position;
  enum AnimationState { NONE, WAVE, SHAKE };
  double nextShakeTime = 0.0;
  float shakeOffsetX = 0.0f;
  float shakeOffsetY = 0.0f;

  void DrawSprite(void);
  void LoadSprite(const char *filename);
  void UnloadSprite(void);
  void AutoScale(void);
  void AnimateWave(const AnimationWaveShakeParams &params);
  void AnimationShake(const AnimationWaveShakeParams &params);
  void StopAnimation(void);
};