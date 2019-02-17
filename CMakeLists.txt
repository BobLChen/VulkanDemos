cmake_minimum_required(VERSION 3.12.0)

project(VulkanTutorials)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DNOMINMAX=1")

if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_WINDOWS=1")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 HINTS "${CMAKE_SOURCE_DIR}/external/vulkan/windows/libs/x64" REQUIRED)
	else (CMAKE_SIZEOF_VOID_P EQUAL 8)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 HINTS "${CMAKE_SOURCE_DIR}/external/vulkan/windows/libs/x86" REQUIRED)
	endif (CMAKE_SIZEOF_VOID_P EQUAL 8)
	include_directories(
		external/vulkan/windows/include/
	)
elseif (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_MAC=1")
	set(CMAKE_XCODE_GENERATE_SCHEME ON)
	set(XCODE_SCHEME_ENVIRONMENT ON)
	find_library(Vulkan_LIBRARY NAMES vulkan HINTS "${CMAKE_SOURCE_DIR}/external/vulkan/macos/lib" REQUIRED)
	include_directories(
		external/vulkan/macos/include/
	)
elseif (UNIX AND NOT APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_UNIX=1")
	find_library(Vulkan_LIBRARY NAMES vulkan HINTS "${CMAKE_SOURCE_DIR}/external/vulkan/linux/lib" REQUIRED)
	include_directories(
		external/vulkan/linux/include/
	)
endif ()

set(ALL_LIBS
	${Vulkan_LIBRARY}
	glfw
	imgui
	Monkey
)

include_directories(
	Engine/Monkey/
	external/glfw/include/
	external/glm/include/
	external/imgui/
)

add_subdirectory(Engine)
add_subdirectory(external/imgui)
add_subdirectory(examples)
add_subdirectory(external/glfw)