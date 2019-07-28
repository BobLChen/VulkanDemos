## 简要

Vulkan Samples

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

### [Triangle](https://github.com/BobLChen/VulkanDemos/tree/master/examples/2_Triangle)

最基础的Vulkan案例，显示一个自动旋转的三角形。

![2_Triangle](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/2_Triangle.jpg)

### [DemoBase](https://github.com/BobLChen/VulkanDemos/tree/master/examples/3_DemoBase)

基于上一个Demo改造而来，对一些常用的属性和功能进行了简单封装。减少后续Demo的代码量，代码量越多，功能越不清晰，理解越困难。

### [OptimizeBuffer](https://github.com/BobLChen/VulkanDemos/tree/master/examples/4_OptimizeBuffer)

基于上一个Demo，对Buffer进行了简单封装，只需要提供Buffer类型，长度或者数据即可创建。还提供了一些常用的Map、Copy等接口，方便后续Demo使用。

### [OptimizeCommandBuffer](https://github.com/BobLChen/VulkanDemos/tree/master/examples/5_OptimizeCommandBuffer)

基于上一个Demo，对CommandBuffer进行了简单封装，更简洁的方式进行Begin、End、Submit等操作。减少不必要的代码编写同时也方便后续的Demo使用。

### [ImageGUI](https://github.com/BobLChen/VulkanDemos/tree/master/examples/6_ImageGUI)

为了方便调试，功能切换等，需要集成简单的UI组件。目前使用的是ImageGUI，只需一个头文件和CPP文件即可方便的集成进来。集成代码可以参考ImageGUI关于Vulkan的实现。

![6_ImageGUI](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/6_ImageGUI.jpg)

### [UniformBuffer](https://github.com/BobLChen/VulkanDemos/tree/master/examples/7_UniformBuffer)

完成了ImageGUI集成之后，我们就可以开始尝试给Shader中传入参数以实现特定的功能。下面的Demo展示的是如何在Shader中通过函数来近视的模拟拟合[Pre-Integrated Skin Shading](https://zhuanlan.zhihu.com/p/56052015)中的预积分贴图。虽然数学上不正常，但是可以得到一个非常近似的结果。通过近似的拟合可以减少一张贴图的使用，但是增加了Shader中的计算量。

![7_UniformBuffer](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/7_UniformBuffer.jpg)

### [Load Mesh](https://github.com/BobLChen/VulkanDemos/tree/master/examples/9_LoadMesh)

之前的Demo都是手动填充的模型数据（手动填充顶点数据以及索引数据）。对于非常简单的需求或者功能来讲，这种方式是最快捷的。但是这样并不能满足复杂的需求，在这个Demo中就演示了如何加载模型文件并显示出来。

![9_LoadMesh](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/9_LoadMesh.jpg)

### [Pipelines](https://github.com/BobLChen/VulkanDemos/tree/master/examples/10_Pipelines)

在上一个Demo中演示了如何加载一个模型，但是我们会发现在创建Pipeline的时候，整个过程会变得异常繁琐。如果我们有需要多个Pipeline的需求时，就会进行重复性的编码工作，重复的工作一般都会通过复制粘贴来完成，但是这样也增加错误的几率。在前期出现错误时，由于不熟悉或者经验不足，会导致我们排错异常困难。在该Demo中，我们对Pipeline进行了简单的封装，其中需要注意的是Pipeline需要顶点数据的结构，这个数据结构由上一个Demo中提供。

![10_Pipelines](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/10_Pipelines.jpg)

### [Texture](https://github.com/BobLChen/VulkanDemos/tree/master/examples/11_Texture)

此Demo展示了如何加载2D贴图并通过UV与模型绑定。同时通过UniformBuffer传递参数，在Shader中简单实现皮肤的效果。

![11_Texture](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/11_Texture.jpg)

### [PushConstants](https://github.com/BobLChen/VulkanDemos/tree/master/examples/12_PushConstants)

往Shader中传递参数不止UniformBuffer一种形式，还有另外的一种形式：**PushConstants**。在该Demo中展示了如果通过**PushConstants**来对每个Mesh进行赋予**MVP**矩阵数据。想象一下，在场景中为了尽可能的复用模型，一般美术会单独制作每个Mesh，这些Mesh都是通过摆放组合成一个大场景。结合之前的Demo，我们就会发现必须为每个Mesh创建一个UniformBuffer才能完成对每个Mesh赋予不同的**MVP**数据。但是使用**PushConstants**可以避免这个操作。

![12_PushConstants](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/12_PushConstants.jpg)

### [DynamicUniformBuffer](https://github.com/BobLChen/VulkanDemos/tree/master/examples/13_DynamicUniformBuffer)

上一个Demo中演示了**PushConstants**的便利性，但是**PushConstants**有一些限制，实际中使用中推荐使用**DynamicUniformBuffer**，它的性质与**UniformBuffer**没有任何不同，只是在录制Draw命令时，可以指定使用哪一段的数据，从而实现使用一个Buffer但是可以为每个Mesh传递不同的数据。

![13_DynamicUniformBuffer](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/13_DynamicUniformBuffer.jpg)

### [TextureArray](https://github.com/BobLChen/VulkanDemos/tree/master/examples/14_TextureArray)

之前的Demo中展示了如何创建Texture2D，在该Demo我们使用之前的技术，但是创建的是Texture2DArray。Texture2DArray可以让我们在Shader中使用一组Texture。这个功能可以很好的利用到地形上。

![14_TextureArray](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/14_TextureArray.jpg)

### [Texture3D ](https://github.com/BobLChen/VulkanDemos/tree/master/examples/15_Texture3D)

上一个Demo展示了如何创建Texture2DArray，在这个Demo中我们更进一步，展示如何创建Texture3D。Texture3D一般使用的比较少，多出现在一些非常高级的功能中。在这个Demo中，展示了如何通过Texture3D来 实现3D ColorGrading的特效，通俗讲就是色调映射技术。下面第一张图是原图，第二张是通过Texture3D映射过的复古风图，第三张是映射时对应的Texture3D信息，第四张是Texture3D对应的色调映射信息。

![15_Texture3D](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/15_Texture3D.jpg)

### [OptimizeShaderAndLayout](https://github.com/BobLChen/VulkanDemos/tree/master/examples/16_OptimizeShaderAndLayout)

在之前的Demo中，我们可以发现大量的代码花费在**DescriptorSetLayout**以及**DescriptorSet**上。为了方便后续的操作，我们需要提炼功能出它们。在此我引入了一个非常简单化的Shader进来，Shader中含有所有的**DescriptorLayout**信息。就看我们如何提取出来。在该Demo中，我使用了一个SPIV的反编译工具。通过反编译二进制Shader文件，提取出**Layout**信息。这样我们即可在加载完Shader文件之后，即可知道Shader对应的**Layout**信息。这种方式其实不是最佳的方式，Shader文件我们一般是在**Editor**中编辑之后生成的，其实我们可以在这一步把Shader的信息抽取出来，然后跟编译之后的Shader二进制文件存放到一起，这样我们加载之后即可使用。

### [InputAttachments](https://github.com/BobLChen/VulkanDemos/tree/master/examples/17_InputAttachments)

在该Demo中，展示了如何增加附件，既如何在Shader中输出多组信息出来。在一些比较高级的特效中，不仅需要颜色信息，可能还会需要深度、法线或者其它自定义的一些信息。

![17_InputAttachments](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/17_InputAttachments.jpg)

### [SimpleDeferredShading](https://github.com/BobLChen/VulkanDemos/tree/master/examples/18_DeferredShading)

基于上一个Demo，既然我们可以输出多组信息，那么我们可以结合**SubPass**来实现一个非常简单的延迟着色。**SubPass**局限性非常大，它只能读取对应像素点的信息，无法读取附近的信息。也意味不能再**SubPass**中修改视图的大小，不能采用附近信息。但是它有一个好处，就是速度非常快，特别是在手机设备上，手机设备一般基于**Tile**的方式进行渲染输出。那么如果使用**SubPass**，**SubPass**中的采样也是基于**Tile**的，而不是像传统的那种方式，必须等到所有的**Tile**都完成才能继续。在这个Demo中输出三个信息:**Albedo**、**Normal**、**Position**。它们的格式如下：

- Albedo:VK_FORMAT_R8G8B8A8_UNORM
- Normal:VK_FORMAT_R16G16B16A16_SFLOAT
- Position:VK_FORMAT_R16G16B16A16_SFLOAT

![18_DeferredShading](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/18_DeferredShading.jpg)

### [OptimizeDeferredShading](https://github.com/BobLChen/VulkanDemos/tree/master/examples/19_OptimizeDeferredShading)

此Demo是基于上一个Demo进行优化的版本。在上一个Demo中我们输出**16bit x 4**的法线数据以及坐标数据。之所使用16bit而不是8bit，是因为法线含有负值，坐标数据范围超过255等原因。但是这并不意味我们不能进行优化，在该Demo中我们将法线数据简单编码存储到R8G8B8A8格式贴图中。顶点数据通过深度信息以及射线重建出来。这样极大的减小了每次绘制的带宽。

- Albedo: VK_FORMAT_R8G8B8A8_UNORM
- Normal: VK_FORMAT_R8G8B8A8_UNORM
- Position: Reconstructing world space position from depth buffer

![19_OptimizeDeferredShading](https://raw.githubusercontent.com/BobLChen/VulkanTutorials/master/preview/19_OptimizeDeferredShading.jpg)

### [SimpleMaterial](https://github.com/BobLChen/VulkanDemos/tree/master/examples/20_Material)

在之前的所有Demo里面，我们都在频繁的创建UniformBuffer以及管理UniformBuffer的生命周期。比如在上一个简单延迟着色的Demo里面，我们就会发现Shader参数的设置愈来愈变得非常繁琐。为了简化这些工作量，在这个Demo里面，简单设计了一个Material。针对UniformBuffer统一使用DynamicUniformBuffer，在Material内部维护了一个全局的RingUniformBuffer用来存储Uniform数据。在以后的Demo中，我们不在自己创建管理维护UniformBuffer数据。
