cmake_minimum_required(VERSION 3.12.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

project(VulkanTutorials)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DNOMINMAX=1")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-std=c++11)

# res data
add_definitions(-DDEMO_RES_PATH=\"${CMAKE_SOURCE_DIR}/examples/\")

if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_WINDOWS=1 -DMONKEY_DEBUG=1")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 HINTS "${CMAKE_SOURCE_DIR}/external/vulkan/windows/libs/x64" REQUIRED)
	else (CMAKE_SIZEOF_VOID_P EQUAL 8)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 HINTS "${CMAKE_SOURCE_DIR}/external/vulkan/windows/libs/x86" REQUIRED)
	endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
	include_directories(
		external/vulkan/windows/include/
	)
elseif (IOS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_IOS=1")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Metal -framework Foundation -framework QuartzCore -framework IOKit -framework UIKit -framework IOSurface")
	set(Vulkan_LIBRARY "${CMAKE_SOURCE_DIR}/external/vulkan/ios/lib/libMoltenVK.a")
	include_directories(
		external/vulkan/ios/include/
	)
elseif (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_MAC=1 -DMONKEY_DEBUG=1")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework QuartzCore")
	set(CMAKE_XCODE_GENERATE_SCHEME ON)
	set(XCODE_SCHEME_ENVIRONMENT ON)
	find_library(Vulkan_LIBRARY NAMES vulkan HINTS "${CMAKE_SOURCE_DIR}/external/vulkan/macos/lib" REQUIRED)
	include_directories(
		external/vulkan/macos/include/
	)
elseif (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_LINUX=1 -DMONKEY_DEBUG=1")
	find_library(Vulkan_LIBRARY NAMES vulkan HINTS "${CMAKE_SOURCE_DIR}/external/vulkan/linux/lib" REQUIRED)
	include_directories(
		external/vulkan/linux/include/
	)
	find_package(XCB REQUIRED)
	include_directories(
		${XCB_INCLUDE_DIRS}
	)
endif ()

set(ALL_LIBS
	${Vulkan_LIBRARY}
	imgui
	assimp
	SPIRV_Cross
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