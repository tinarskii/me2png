cmake_minimum_required(VERSION 3.16)
project(me2png)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
  add_library(raylib STATIC IMPORTED)
  set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/raylib-win/lib/libraylib.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/raylib-win/include
  )
  add_library(portaudio STATIC IMPORTED)
  set_target_properties(portaudio PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/portaudio-win/mingw64/lib/libportaudio.a
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/portaudio-win/mingw64/include
  )
  set(PORTAUDIO_LIBRARY portaudio)
  set(PORTAUDIO_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/portaudio-win/mingw64/include)
else()
  find_package(raylib 5.5 REQUIRED)
  find_path(PORTAUDIO_INCLUDE_DIR NAMES portaudio.h)
  find_library(PORTAUDIO_LIBRARY NAMES portaudio)
endif()

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

target_compile_definitions(me2png PRIVATE 
  DATADIR="${CMAKE_INSTALL_PREFIX}/share/me2png"
)

if(WIN32)
  target_link_libraries(me2png raylib ${PORTAUDIO_LIBRARY}
    opengl32 gdi32 winmm ole32 setupapi)
else()
  target_link_libraries(me2png raylib ${PORTAUDIO_LIBRARY})
endif()

target_include_directories(me2png PRIVATE ${PORTAUDIO_INCLUDE_DIR})
install(TARGETS me2png DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/me2png/assets)
install(FILES me2png.desktop DESTINATION share/applications)
install(FILES assets/icon.png DESTINATION share/icons/hicolor/64x64/apps RENAME me2png.png)
install(FILES assets/DynaPuff.ttf DESTINATION share/me2png/assets)