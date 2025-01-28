cmake_minimum_required(VERSION 3.1.0)
project(subnettergui VERSION 0.0.1)
set(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:LIBCMT /NODEFAULTLIB:MSVCRT /IGNORE:4099")
set(CMAKE_CXX_STANDARD 23)

add_executable(subnettergui WIN32 src/main.cpp)

include_directories(include)
include_directories(include/random)
include_directories(include/glfw)
include_directories(include/gl)
include_directories(include/imgui)
include_directories(include/implot3d)
include_directories(include/subnetter)

add_library(glew STATIC IMPORTED)
set_target_properties(glew PROPERTIES IMPORTED_LOCATION "./bin/glew/glew32s.lib")

add_library(glfw3 STATIC IMPORTED)
set_target_properties(glfw3 PROPERTIES IMPORTED_LOCATION "./bin/glfw/glfw3.lib")

add_library(imgui STATIC 
    lib/imgui/imgui.cpp lib/imgui/imgui_impl_glfw.cpp lib/imgui/imgui_impl_opengl3.cpp lib/imgui/imgui_draw.cpp 
    lib/imgui/imgui_tables.cpp lib/imgui/imgui_widgets.cpp lib/imgui/imgui_demo.cpp)
add_library(implot3d STATIC lib/implot3d/implot3d.cpp lib/implot3d/implot3d_demo.cpp lib/implot3d/implot3d_items.cpp 
    lib/implot3d/implot3d_meshes.cpp)

target_compile_definitions(subnettergui PRIVATE GLEW_STATIC)
target_link_libraries(subnettergui PRIVATE opengl32 glew)
target_link_libraries(subnettergui PRIVATE imgui)
target_link_libraries(subnettergui PRIVATE glfw3)
target_link_libraries(subnettergui PRIVATE implot3d)