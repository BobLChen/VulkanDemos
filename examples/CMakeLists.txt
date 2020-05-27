MACRO(SETUP_SAMPLE_START EXE_NAME)
	UNSET(SOURCE_FILES)
	UNSET(HEADER_FILES)
	UNSET(RESOURCE_FILES)
	UNSET(DEFINITIONS)
ENDMACRO(SETUP_SAMPLE_START)

MACRO(SETUP_SAMPLE_END EXE_NAME)
	SOURCE_GROUP("Source Files" FILES ${SOURCE_FILES})
	SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
	SOURCE_GROUP(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Resources" FILES ${RESOURCE_FILES})

	ADD_EXECUTABLE(${EXE_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES} ${EFFECT_FILES})
	SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES FOLDER examples)

if (IOS)
	# set_source_files_properties( 
	# 	star.obj        # 3D model as OBJ txt file
	# 	star.off        # 3D model as OFF txt file
	# 	PROPERTIES 
	# 	HEADER_FILE_ONLY ON 
	# ) 
	SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES
		MACOSX_BUNDLE TRUE
		RESOURCE "${RESOURCE_FILES}"
	)
elseif (APPLE)
	SET(MACOS_ENVIRONMENT "VK_LAYER_PATH=${CMAKE_SOURCE_DIR}/external/vulkan/macos/etc/vulkan/explicit_layer.d;VK_ICD_FILENAMES=${CMAKE_SOURCE_DIR}/external/vulkan/macos/etc/vulkan/icd.d/MoltenVK_icd.json")
	SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES XCODE_SCHEME_ENVIRONMENT "${MACOS_ENVIRONMENT}")
elseif(WIN32)
	SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Debug")
	SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES VS_RELEASE_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Debug")
endif()
	
	TARGET_COMPILE_DEFINITIONS(${EXE_NAME} PRIVATE ${DEFINITIONS})
	SET(EXTRA_LINKED_LIBRARIES 
		${EXTRA_LINKED_LIBRARIES}
		${ALL_LIBS}
	)
	TARGET_LINK_LIBRARIES(${EXE_NAME} ${EXTRA_LINKED_LIBRARIES})
ENDMACRO(SETUP_SAMPLE_END)

if (WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	SET(MainLaunch
		${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Monkey/Launch/LaunchWindows.cpp
	)
elseif (IOS)
	SET(MainLaunch
		${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Monkey/Launch/LaunchIOS.cpp
	)
	set_source_files_properties(
		${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Monkey/Launch/LaunchIOS.cpp
		PROPERTIES 
        COMPILE_FLAGS "-x objective-c++"
	)
elseif (APPLE)
	SET(MainLaunch
		${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Monkey/Launch/LaunchMac.cpp
	)
	set_source_files_properties(
		${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Monkey/Launch/LaunchMac.cpp
		PROPERTIES 
		COMPILE_FLAGS "-x objective-c++"
	)
elseif (UNIX)
	SET(MainLaunch
		${CMAKE_CURRENT_SOURCE_DIR}/../Engine/Monkey/Launch/LaunchLinux.cpp
	)
endif ()

SETUP_SAMPLE_START(2_Triangle)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/2_Triangle/Triangle.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/2_Triangle/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(2_Triangle)

SETUP_SAMPLE_START(3_DemoBase)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/3_DemoBase/Triangle.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/2_Triangle/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(3_DemoBase)

SETUP_SAMPLE_START(4_OptimizeBuffer)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/4_OptimizeBuffer/Triangle.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/2_Triangle/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(4_OptimizeBuffer)

SETUP_SAMPLE_START(5_OptimizeCommandBuffer)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/5_OptimizeCommandBuffer/Triangle.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/2_Triangle/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(5_OptimizeCommandBuffer)

SETUP_SAMPLE_START(6_ImageGUI)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/6_ImageGUI/Triangle.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/2_Triangle/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(6_ImageGUI)

SETUP_SAMPLE_START(7_UniformBuffer)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/7_UniformBuffer/UniformBuffer.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/7_UniformBuffer/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(7_UniformBuffer)

SETUP_SAMPLE_START(8_OptimizeVertexIndexBuffer)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/8_OptimizeVertexIndexBuffer/Triangle.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/8_OptimizeVertexIndexBuffer/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(8_OptimizeVertexIndexBuffer)

SETUP_SAMPLE_START(9_LoadMesh)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/9_LoadMesh/9_LoadMesh.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/9_LoadMesh/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(9_LoadMesh)

SETUP_SAMPLE_START(10_Pipelines)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/10_Pipelines/Pipelines.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/10_Pipelines/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(10_Pipelines)

SETUP_SAMPLE_START(11_Texture)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/11_Texture/Texture.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/11_Texture/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(11_Texture)

SETUP_SAMPLE_START(12_PushConstants)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/12_PushConstants/PushConstants.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/12_PushConstants/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(12_PushConstants)

SETUP_SAMPLE_START(13_DynamicUniformBuffer)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/13_DynamicUniformBuffer/DynamicUniformBuffer.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/13_DynamicUniformBuffer/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(13_DynamicUniformBuffer)

SETUP_SAMPLE_START(14_TextureArray)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/14_TextureArray/TextureArray.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/14_TextureArray/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(14_TextureArray)

SETUP_SAMPLE_START(15_Texture3D)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/15_Texture3D/Texture3D.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/15_Texture3D/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(15_Texture3D)

SETUP_SAMPLE_START(16_OptimizeShaderAndLayout)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/16_OptimizeShaderAndLayout/OptimizeShaderAndLayout.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/16_OptimizeShaderAndLayout/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(16_OptimizeShaderAndLayout)

SETUP_SAMPLE_START(17_InputAttachments)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/17_InputAttachments/InputAttachments.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/17_InputAttachments/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(17_InputAttachments)

SETUP_SAMPLE_START(18_DeferredShading)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/18_DeferredShading/DeferredShading.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/18_DeferredShading/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(18_DeferredShading)

SETUP_SAMPLE_START(19_OptimizeDeferredShading)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/19_OptimizeDeferredShading/OptimizeDeferredShading.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/19_OptimizeDeferredShading/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(19_OptimizeDeferredShading)

SETUP_SAMPLE_START(20_Material)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/20_Material/MaterialDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/20_Material/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(20_Material)

SETUP_SAMPLE_START(21_Stencil)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/21_Stencil/StencilDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/21_Stencil/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(21_Stencil)

SETUP_SAMPLE_START(22_RenderTarget)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/22_RenderTarget/RenderTargetDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/22_RenderTarget/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(22_RenderTarget)

SETUP_SAMPLE_START(23_OptimizeRenderTarget)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/23_OptimizeRenderTarget/OptimizeRenderTarget.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/23_OptimizeRenderTarget/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(23_OptimizeRenderTarget)

SETUP_SAMPLE_START(24_EdgeDetect)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/24_EdgeDetect/EdgeDetectDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/24_EdgeDetect/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(24_EdgeDetect)

SETUP_SAMPLE_START(25_Bloom)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/25_Bloom/BloomDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/25_Bloom/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(25_Bloom)

SETUP_SAMPLE_START(26_SkeletonMatrix4x4)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/26_SkeletonMatrix4x4/SkeletonMatrix4x4Demo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/26_SkeletonMatrix4x4/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(26_SkeletonMatrix4x4)

SETUP_SAMPLE_START(27_SkeletonPackIndexWeight)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/27_SkeletonPackIndexWeight/SkeletonPackIndexWeightDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/27_SkeletonPackIndexWeight/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(27_SkeletonPackIndexWeight)

SETUP_SAMPLE_START(28_SkeletonQuat)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/28_SkeletonQuat/SkeletonQuatDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/28_SkeletonQuat/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(28_SkeletonQuat)

SETUP_SAMPLE_START(29_SkinInTexture)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/29_SkinInTexture/SkinInTextureDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/29_SkinInTexture/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(29_SkinInTexture)

SETUP_SAMPLE_START(30_SkinInstance)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/30_SkinInstance/SkinInstanceDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/30_SkinInstance/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(30_SkinInstance)

SETUP_SAMPLE_START(31_MSAA)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/31_MSAA/MSAADemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/31_MSAA/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(31_MSAA)

SETUP_SAMPLE_START(32_FXAA)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/32_FXAA/FXAADemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/32_FXAA/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(32_FXAA)

SETUP_SAMPLE_START(33_InstanceDraw)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/33_InstanceDraw/InstanceDrawDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/33_InstanceDraw/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(33_InstanceDraw)

SETUP_SAMPLE_START(34_SimpleShadow)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/34_SimpleShadow/SimpleShadowDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/34_SimpleShadow/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(34_SimpleShadow)

SETUP_SAMPLE_START(35_PCFShadow)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/35_PCFShadow/PCFShadowDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/35_PCFShadow/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(35_PCFShadow)

SETUP_SAMPLE_START(36_OmniShadow)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/36_OmniShadow/OmniShadowDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/36_OmniShadow/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(36_OmniShadow)

SETUP_SAMPLE_START(37_CascadedShadow)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/37_CascadedShadow/CascadedShadowDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/37_CascadedShadow/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(37_CascadedShadow)

SETUP_SAMPLE_START(38_IndirectDraw)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/38_IndirectDraw/IndirectDrawDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/38_IndirectDraw/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(38_IndirectDraw)

SETUP_SAMPLE_START(39_OcclusionQueries)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/39_OcclusionQueries/OcclusionQueryDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/39_OcclusionQueries/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(39_OcclusionQueries)

SETUP_SAMPLE_START(40_QueryStatistics)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/40_QueryStatistics/QueryStatisticsDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/40_QueryStatistics/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(40_QueryStatistics)

SETUP_SAMPLE_START(41_ComputeShader)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/41_ComputeShader/ComputeShaderDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/41_ComputeShader/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(41_ComputeShader)

SETUP_SAMPLE_START(42_OptimizeComputeShader)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/42_OptimizeComputeShader/OptimizeComputeShaderDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/42_OptimizeComputeShader/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(42_OptimizeComputeShader)

SETUP_SAMPLE_START(43_ComputeParticles)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/43_ComputeParticles/ComputeParticlesDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/43_ComputeParticles/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(43_ComputeParticles)

SETUP_SAMPLE_START(44_ComputeRaytracing)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/44_ComputeRaytracing/ComputeRaytracingDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/44_ComputeRaytracing/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(44_ComputeRaytracing)

SETUP_SAMPLE_START(45_ComputeFrustum)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/45_ComputeFrustum/ComputeFrustumDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/45_ComputeFrustum/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(45_ComputeFrustum) 

SETUP_SAMPLE_START(46_GeometryHouse)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/46_GeometryHouse/GeometryHouseDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/46_GeometryHouse/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(46_GeometryHouse) 

SETUP_SAMPLE_START(47_DebugNormal)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/47_DebugNormal/DebugNormalDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/47_DebugNormal/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(47_DebugNormal)

SETUP_SAMPLE_START(48_GeometryOmniShadow)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/48_GeometryOmniShadow/GeometryOmniShadowDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/48_GeometryOmniShadow/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(48_GeometryOmniShadow)

SETUP_SAMPLE_START(49_SimpleTessellation)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/49_SimpleTessellation/SimpleTessellationDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/49_SimpleTessellation/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(49_SimpleTessellation)

SETUP_SAMPLE_START(50_PNTessellation)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/50_PNTessellation/PNTessellationDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/50_PNTessellation/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(50_PNTessellation)

SETUP_SAMPLE_START(51_Pick)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/51_Pick/PickDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/51_Pick/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(51_Pick)

SETUP_SAMPLE_START(52_HDRPipeline)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/52_HDRPipeline/HDRPipelineDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/52_HDRPipeline/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(52_HDRPipeline)

SETUP_SAMPLE_START(53_SSAO)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/53_SSAO/SSAODemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/53_SSAO/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(53_SSAO)

SETUP_SAMPLE_START(54_ThreadedRendering)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/54_ThreadedRendering/ThreadedRenderingDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/54_ThreadedRendering/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(54_ThreadedRendering)

if (NOT WIN32)
	TARGET_LINK_LIBRARIES(54_ThreadedRendering pthread)
endif()

SETUP_SAMPLE_START(55_PBR_DirectLighting)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/55_PBR_DirectLighting/PBRDirectLightingDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/55_PBR_DirectLighting/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(55_PBR_DirectLighting)

SETUP_SAMPLE_START(56_PBR_IBL)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/56_PBR_IBL/PBRIBLDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/56_PBR_IBL/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(56_PBR_IBL)

SETUP_SAMPLE_START(57_GodRay)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/57_GodRay/GodRayDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/57_GodRay/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(57_GodRay)

SETUP_SAMPLE_START(58_Imposter)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/58_Imposter/ImposterDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/58_Imposter/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(58_Imposter)

SETUP_SAMPLE_START(59_MotionBlur)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/59_MotionBlur/MotionBlurDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/59_MotionBlur/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(59_MotionBlur)

SETUP_SAMPLE_START(60_DepthPeeling)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/60_DepthPeeling/DepthPeelingDemo.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/60_DepthPeeling/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(60_DepthPeeling)

SETUP_SAMPLE_START(61_CPURayTracing)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/CPURayTracingDemo.cpp

		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/Runnable.h
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/RunnableThread.h
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/RunnableThread.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/TaskThread.h
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/TaskThread.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/TaskThreadPool.h
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/TaskThreadPool.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/ThreadEvent.h
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/ThreadEvent.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/ThreadManager.h
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/ThreadManager.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/ThreadTask.h
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/RayTracing.h
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/RayTracing.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/Material.h
		${CMAKE_CURRENT_SOURCE_DIR}/61_CPURayTracing/Material.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/61_CPURayTracing/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(61_CPURayTracing)

if (NOT WIN32)
	TARGET_LINK_LIBRARIES(61_CPURayTracing pthread)
endif()

SETUP_SAMPLE_START(62_RTXRayTracingBasic)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/62_RTXRayTracingBasic/RTXRayTracingStartBasic.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/62_RTXRayTracingBasic/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(62_RTXRayTracingBasic)

SETUP_SAMPLE_START(63_RTXRayTracingMesh)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/63_RTXRayTracingMesh/RTXRayTracingMeshDemo.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/63_RTXRayTracingMesh/tiny_gltf.h
		${CMAKE_CURRENT_SOURCE_DIR}/63_RTXRayTracingMesh/json.hpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/63_RTXRayTracingMesh/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(63_RTXRayTracingMesh)

SETUP_SAMPLE_START(64_RTXRayTracingSimple)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/64_RTXRayTracingSimple/RTXRayTracingSimpleDemo.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/64_RTXRayTracingSimple/tiny_gltf.h
		${CMAKE_CURRENT_SOURCE_DIR}/64_RTXRayTracingSimple/json.hpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/64_RTXRayTracingSimple/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(64_RTXRayTracingSimple)

SETUP_SAMPLE_START(65_RTXRayTracingReflection)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/65_RTXRayTracingReflection/RTXRayTracingReflectionDemo.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/65_RTXRayTracingReflection/tiny_gltf.h
		${CMAKE_CURRENT_SOURCE_DIR}/65_RTXRayTracingReflection/json.hpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/65_RTXRayTracingReflection/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(65_RTXRayTracingReflection)

SETUP_SAMPLE_START(66_RTXRayTracingHitGroup)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/66_RTXRayTracingHitGroup/RTXRayTracingHitGroupDemo.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/66_RTXRayTracingHitGroup/tiny_gltf.h
		${CMAKE_CURRENT_SOURCE_DIR}/66_RTXRayTracingHitGroup/json.hpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/66_RTXRayTracingHitGroup/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(66_RTXRayTracingHitGroup)

SETUP_SAMPLE_START(67_RTXRayTracingMonteCarlo)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/67_RTXRayTracingMonteCarlo/RTXRayTracingMonteCarloDemo.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/67_RTXRayTracingMonteCarlo/tiny_gltf.h
		${CMAKE_CURRENT_SOURCE_DIR}/67_RTXRayTracingMonteCarlo/json.hpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/67_RTXRayTracingMonteCarlo/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(67_RTXRayTracingMonteCarlo)

SETUP_SAMPLE_START(68_RTXPathTracing)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/68_RTXPathTracing/RTXPathTracingDemo.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/68_RTXPathTracing/tiny_gltf.h
		${CMAKE_CURRENT_SOURCE_DIR}/68_RTXPathTracing/json.hpp
		${CMAKE_CURRENT_SOURCE_DIR}/68_RTXPathTracing/LightData.h
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/68_RTXPathTracing/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(68_RTXPathTracing)