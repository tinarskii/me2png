# Me2png

A cross-platform, lightweight, and simple program for PNGtubing. Written in C++ using raylib and PortAudio.

<img width="1920" height="1045" alt="image" src="https://github.com/user-attachments/assets/a0b70d1a-7962-43ee-a8e4-be2c0974b7c3" />

## Installation

- Windows: [Release Page](https://github.com/tinarskii/me2png/releases)
- Fedora Linux: `sudo dnf copr enable tinvv/me2png` then `sudo dnf install me2png`
- MacOS / Other Linux Distros: [Release Page](https://github.com/tinarskii/me2png/releases)

## Usage

Hover to the top-left side for config menu. You can change the idle/talk/blink images, microphone input, and other settings there. 

For integrating into your preferred streaming software, you can directly capture this window,
the window title, for example, is "Me2PNG" and followed by the version number.

In OBS, you can add a Window Capture source and select the Me2PNG window.

Use Chroma Key for transparent background, should be the same color of the background of the window.
You can set it in the config menu, default is pure white.

In OBS, this can be done by right clicking the Window Capture source, then select Filters, then add Chroma Key filter.

## Building for Windows

- Download development tools: `sudo dnf install mingw64-gcc-c++ mingw64-winpthreads-static`
- Download raylib 6.0 for mingw-w64: [SourceForge Download (.zip)](https://sourceforge.net/projects/raylib.mirror/files/6.0/raylib-6.0_win64_mingw-w64.zip/download)
- Download PortAudio for mingw-w64: [Download (.tar.zst)](https://mirror.msys2.org/mingw/sources/mingw-w64-portaudio-1~19.7.0-4.src.tar.zst)
- In project, unzip raylib to raylib-win/
- In project, extract portaudio to portaudio-win

It should look like this:

```sh
$ ls raylib-win/
CHANGELOG  include/  lib/  LICENSE  README.md
```

```sh
$ ls portaudio-win/
mingw64/
```

Then build

```
cmake -DCMAKE_TOOLCHAIN_FILE=mingw64.cmake \
               -DCMAKE_PREFIX_PATH="$(pwd)/portaudio-win/mingw64" \
               -B build-win
cmake --build build-win
```

## Credits

Default Reg avatar drawn by [Becbec999](https://lit.link/en/honeytea999)
