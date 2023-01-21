#include "hzpch.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
//注意是包含cpp文件
#include "examples/imgui_impl_opengl3.cpp"
#include "examples/imgui_impl_glfw.cpp"

// 这个cpp 会被编译成obj文件，到时候链接一起使得在别处可以使用imgui