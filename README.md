# Subnetter GUI Project

## Overview

SubnetterGUI is a graphical interface for Subnetting and VLSM calculations written in C++. This tool has a variety of tools for learning how to subnet and calculating subnets.

## Dependencies

This program uses the `ImGui` library and a 3D plotting extension for ImGui called `ImPlot3D`. To use the ImGui library, `GLFW` and `GLEW` are linked and initialized. ImGui is designed to run on top of a window platform, so it does not have any unique dependencies. GLFW and GLEW, however, will require various packages depending on your operating system. On Ubuntu-based Linux distributions, for example, the following packages are always required:

```
libwayland-dev libxkbcommon-dev
```
Note: `xorg-dev` is also required when using the X11 window system.

## Compatibility

SubnetterGUI is designed for Windows x86-64, macOS arm64, and Linux x86-64. GLFW and GLEW are statically linked into this program with pre-compiled libraries. Due to limitations in GLFW's and GLEW's build process, other operating systems/architectures are not currently supported, and their library files are not included.

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

## How To Use The Program

### Subnet Calculator

This program's primary function is to calculate subnets. This can be done from the main menu.

Enter an IP address and at least one subnet mask[^1] into the `IP`, `Netmask 1`, and `Netmask 2` input text fields respectively.
[^1]: If one subnet mask is entered, a single subnet will be calculated. If two subnet masks are entered, VLSM will be calculated with the two subnet masks.
