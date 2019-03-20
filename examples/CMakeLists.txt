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

SETUP_SAMPLE_START(3_OBJLoader)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/3_OBJLoader/OBJLoader.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/3_OBJLoader/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(3_OBJLoader)

SETUP_SAMPLE_START(4_Pipelines)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/4_Pipelines/Pipelines.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/4_Pipelines/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(4_Pipelines)

SETUP_SAMPLE_START(5_DescriptorSets)
	SET(SOURCE_FILES
		${MainLaunch}
		${CMAKE_CURRENT_SOURCE_DIR}/5_DescriptorSets/DescriptorSets.cpp
	)
	file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/5_DescriptorSets/*.*")
	foreach(file ${files})
		SET(ASSETS
			${ASSETS}
			${file}
		)
	endforeach()
	SET(RESOURCE_FILES ${ASSETS})
SETUP_SAMPLE_END(5_DescriptorSets)