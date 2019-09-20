## 简要

Vulkan Samples

## 码云
[码云地址](https://gitee.com/BobLChen/VulkanDemos)

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
- Ubuntu 18.04：目前我使用的是Ubuntu 18.04系统，其它版本的没有尝试。
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

- git clone https://github.com/BobLChen/VulkanDemos.git
- cd VulkanDemos
- mkdir build
- cd build
- cmake ..

### CMake-GUI

- git clone https://github.com/BobLChen/VulkanDemos.git
- Open CMake-GUI
- Where is the source code : VulkanDemos
- Where to build the binaries : VulkanDemos/build
- Click Configure button
- Choose your generator
- Click Generate button

## Example

### [2_Triangle](https://github.com/BobLChen/VulkanDemos/tree/master/examples/2_Triangle)
[博客地址](http://xiaopengyou.fun/public/2019/07/28/2_Triangle/)
![2_Triangle](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/2_Triangle.jpg)

### [3_DemoBase](https://github.com/BobLChen/VulkanDemos/tree/master/examples/3_DemoBase)
[博客地址](http://xiaopengyou.fun/public/2019/08/01/3_DemoBase/)

### [4_OptimizeBuffer](https://github.com/BobLChen/VulkanDemos/tree/master/examples/4_OptimizeBuffer)
[博客地址](http://xiaopengyou.fun/public/2019/08/01/4_OptimizeBuffer/)

### [5_OptimizeCommandBuffer](https://github.com/BobLChen/VulkanDemos/tree/master/examples/5_OptimizeCommandBuffer)
[博客地址](http://xiaopengyou.fun/public/2019/08/01/5_OptimizeCommandBuffer/)

### [6_ImageGUI](https://github.com/BobLChen/VulkanDemos/tree/master/examples/6_ImageGUI)
[博客地址](http://xiaopengyou.fun/public/2019/08/01/6_ImageGUI/)
![6_ImageGUI](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/6_ImageGUI.jpg)

### [7_UniformBuffer](https://github.com/BobLChen/VulkanDemos/tree/master/examples/7_UniformBuffer)
[博客地址](http://xiaopengyou.fun/public/2019/08/02/7_UniformBuffer/)
![7_UniformBuffer](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/7_UniformBuffer.jpg)

### [8_OptimizeVertexIndexBuffer](https://github.com/BobLChen/VulkanDemos/tree/master/examples/8_OptimizeVertexIndexBuffer)
[博客地址](http://xiaopengyou.fun/public/2019/08/02/8_OptimizeVertexIndexBuffer/)

### [9_LoadMesh](https://github.com/BobLChen/VulkanDemos/tree/master/examples/9_LoadMesh)
[博客地址](http://xiaopengyou.fun/public/2019/08/02/9_LoadMesh/)
![9_LoadMesh](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/9_LoadMesh.jpg)

### [10_Pipelines](https://github.com/BobLChen/VulkanDemos/tree/master/examples/10_Pipelines)
[博客地址](http://xiaopengyou.fun/public/2019/08/02/10_Pipelines/)
![10_Pipelines](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/10_Pipelines.jpg)

### [11_Texture](https://github.com/BobLChen/VulkanDemos/tree/master/examples/11_Texture)
[博客地址](http://xiaopengyou.fun/public/2019/08/02/11_Texture/)
![11_Texture](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/11_Texture.jpg)

### [12_PushConstants](https://github.com/BobLChen/VulkanDemos/tree/master/examples/12_PushConstants)
[博客地址](http://xiaopengyou.fun/public/2019/08/12/12_PushConstants/#more)
![12_PushConstants](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/12_PushConstants.jpg)

### [13_DynamicUniformBuffer](https://github.com/BobLChen/VulkanDemos/tree/master/examples/13_DynamicUniformBuffer)
![13_DynamicUniformBuffer](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/13_DynamicUniformBuffer.jpg)

### [14_TextureArray](https://github.com/BobLChen/VulkanDemos/tree/master/examples/14_TextureArray)
![14_TextureArray](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/14_TextureArray.jpg)

### [15_Texture3D](https://github.com/BobLChen/VulkanDemos/tree/master/examples/15_Texture3D)
![15_Texture3D](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/15_Texture3D.jpg)

### [16_OptimizeShaderAndLayout](https://github.com/BobLChen/VulkanDemos/tree/master/examples/16_OptimizeShaderAndLayout)

### [17_InputAttachments](https://github.com/BobLChen/VulkanDemos/tree/master/examples/17_InputAttachments)
![17_InputAttachments](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/17_InputAttachments.jpg)

### [18_SimpleDeferredShading](https://github.com/BobLChen/VulkanDemos/tree/master/examples/18_DeferredShading)
- Albedo:VK_FORMAT_R8G8B8A8_UNORM
- Normal:VK_FORMAT_R16G16B16A16_SFLOAT
- Position:VK_FORMAT_R16G16B16A16_SFLOAT
![18_DeferredShading](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/18_DeferredShading.jpg)

### [19_OptimizeDeferredShading](https://github.com/BobLChen/VulkanDemos/tree/master/examples/19_OptimizeDeferredShading)
- Albedo: VK_FORMAT_R8G8B8A8_UNORM
- Normal: VK_FORMAT_R8G8B8A8_UNORM
- Position: Reconstructing world space position from depth buffer
![19_OptimizeDeferredShading](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/19_OptimizeDeferredShading_1.jpg)

### [20_Material](https://github.com/BobLChen/VulkanDemos/tree/master/examples/20_Material)

### [21_Stencil](https://github.com/BobLChen/VulkanDemos/tree/master/examples/21_Stencil)
![21_Stencil](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/21_Stencil.jpg)

### [22_RenderTarget(30 Filter)](https://github.com/BobLChen/VulkanDemos/tree/master/examples/21_Stencil)
![22_RenderTarget](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/22_RenderTarget.gif)

### [23_RenderTarget](https://github.com/BobLChen/VulkanDemos/tree/master/examples/22_RenderTarget)

### [24_EdgeDetect](https://github.com/BobLChen/VulkanDemos/tree/master/examples/24_EdgeDetect)
![24_EdgeDetect](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/24_NormalEdge.jpg)

### [25_Bloom](https://github.com/BobLChen/VulkanDemos/tree/master/examples/25_Bloom)
![25_Bloom](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/25_Bloom.jpg)

### [26_SkeletonMatrix4x4](https://github.com/BobLChen/VulkanDemos/tree/master/examples/26_SkeletonMatrix4x4)
![26_Skeleton](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/26_Skeleton.gif)

### [27_SkeletonPackIndexWeight](https://github.com/BobLChen/VulkanDemos/tree/master/examples/27_SkeletonPackIndexWeight)
- Pack 4 bone index(uint32) to 1 UInt32
- Pack 4 bone weight(float) to 2 UInt32
Reduce 5 float per vertex

### [28_SkeletonQuat](https://github.com/BobLChen/VulkanDemos/tree/master/examples/28_SkeletonQuat)
- Dual quat animation, reduce 8 float per bone. From matrix4x4 to 2 vector.

![28_SkeletonDualQuat](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/28_SkeletonDualQuat.gif)

### [29_VertexTextureSkin](https://github.com/BobLChen/VulkanDemos/tree/master/examples/29_SkinInTexture)
- Store skeleton datas in texture and used in vertex shader.

### [30_InstanceSkin](https://github.com/BobLChen/VulkanDemos/tree/master/examples/30_SkinInstance)
![30_InstanceSkin](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/32_SkinInstance.gif)

### [31_MSAA](https://github.com/BobLChen/VulkanDemos/tree/master/examples/31_MSAA)
![31_MSAA](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/29_MSAA.gif)

### [32_FXAA](https://github.com/BobLChen/VulkanDemos/tree/master/examples/32_FXAA)
![32_FXAA](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/30_FXAA.gif)

### [33_InstanceDraw](https://github.com/BobLChen/VulkanDemos/tree/master/examples/33_InstanceDraw)
![33_InstanceDraw](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/33_InstanceDraw.jpg)

### [34_SimpleShadow](https://github.com/BobLChen/VulkanDemos/tree/master/examples/34_SimpleShadow)
![34_SimpleShadow](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/34_SimpleShadow.jpg)

### [35_PCFShadow](https://github.com/BobLChen/VulkanDemos/tree/master/examples/35_PCFShadow)
![35_PCFShadow](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/35_PCFShadow.jpg)

### [36_OmniShadow(Multiview)](https://github.com/BobLChen/VulkanDemos/tree/master/examples/36_OmniShadow)
![36_OmniShadow](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/36_OmniShadow.jpg)

### [37_CascadedShadow](https://github.com/BobLChen/VulkanDemos/tree/master/examples/37_CascadedShadow)
![37_CascadedShadow](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/37_CascadedShadow.gif)

### [38_IndirectDraw](https://github.com/BobLChen/VulkanDemos/tree/master/examples/38_IndirectDraw)
![38_IndirectDraw](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/38_IndirectDraw.jpg)

### [39_OcclusionQueries](https://github.com/BobLChen/VulkanDemos/tree/master/examples/39_OcclusionQueries)
![39_OcclusionQueries](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/39_GpuQuery.gif)

### [40_QueryStatistics](https://github.com/BobLChen/VulkanDemos/tree/master/examples/40_QueryStatistics)
![40_QueryStatistics](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/40_QueryStatistics.jpg)

### [41_ComputeShader](https://github.com/BobLChen/VulkanDemos/tree/master/examples/41_ComputeShader)
![41_ComputeShader](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/41_ComputeShader.gif)

### [42_OptimizeComputeShader](https://github.com/BobLChen/VulkanDemos/tree/master/examples/42_OptimizeComputeShader)

### [43_ComputeParticles](https://github.com/BobLChen/VulkanDemos/tree/master/examples/43_ComputeParticles)
![43_ComputeParticles](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/43_ComputeParticles.jpg)

### [44_ComputeRaytracing](https://github.com/BobLChen/VulkanDemos/tree/master/examples/44_ComputeRaytracing)
![44_ComputeRaytracing](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/44_ComputeRaytracing.png)

### [45_ComputeFrustum](https://github.com/BobLChen/VulkanDemos/tree/master/examples/45_ComputeFrustum)
![45_ComputeFrustum](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/45_ComputeFrustum.jpg)

### [46_GeometryHouse](https://github.com/BobLChen/VulkanDemos/tree/master/examples/46_GeometryHouse)
![46_GeometryHouse](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/46_GeometryHouse.jpg)

### [47_DebugNormal](https://github.com/BobLChen/VulkanDemos/tree/master/examples/47_DebugNormal)
![47_DebugNormal](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/47_DebugNormal.jpg)

### [48_GeometryOmniShadow](https://github.com/BobLChen/VulkanDemos/tree/master/examples/48_GeometryOmniShadow)
![48_GeometryOmniShadow](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/36_OmniShadow.jpg)

### [49_SimpleTessellation](https://github.com/BobLChen/VulkanDemos/tree/master/examples/49_SimpleTessellation)
![49_SimpleTessellation](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/49_SimpleTessellation.jpg)

### [50_PNTessellation](https://github.com/BobLChen/VulkanDemos/tree/master/examples/50_PNTessellation)
![50_PNTessellation](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/50_PNTessellation.gif)

### [51_Pick](https://github.com/BobLChen/VulkanDemos/tree/master/examples/51_Pick)
![51_Pick](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/51_Pick.gif)

### [52_HDRPipeline](https://github.com/BobLChen/VulkanDemos/tree/master/examples/52_HDRPipeline)
![52_HDRPipeline](https://raw.githubusercontent.com/BobLChen/VulkanDemos/master/preview/52_HDRPipelines.gif)
