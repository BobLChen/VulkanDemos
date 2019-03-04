#pragma once

// 保证GLFW能够支持Vulkan，头文件不再导入glfw3.h，改为导入GLFWWrapper.h
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
