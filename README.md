# Subnetter++

## Overview

Subnetter++ is a graphical interface for Subnetting and VLSM calculations written in C++. This tool has a variety of tools for learning how to subnet and calculating subnets.

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

Subnetter++ can be opened just like any other standalone executable program.

### Subnet Calculator

Subnetter++'s main function is to calculate subnets. This can be done from the main menu.

Enter an IP address and at least one subnet mask[^1] into the `IP`, `Netmask 1`, and `Netmask 2` input text fields respectively. The output will be shown in a scrolling text field below.

Below, four subnets under the address pool `44.117.240.0/21` are calculated:

![myimage](content/calculator-output.png)

Because VLSM can become quite large and unruly, only 256 subnets will be shown on-screen at a time. 

The `Go to Start`, `Previous 256 Subnets`, `Next 256 Subnets`, and `Go to End` buttons will change which block of 256 subnets are on-screen.

#### Calculator Flags
There are many different flags than can be toggled when calculating a subnet.

The `Binary` flag will show all IP addresses in binary format. 

The `Reverse` flag shows all subnets in reverse order.

The `Extra Info` flag will show some debug information such as what IP address is currently being stored in memory as a cursor for subnet calculations.

The `3D Plotting` flag will open a new tab with a visual representation of the subnet blocks associated with the entered subnet masks.

### Exporting Subnet Calculator Output

Subnetter++ also supports exporting subnets calculated in the main window. To do this, go to `File > Export` in the menu bar. Doing so will open a separate tab for exports. Then, enter the absolute path of a text file which will be created with the subnet data in it.

### Practicing Subnetting

You can also practice calculating subnets on your own. Go to `File > Study` to get started.

The study tab asks for various pieces of information about any single subnet. It provides a CIDR mask and an IP address.

The tab has eight question boxes to be filled:

- The IP-formatted Subnet Mask
- The block size of the subnet
- The network ID of the subnet
- The first usable IP address of the subnet
- The last usable IP address of the subnet
- The broadcast IP address of the subnet
- The binary representation of the IP address provided in the question
- The binary representation of the CIDR mask provided in the question

The `Check Answers` button will check if each entered value is correct and show the results below. 

The `Show Answers` button will fill the text boxes with the correct answers.

The `Hide Answers` button will clear the text boxes.

The `Generate New Question` button will create a new question with a randomly generated IP address and CIDR mask.

[^1]: If one subnet mask is entered, a single subnet will be calculated. If two subnet masks are entered, VLSM will be calculated with the two subnet masks.