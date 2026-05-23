#include <raylib.h>

struct Position {
  float x;
  float y;
};

class Avatar {
public:
  float scale = 1.0f;
  Texture2D texture;
  Rectangle rect;
  Vector2 position;
  enum AnimationState { NONE, WAVE, SHAKE };

  void DrawSprite(void);
  void LoadSprite(const char *filename);
  void UnloadSprite(void);
  void AutoScale(void);
  void Animation(AnimationState state);
  void StopAnimation(void);
};