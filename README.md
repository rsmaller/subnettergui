# Subnetter GUI Project

## Overview

SubnetterGUI is a graphical interface for Subnetting and VLSM calculations written in C++.

## Dependencies

This program uses the `ImGui` library and a 3D plotting extension for ImGui called `ImPlot3D`. To use the ImGui library, `GLFW` and `GLEW` are linked and initialized. ImGui is designed to run on top of a window platform, so it does not have any unique dependencies. GLFW and GLEW, however, will require various packages depending on your operating system. On Ubuntu-based Linux distributions, for example, the following packages are always required:

```
libwayland-dev libxkbcommon-dev
```
Note: `xorg-dev` is also required when using the X11 window system.

## Compatibility

SubnetterGUI is designed for Windows x86-64, macOS arm64, and Linux x86-64. GLFW and GLEW are statically linked into this program with pre-compiled libraries. Other operating systems/architectures are not currently supported and their binaries are not included.

## Compiling

Before compiling, don't forget to initialize the CMake cache:

```
cmake .
```

This program can then be compiled using CMake's build option:

```
cmake --build .
```

The binary will be dropped in the `build` folder under the name `subnettergui`.