#include "headers/Utils.hpp"
#include <raylib.h>
#include <string>

std::string AssetPath(const std::string &path) {
  std::string local = "assets/" + path;
  if (FileExists(local.c_str()))
    return local;
#ifdef _WIN32
  return local;
#else
  return std::string(DATADIR) + "/assets/" + path;
#endif
}