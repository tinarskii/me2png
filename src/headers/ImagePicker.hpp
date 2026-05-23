#pragma once

#include <raylib.h>
#include <string>
#include <vector>

constexpr int kPathMax = 256;

enum class SpriteMode { None, Idle, Talk, BlinkIdle, BlinkTalk };

struct ImagePaths {
  char idle[kPathMax];
  char talk[kPathMax];
  char blinkIdle[kPathMax];
  char blinkTalk[kPathMax];
};

class ImagePicker {
public:
  ImagePicker();

  ImagePaths &Paths();
  const ImagePaths &Paths() const;
  void SetPaths(const ImagePaths &paths);
  void SetWorkingDirectory(const char *dir);
  void DrawRow(const Rectangle &labelRect, const Rectangle &buttonRect,
               const Rectangle &valueRect, const char *label,
               SpriteMode mode);
  bool DrawDialog();
  void CloseDialog();
  void Cleanup();

private:
  struct FileDialogState {
    bool open = false;
    SpriteMode target = SpriteMode::None;
    std::string currentDir;
    FilePathList files = {};
    std::vector<std::string> labels;
    std::vector<std::string> paths;
    std::string listText;
    int scrollIndex = 0;
    int active = -1;
  };

  const char *PathForMode(SpriteMode mode) const;
  char *MutablePathForMode(SpriteMode mode);
  void Open(SpriteMode target);
  void RefreshDialog();

  ImagePaths paths_{};
  FileDialogState dialog_{};
  std::string workingDir_;
};
