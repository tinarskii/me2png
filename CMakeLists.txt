cmake_minimum_required(VERSION 3.16)

project(me2png)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib 3.0 REQUIRED)
find_path(PORTAUDIO_INCLUDE_DIR NAMES portaudio.h)
find_library(PORTAUDIO_LIBRARY NAMES portaudio)

add_executable(
    me2png 
        src/Main.cpp
        src/Avatar.cpp
        src/Config.cpp
        src/ImagePicker.cpp
        src/Microphone.cpp
)
target_link_libraries(me2png raylib ${PORTAUDIO_LIBRARY})
target_include_directories(me2png PRIVATE ${PORTAUDIO_INCLUDE_DIR})