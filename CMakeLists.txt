cmake_minimum_required(VERSION 3.11.0)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./build)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ./bin)
project(subnettergui VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 23)

include_directories(include include/random include/GLFW include/GL include/imgui include/implot3d include/subnetter include/ipv6tools include/memsafety)

add_library(glew STATIC IMPORTED)
add_library(glfw3 STATIC IMPORTED)
add_library(imgui STATIC 
    lib/imgui/imgui.cpp lib/imgui/imgui_impl_glfw.cpp lib/imgui/imgui_impl_opengl3.cpp lib/imgui/imgui_draw.cpp 
    lib/imgui/imgui_tables.cpp lib/imgui/imgui_widgets.cpp lib/imgui/imgui_demo.cpp)
add_library(implot3d STATIC lib/implot3d/implot3d.cpp lib/implot3d/implot3d_demo.cpp lib/implot3d/implot3d_items.cpp 
    lib/implot3d/implot3d_meshes.cpp)
add_library(memsafety STATIC lib/memsafety/memsafety.c)

IF (WIN32)
    add_executable(subnettergui WIN32 src/main.cpp)
    set_target_properties(glew PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/bin/glew/win32/glew32s.lib")
    set_target_properties(glfw3 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/bin/glfw/win32/glfw3.lib")
    target_link_libraries(subnettergui PRIVATE opengl32 glew)
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_executable(subnettergui src/main.cpp)
    set_target_properties(glew PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/bin/glew/macOS/libGLEW.a")
    set_target_properties(glfw3 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/bin/glfw/macOS/libglfw3mac.a")
    find_package(OpenGL REQUIRED)
    target_link_libraries(subnettergui PRIVATE "-framework OpenGL" "-framework CoreFoundation" "-framework Cocoa" "-framework AppKit" "-framework IOKit" OpenGL::GL glew)
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(subnettergui src/main.cpp)
    set_target_properties(glew PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/bin/glew/linux32/libGLEW.a")
    set_target_properties(glfw3 PROPERTIES IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/bin/glfw/linux32/libglfw3.a")
    target_link_libraries(subnettergui PRIVATE GL glew glfw3)
ENDIF()

IF (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "/IGNORE:4099 /IGNORE:4098")
    target_compile_options(subnettergui PRIVATE /wd4710 /wd4711) # Ignore warnings about inlining
    target_compile_options(subnettergui PRIVATE /W4 /Wall /wd4514 /wd5045 /wd4820 /wd4668) # unreferenced library functions, spectre mitigations, and other trivial warning messages disabled
ELSE()
    target_compile_options(subnettergui PRIVATE -Wall -Wextra -Wpedantic -Werror -Walloca -Wcast-qual -Wconversion -std=c++23)
ENDIF()

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
     add_definitions(-DCMAKE_DEBUG) # debug switch
ENDIF()

target_compile_definitions(subnettergui PRIVATE GLEW_STATIC)
target_link_libraries(subnettergui PRIVATE imgui)
target_link_libraries(subnettergui PRIVATE glfw3)
target_link_libraries(subnettergui PRIVATE implot3d)
target_link_libraries(subnettergui PRIVATE memsafety)