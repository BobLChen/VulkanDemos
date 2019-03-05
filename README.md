## 简要
Vulkan的一些案例，建议初学者从我第一次Commit开始跟，一般情况下每次Commit应该都是有效Commit，不排除偷懒的情况下随便瞎提交。

## 环境要求
### Windows
- CMake 3.13.0：下载安装最新版本即可。
- Visual Studio 2017 (64位)：比它高的应该也没问题。

### MacOS
- CMake 3.13.0：下载安装最新版本即可。
- XCode 10：比它高应该也没什么问题。
- macOS 10.11 or iOS 9：因为Vulkan在苹果那边没有得到官方支持，是通过对Metal进行的封装，因此需要10.11系统以上。

### Linux
- CMake 3.13.0：下载安装最新版本即可。
- Ubuntu 18.04：目前我使用的是Ubuntu 18.04系统，其它版本的没有尝试，用到了GLFW库，因此需要安装GLFW的一些依赖。详情参考GLFW:https://www.glfw.org/docs/latest/vulkan_guide.html
- VSCode：Ubuntu下我使用了VSCode作为开发环境，VSCode下Configure(Task)，Build(Task)，Debug我都配置好了，但是需要安装VSCode C++插件，插件名称：C/C++。

## 配置
我只说Windows的搭建，因为MacOS搭建过程跟Windows一样，Ubuntu装好VSCode就已经算搭建好了。

**第一步**
随便找个地方，右键Git Bash(没有安装Git的去安装Git，没有Git你跑这儿来干嘛)

![Step0](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/examples/assets/0.png)

**第二步**
在那个黑框中输入```git clone --recursive https://github.com/BobLChen/VulkanTutorials.git```
![Step1](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/examples/assets/2.png)

**第三步**
下载好了代码工程之后，打开CMake，填入工程地址。
![Step2](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/examples/assets/3.png)

**第四步**
填好之后，点击```Configure```按钮，然后在弹出框选择VS2017 Win64，其它默认，点击Finish即可。
![Step3](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/examples/assets/4.png)
![Step4](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/examples/assets/5.png)

**第五步**
最后点击```Generate```生成VS2017工程。生成好之后，点击```Open Project```，打开之后选中任意examples目录下的项目```右键设置为启动项```即可。
![Step5](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/examples/assets/6.png)

## Introduction
Vulkan Examples 

## Requirements
### Windows
- CMake 3.13.0
- virtual studio 2017

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
![Triangle](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/examples/2_Triangle/triangle.jpg)

### OBJ
![OBJLoader](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/examples/3_OBJLoader/obj.png)

### Pipelines
![Pipelines](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/examples/4_Pipelines/pipelines.png)
