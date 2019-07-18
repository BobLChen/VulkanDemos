## 简要
Vulkan的一些案例，建议初学者从我第一次Commit开始跟，一般情况下每次Commit应该都是有效Commit，不排除偷懒的情况下随便瞎提交。

## 环境要求
### Windows
- CMake 3.13.0：下载安装最新版本即可。
- Visual Studio 2017 (64位)：比它高的应该也没问题。
- VulkanSDK：https://www.lunarg.com/vulkan-sdk/

### MacOS
- CMake 3.13.0：下载安装最新版本即可。
- XCode 10：比它高应该也没什么问题。
- macOS 10.11 or iOS 9：因为Vulkan在苹果那边没有得到官方支持，是通过对Metal进行的封装，因此需要10.11系统以上。

### Linux
- CMake 3.13.0：下载安装最新版本即可。
- Ubuntu 18.04：目前我使用的是Ubuntu 18.04系统，其它版本的没有尝试，用到了GLFW库，因此需要安装GLFW的一些依赖。详情参考GLFW:https://www.glfw.org/docs/latest/vulkan_guide.html
- VSCode：Ubuntu下我使用了VSCode作为开发环境，VSCode下Configure(Task)，Build(Task)，Debug我都配置好了，但是需要安装VSCode C++插件，插件名称：C/C++。

## Window环境搭建
- 参考文档:https://github.com/BobLChen/VulkanTutorials/blob/master/document/BUILD_Windows.md

## Ubuntu环境搭建
- 参考文档:https://github.com/BobLChen/VulkanTutorials/blob/master/document/BUILD_Ubuntu.md

## MacOS环境搭建
- 参考文档:https://github.com/BobLChen/VulkanTutorials/blob/master/document/BUILD_MacOS.md
## Introduction
Vulkan Examples 

## Requirements
### Windows
- CMake 3.13.0
- virtual studio 2017
- VulkanSDK：https://www.lunarg.com/vulkan-sdk/

### MacOS
- XCode 10
- CMake 3.13.0
- macOS 10.11 or iOS 9

### Linux
- CMake 3.13.0

## Usage
### Command line
- git clone --recursive https://github.com/BobLChen/VulkanTutorials.git
- cd VulkanTutorials
- mkdir build
- cd build
- cmake ..

### CMake-GUI
- git clone --recursive https://github.com/BobLChen/VulkanTutorials.git
- Open CMake-GUI
- Where is the source code : VulkanTutorials
- Where to build the binaries : VulkanTutorials/build
- Click Configure button
- Choose your generator
- Click Generate button

## Example

### Triangle
![Triangle](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/2_Triangle.jpg)

### ImageGUI
![ImageGUI](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/6_ImageGUI.jpg)

### UniformBuffer
![UniformBuffer](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/7_UniformBuffer.jpg)

### Load Mesh
![LoadMesh](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/9_LoadMesh.jpg)

### Pipelines
![Pipelines](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/10_Pipelines.jpg)

### Texture
![Texture](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/11_Texture.jpg)

### PushConstants
![PushConstants](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/12_PushConstants.jpg)
