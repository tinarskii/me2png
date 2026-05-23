#include "headers/ImagePicker.hpp"
#include "headers/raygui.h"
#include <cstdio>

namespace {
bool IsAbsolutePath(const char *path) {
  return path != nullptr && path[0] == '/';
}

std::string JoinPath(const std::string &base, const char *suffix) {
  if (suffix == nullptr || suffix[0] == '\0') {
    return base;
  }
  if (base.empty()) {
    return std::string(suffix);
  }
  if (base.back() == '/') {
    return base + suffix;
  }
  return base + "/" + suffix;
}

std::string ToAbsolutePath(const char *path, const std::string &base) {
  if (path == nullptr || path[0] == '\0') {
    return base;
  }
  if (IsAbsolutePath(path)) {
    return std::string(path);
  }
  return JoinPath(base, path);
}
} // namespace

ImagePicker::ImagePicker() {
  std::snprintf(paths_.idle, kPathMax, "%s", "./assets/idle.png");
  std::snprintf(paths_.talk, kPathMax, "%s", "./assets/talk.png");
  std::snprintf(paths_.blinkIdle, kPathMax, "%s", "./assets/blink-idle.png");
  std::snprintf(paths_.blinkTalk, kPathMax, "%s", "./assets/blink-talk.png");
  workingDir_ = GetWorkingDirectory();
  dialog_.currentDir = workingDir_;
}

ImagePaths &ImagePicker::Paths() { return paths_; }

const ImagePaths &ImagePicker::Paths() const { return paths_; }

void ImagePicker::SetPaths(const ImagePaths &paths) {
  std::snprintf(paths_.idle, kPathMax, "%s", paths.idle);
  std::snprintf(paths_.talk, kPathMax, "%s", paths.talk);
  std::snprintf(paths_.blinkIdle, kPathMax, "%s", paths.blinkIdle);
  std::snprintf(paths_.blinkTalk, kPathMax, "%s", paths.blinkTalk);
}

void ImagePicker::SetWorkingDirectory(const char *dir) {
  std::string candidate = ToAbsolutePath(dir, GetWorkingDirectory());
  if (!candidate.empty() && DirectoryExists(candidate.c_str())) {
    workingDir_ = candidate;
  } else {
    workingDir_ = GetWorkingDirectory();
  }
  if (!dialog_.open) {
    dialog_.currentDir = workingDir_;
  }
}

void ImagePicker::DrawRow(const Rectangle &labelRect,
                          const Rectangle &buttonRect,
                          const Rectangle &valueRect, const char *label,
                          SpriteMode mode) {
  GuiLabel(labelRect, label);
  if (GuiButton(buttonRect, GuiIconText(ICON_FILE_OPEN, ""))) {
    Open(mode);
  }

  const char *path = PathForMode(mode);
  if (path != nullptr) {
    GuiLabel(valueRect, FileExists(path) ? GetFileName(path) : "Not set");
  }
}

bool ImagePicker::DrawDialog() {
  if (!dialog_.open) {
    return false;
  }

  bool updated = false;
  const float dialogWidth = 560.0f;
  const float dialogHeight = 380.0f;
  Rectangle dialogBounds = {(GetScreenWidth() - dialogWidth) / 2.0f,
                            (GetScreenHeight() - dialogHeight) / 2.0f,
                            dialogWidth, dialogHeight};
  Rectangle pathLabelRect = {dialogBounds.x + 10.0f, dialogBounds.y + 10.0f,
                             dialogBounds.width - 20.0f, 20.0f};
  Rectangle upButtonRect = {dialogBounds.x + 10.0f, dialogBounds.y + 34.0f,
                            24.0f, 20.0f};
  Rectangle listBounds = {dialogBounds.x + 10.0f, dialogBounds.y + 60.0f,
                          dialogBounds.width - 20.0f,
                          dialogBounds.height - 100.0f};
  Rectangle selectButtonRect = {dialogBounds.x + dialogBounds.width - 180.0f,
                                dialogBounds.y + dialogBounds.height - 30.0f,
                                80.0f, 20.0f};
  Rectangle cancelButtonRect = {dialogBounds.x + dialogBounds.width - 90.0f,
                                dialogBounds.y + dialogBounds.height - 30.0f,
                                80.0f, 20.0f};

  if (GuiWindowBox(dialogBounds, "Select Image")) {
    dialog_.open = false;
  }

  GuiLabel(pathLabelRect, dialog_.currentDir.c_str());
  if (GuiButton(upButtonRect, GuiIconText(ICON_ARROW_UP, ""))) {
    const char *prevDir = GetPrevDirectoryPath(dialog_.currentDir.c_str());
    if (prevDir != nullptr && DirectoryExists(prevDir)) {
      dialog_.currentDir = prevDir;
      RefreshDialog();
    }
  }

  int prevActive = dialog_.active;
  GuiListView(listBounds, dialog_.listText.c_str(), &dialog_.scrollIndex,
              &dialog_.active);
  if (dialog_.active != prevActive && dialog_.active >= 0 &&
      dialog_.active < static_cast<int>(dialog_.paths.size())) {
    const std::string &selectedPath = dialog_.paths[dialog_.active];
    if (!IsPathFile(selectedPath.c_str())) {
      std::string nextDir =
          ToAbsolutePath(selectedPath.c_str(), dialog_.currentDir);
      if (DirectoryExists(nextDir.c_str())) {
        dialog_.currentDir = nextDir;
      } else {
        dialog_.currentDir = selectedPath;
      }
      RefreshDialog();
    }
  }

  bool selectPressed = GuiButton(selectButtonRect, "Select");
  if (GuiButton(cancelButtonRect, "Cancel")) {
    dialog_.open = false;
  }

  if (selectPressed && dialog_.active >= 0 &&
      dialog_.active < static_cast<int>(dialog_.paths.size())) {
    const std::string &selectedPath = dialog_.paths[dialog_.active];
    if (!IsPathFile(selectedPath.c_str())) {
      std::string nextDir =
          ToAbsolutePath(selectedPath.c_str(), dialog_.currentDir);
      if (DirectoryExists(nextDir.c_str())) {
        dialog_.currentDir = nextDir;
      } else {
        dialog_.currentDir = selectedPath;
      }
      RefreshDialog();
    } else {
      char *targetPath = MutablePathForMode(dialog_.target);
      if (targetPath != nullptr) {
        std::string absolutePath =
            ToAbsolutePath(selectedPath.c_str(), dialog_.currentDir);
        std::snprintf(targetPath, kPathMax, "%s", absolutePath.c_str());
        updated = true;
      }
      dialog_.open = false;
    }
  }

  return updated;
}

void ImagePicker::CloseDialog() { dialog_.open = false; }

void ImagePicker::Cleanup() {
  if (dialog_.files.count > 0) {
    UnloadDirectoryFiles(dialog_.files);
    dialog_.files = {};
  }
}

const char *ImagePicker::PathForMode(SpriteMode mode) const {
  switch (mode) {
  case SpriteMode::Idle:
    return paths_.idle;
  case SpriteMode::Talk:
    return paths_.talk;
  case SpriteMode::BlinkIdle:
    return paths_.blinkIdle;
  case SpriteMode::BlinkTalk:
    return paths_.blinkTalk;
  case SpriteMode::None:
    break;
  }
  return nullptr;
}

char *ImagePicker::MutablePathForMode(SpriteMode mode) {
  return const_cast<char *>(PathForMode(mode));
}

void ImagePicker::Open(SpriteMode target) {
  dialog_.open = true;
  dialog_.target = target;
  const char *path = PathForMode(target);
  std::string absolutePath = ToAbsolutePath(path, workingDir_);
  const char *startDir =
      absolutePath.empty() ? nullptr : GetDirectoryPath(absolutePath.c_str());
  if (startDir != nullptr && DirectoryExists(startDir)) {
    dialog_.currentDir = startDir;
  } else {
    dialog_.currentDir = workingDir_;
  }
  RefreshDialog();
}

void ImagePicker::RefreshDialog() {
  if (dialog_.files.count > 0) {
    UnloadDirectoryFiles(dialog_.files);
    dialog_.files = {};
  }

  dialog_.labels.clear();
  dialog_.paths.clear();
  dialog_.listText.clear();

  dialog_.files = LoadDirectoryFilesEx(dialog_.currentDir.c_str(),
                                       ".png;.PNG;DIR", false);

  for (unsigned int i = 0; i < dialog_.files.count; ++i) {
    const char *path = dialog_.files.paths[i];
    if (path == nullptr) {
      continue;
    }
    const char *name = GetFileName(path);
    bool isFile = IsPathFile(path);
    std::string label = isFile ? GuiIconText(ICON_FILETYPE_IMAGE, name)
                               : GuiIconText(ICON_FOLDER_OPEN, name);
    dialog_.labels.push_back(label);
    dialog_.paths.push_back(path);
  }

  for (size_t i = 0; i < dialog_.labels.size(); ++i) {
    if (i > 0) {
      dialog_.listText += ";";
    }
    dialog_.listText += dialog_.labels[i];
  }

  dialog_.scrollIndex = 0;
  dialog_.active = -1;
}
