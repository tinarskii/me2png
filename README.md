# Me2PNG

A very lightweight and easy-to-use pngtuber software. No complicated setup, no overly complex
features you don't need, just download, run, select your mic and avatar, and you're good to go.

But if you need that overly complex feature,
or want to improve its UI, or increasing your GitHub karma, you sure can contribute to this project! +1 byte contributions are welcome too.

<img width="1920" height="1045" alt="image" src="./assets/Me2PNG.png" />

<a href="https://tinarskii.itch.io/me2png">
<img width="auto" height="64" alt="image" src="https://github.com/user-attachments/assets/82f6a1a9-3631-488c-ad66-84bc70e8e0a8" />
</a>

## Table of Contents

- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [Building for Windows](#building-for-windows)
- [Building for Linux](#building-for-linux)
- [Questions / Bugs](#questions--bugs)
- [Contact](#contact)
- [Credits & License](#credits--license)

## Requirements

#### System Requirements

**TL;DR** If you can see this page rendered correctly, it will run.

**More Details**:
- **OS**: Windows 7 or later, 
- **GPU**: OpenGL 3.3 compatible GPU
- **RAM**: At least 100MB of RAM
- **Storage**: At least 10 MB of free disk space
- **CPU**: Any x86_64 CPU
- **Audio Input**: A microphone or any audio input device 

## Installation

### Windows

You can download the latest release from the [itch.io page](https://tinarskii.itch.io/me2png), and extract the zip file. Then run `me2png.exe`.

### Linux / MacOS

**Fedora Linux**: You can install from the Copr repository. Run `sudo dnf copr enable tinvv/me2png`, then `sudo dnf install me2png`. See [Copr Page](https://copr.fedorainfracloud.org/coprs/tinvv/me2png/) for more details.

**Arch Linux**: You can install from the AUR. Run `yay -S me2png`, or `paru -S me2png`. See [AUR Page](https://aur.archlinux.org/packages/me2png/) for more details.

**Other Linux Distros / MacOS**: You can download the latest release from the [itch.io page](https://tinarskii.itch.io/me2png), extract the tar.gz file, and run `./me2png`.

### Universal

Coming soon! Or it won't come at all, who knows. But if you want to help with that, contributions are always welcome!

### Other Platforms

See [Building for Windows](#building-for-windows) and [Building for Linux](#building-for-linux) sections below for building instructions.

Otherwise, you can build it by yourself. See [CMakeLists.txt](https://github.com/tinarskii/me2png/blob/main/CMakeLists.txt) for the dependencies and build configuration.

## Usage

#### Config Menu
Navigate to the **top-left side** for config menu. You can change the idle/talk/blink images, microphone input, and other settings there. The config icon will automatically disappear when
you switch to other window. 

#### Integrating with Streaming Software
For integrating into your preferred streaming software, you can directly capture this window,
the window title, for example, is **"Me2PNG"** and followed by the version number.

In OBS, you can add a **Window Capture** source and select the Me2PNG window.

#### Transparent Background
Use **Chroma Key** for transparent background, should be the same color of the background of the window.
You can set it in the config menu, default is pure white.

In OBS, this can be done by right clicking the Window Capture source, then select **Filters**, then add **Chroma Key filter**.

## Building for Windows

*(This tutorial assumes you are on Fedora)*

- Download development tools: `sudo dnf install mingw64-gcc-c++ mingw64-winpthreads-static`
- Download raylib 5.5 for mingw-w64: [SourceForge Download (.zip)](https://sourceforge.net/projects/raylib.mirror/files/5.5/raylib-5.5_win64_mingw-w64.zip/download)
- Download PortAudio for mingw-w64: [Download (.pkg.tar.zst)](https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-portaudio-1~19.7.0-4-any.pkg.tar.zst)
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

- After building, copy required DLLs next to the exe:
  - `raylib-win/lib/raylib.dll`
  - `portaudio-win/mingw64/bin/libportaudio.dll`

## Building for Linux

*(This tutorial assumes you are on Fedora)*

- Download raylib and portaudio: `sudo dnf install raylib-devel portaudio-devel`, use your distro's package manager if not on Fedora
- Build: `cmake -B build && cmake --build build`

## Questions / Bugs

Contact me, scroll down a bit and you will find the contact section. 

> If you are having trouble with audio on Linux, try switching from PulseAudio to PipeWire, or vice versa.

## Contact

If you have any questions, suggestions, wanting to contribute, or just want to talk with me (please do!!) [Join the Discord Server](https://discord.gg/vkW7YMyYaf). It's kinda dead and if you don't want to join, add me instead!! **@acsp** just send me a message first so I know who you are.

If you have found a security vulnerability, please contact me directly at [tinvv@outlook.co.th](mailto:tinvv@outlook.co.th) please do not open an issue for that or else I will die.

## Credits & License

- Default Reg chibi avatar drawn by [Becbec999](https://lit.link/en/honeytea999)
- This project is licensed under the MIT License