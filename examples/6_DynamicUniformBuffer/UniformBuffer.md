# Vulkan Uniform Buffer设计
最近我一直在思考关于UniformBuffer的结构如何设计。由于工作太忙，只能断断续续的设计它的结构以便于能够更好的匹配绘制VkPipeline。对D3d12或者Vulkan有过了解的同学应该也会有这个疑问，就是它们的UniformBuffer如何才能使用起来更加方便。

例如D3D12关于UniformBuffer的使用

[Github直达](https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12Bundles/src/FrameResource.cpp#L31)
```C++
// Create an upload heap for the constant buffers.
ThrowIfFailed(pDevice->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
    D3D12_HEAP_FLAG_NONE,
    &CD3DX12_RESOURCE_DESC::Buffer(sizeof(SceneConstantBuffer) * m_cityRowCount * m_cityColumnCount),
    D3D12_RESOURCE_STATE_GENERIC_READ,
    nullptr,
    IID_PPV_ARGS(&m_cbvUploadHeap)));
```

或者Vulkan关于UniformBuffer的创建。

[Github直达](https://github.com/SaschaWillems/Vulkan/blob/master/examples/triangle/triangle.cpp#L1023)
```C++
// Create a new buffer
VK_CHECK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &uniformBufferVS.buffer));
// Get memory requirements including size, alignment and memory type 
vkGetBufferMemoryRequirements(device, uniformBufferVS.buffer, &memReqs);
allocInfo.allocationSize = memReqs.size;
// Get the memory type index that supports host visibile memory access
// Most implementations offer multiple memory types and selecting the correct one to allocate memory from is crucial
// We also want the buffer to be host coherent so we don't have to flush (or sync after every update.
// Note: This may affect performance so you might not want to do this in a real world application that updates buffers on a regular base
allocInfo.memoryTypeIndex = getMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
// Allocate memory for the uniform buffer
VK_CHECK_RESULT(vkAllocateMemory(device, &allocInfo, nullptr, &(uniformBufferVS.memory)));
// Bind memory to buffer
VK_CHECK_RESULT(vkBindBufferMemory(device, uniformBufferVS.buffer, uniformBufferVS.memory, 0));
```

从以上代码我们可以看出，如果渲染固定数量的对象，以上方式完全没有任何问题。但是我们现在设想如下两种情况：
- 多个Mesh共用一个材质
- 多个动态创建的Mesh

## 多个Mesh共用一个材质
这种情况在引擎里面非常非常常见，其它人员会制作好一个材质，调节好参数以及Shader，然后赋予给多个相同或者不同的Mesh。在渲染的时候，常规情况下除了Mesh的World矩阵会发生变化，其它材质参数在当前批次里面是不会发生变化的。

我之前设想的是，针对World矩阵创建出多个UniformBuffer或者创建一个Dynamic属性的UniformBuffer。但是这种方式不易于管理，因为我们要针对不同的材质、不同的Backbuffer分别创建出UniformBuffer或者DynamicUniformBuffer，并且这种方式会导致碎片化非常严重。

## 动态创建的Mesh
我设想的是，无论是否为动态创建的物体，最终都是被收集到RenderList里面，按照远近、材质进行归类。这样在真正录制渲染命令的时候，就可以根据这个列表进行UniformBuffer的动态创建以及分配。

如下：

[0, 0, 0] [1, 1, 1, 1] [2, 2]

一共有3 + 4 + 2个Mesh需要渲染，它们的材质ID分别为0，1，2。按照这样的方式，我们就可以在渲染ID为0的材质的时候，为它分配3个Matrix4x4 UniformBuffer用于存放World矩阵数据，为它分配1个ShaderParam UniformBuffer用于存放其它材质参数。

这种方式其实需要依托于我们有一个健壮强大动态分配器以及回收器，同时还要能够规避碎片化问题。最终我放弃了这种方式，觉得实在难以管理以及实现。

## 利用Dynamic属性的UniformBuffer

Dynamic UniformBuffer其实也是UniformBuffer，它们并没有任何不同，也没有性能上的差异。但是可以利用在创建DescriptorSetLayout时指定为Dynamic的UniformBuffer，在录制绘制命令时，指定DynamicUniformBuffer的Offset，以达到每个Drawcall存储不同数据，但是都是在一个UniformBuffer上面操作的目的。

之前设想的方案也是需要利用到这个特性，因为World矩阵需要利用DynamicBuffer来更新。后面放弃这种方案是因为，我打算将所有的UniformBuffer都设置Dynamic属性。
