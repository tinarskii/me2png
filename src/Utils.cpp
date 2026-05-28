#include "headers/Utils.hpp"
#include <string>
#include <raylib.h>

std::string AssetPath(const std::string &path) {
#ifdef _WIN32
  return "assets/" + path;
#else
  std::string installed = std::string(DATADIR) + "/assets/" + path;
  if (FileExists(installed.c_str())) return installed;
  return "assets/" + path;
#endif
}