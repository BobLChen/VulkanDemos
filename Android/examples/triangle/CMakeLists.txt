cmake_minimum_required(VERSION 3.6.0)

set(ANDROID TRUE CACHE BOOL "")

set(NAME triangle)
set(BASE_DIR ../../../../VulkanTutorials)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DNOMINMAX=1 -DMONKEY_DEBUG=1 -DPLATFORM_ANDROID=1")

include_directories(
	${BASE_DIR}/Engine/Monkey/
	${BASE_DIR}/external/imgui/
    ${BASE_DIR}/external/SPIRV-Cross/
    ${BASE_DIR}/external/assimp/include/
    ${ANDROID_NDK}/sources/android/native_app_glue
)

add_subdirectory(${BASE_DIR}/Engine Build/Monkey)
add_subdirectory(${BASE_DIR}/external/imgui Build/Imgui)
add_subdirectory(${BASE_DIR}/external/SPIRV-Cross Build/SPIRV-Cross)
add_subdirectory(${BASE_DIR}/external/assimp Build/assimp)

add_library(native-app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

add_library(monkeyandroid-lib SHARED
    ${BASE_DIR}/Engine/Monkey/Launch/LaunchAndroid.cpp
    ${BASE_DIR}/examples/2_Triangle/Triangle.cpp
)

target_link_libraries(
    monkeyandroid-lib
    imgui
	SPIRV_Cross
    Monkey
    native-app-glue
    android
    log
    z
)