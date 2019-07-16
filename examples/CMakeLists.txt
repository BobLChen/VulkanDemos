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

SETUP_SAMPLE_START(10_Pipelines.cpp)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/10_Pipelines/10_Pipelines.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/10_Pipelines/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(10_Pipelines.cpp)

# SETUP_SAMPLE_START(4_Pipelines)
# 	SET(SOURCE_FILES
# 		${MainLaunch}
# 		${CMAKE_CURRENT_SOURCE_DIR}/4_Pipelines/Pipelines.cpp
# 	)
# 	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/4_Pipelines/*.*")
# 	foreach(file ${files})
# 		SET(ASSETS
# 			${ASSETS}
# 			${file}
# 		)
# 	endforeach()
# 	SET(RESOURCE_FILES ${ASSETS})
# SETUP_SAMPLE_END(4_Pipelines)

# SETUP_SAMPLE_START(5_Texture)
# 	SET(SOURCE_FILES
# 		${MainLaunch}
# 		${CMAKE_CURRENT_SOURCE_DIR}/5_Texture/Texture.cpp
# 	)
# 	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/5_Texture/*.*")
# 	foreach(file ${files})
# 		SET(ASSETS
# 			${ASSETS}
# 			${file}
# 		)
# 	endforeach()
# 	SET(RESOURCE_FILES ${ASSETS})
# SETUP_SAMPLE_END(5_Texture)

# SETUP_SAMPLE_START(6_DynamicUniformBuffer)
# 	SET(SOURCE_FILES
# 		${MainLaunch}
# 		${CMAKE_CURRENT_SOURCE_DIR}/6_DynamicUniformBuffer/DynamicUniformBuffer.cpp
# 	)
# 	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/6_DynamicUniformBuffer/*.*")
# 	foreach(file ${files})
# 		SET(ASSETS
# 			${ASSETS}
# 			${file}
# 		)
# 	endforeach()
# 	SET(RESOURCE_FILES ${ASSETS})
# SETUP_SAMPLE_END(6_DynamicUniformBuffer)

# SETUP_SAMPLE_START(7_PushConstants)
# 	SET(SOURCE_FILES
# 		${MainLaunch}
# 		${CMAKE_CURRENT_SOURCE_DIR}/7_PushConstants/PushConstants.cpp
# 	)
# 	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/7_PushConstants/*.*")
# 	foreach(file ${files})
# 		SET(ASSETS
# 			${ASSETS}
# 			${file}
# 		)
# 	endforeach()
# 	SET(RESOURCE_FILES ${ASSETS})
# SETUP_SAMPLE_END(7_PushConstants)

# SETUP_SAMPLE_START(8_SpecializationConstant)
# 	SET(SOURCE_FILES
# 		${MainLaunch}
# 		${CMAKE_CURRENT_SOURCE_DIR}/8_SpecializationConstant/SpecializationConstant.cpp
# 	)
# 	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/8_SpecializationConstant/*.*")
# 	foreach(file ${files})
# 		SET(ASSETS
# 			${ASSETS}
# 			${file}
# 		)
# 	endforeach()
# 	SET(RESOURCE_FILES ${ASSETS})
# SETUP_SAMPLE_END(8_SpecializationConstant)

# SETUP_SAMPLE_START(9_TextureArray)
# 	SET(SOURCE_FILES
# 		${MainLaunch}
# 		${CMAKE_CURRENT_SOURCE_DIR}/9_TextureArray/TextureArray.cpp
# 	)
# 	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/9_TextureArray/*.*")
# 	foreach(file ${files})
# 		SET(ASSETS
# 			${ASSETS}
# 			${file}
# 		)
# 	endforeach()
# 	SET(RESOURCE_FILES ${ASSETS})
# SETUP_SAMPLE_END(9_TextureArray)

# SETUP_SAMPLE_START(10_Texture3D)
# 	SET(SOURCE_FILES
# 		${MainLaunch}
# 		${CMAKE_CURRENT_SOURCE_DIR}/10_Texture3D/Texture3D.cpp
# 	)
# 	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/10_Texture3D/*.*")
# 	foreach(file ${files})
# 		SET(ASSETS
# 			${ASSETS}
# 			${file}
# 		)
# 	endforeach()
# 	SET(RESOURCE_FILES ${ASSETS})
# SETUP_SAMPLE_END(10_Texture3D)