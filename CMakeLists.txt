cmake_minimum_required(VERSION 3.12.0)

project(VulkanTutorials)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_STANDARD 11)

add_definitions(-DDEMO_RES_PATH=\"${CMAKE_SOURCE_DIR}/examples/\")
add_definitions(-DNOMINMAX=1)
add_definitions(-DMONKEY_DEBUG=1)

if (WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-DPLATFORM_WINDOWS=1)
	include_directories(
		external/vulkan/windows/include/
	)
elseif (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_MAC=1")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework QuartzCore")
	set(CMAKE_XCODE_GENERATE_SCHEME ON)
	set(XCODE_SCHEME_ENVIRONMENT ON)
	include_directories(
		external/vulkan/macos/include/
	)
elseif (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_LINUX=1")
	include_directories(
		external/vulkan/linux/include/
	)
	find_package(XCB REQUIRED)
	include_directories(
		${XCB_INCLUDE_DIRS}
	)
endif ()

find_package(Vulkan)

set(ALL_LIBS
	${Vulkan_LIBRARY}
	imgui
	assimp
	spirv-cross-glsl
	spirv-cross-hlsl
	spirv-cross-cpp
	spirv-cross-reflect
	spirv-cross-msl
	spirv-cross-util
	spirv-cross-core
	Monkey
)

if (UNIX AND NOT APPLE)
	set(ALL_LIBS
		${ALL_LIBS}
		${XCB_LIBRARIES}
	)
endif ()

include_directories(
	Engine/Monkey/
	external/imgui/
	external/SPIRV-Cross/
	external/assimp/include/
)

add_subdirectory(external/imgui)
add_subdirectory(external/SPIRV-Cross)
add_subdirectory(external/assimp)
add_subdirectory(Engine)
add_subdirectory(examples)