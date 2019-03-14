cmake_minimum_required(VERSION 3.12.0)

set(NAME triangle)
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

include_directories(${ANDROID_NDK}/sources/android/native_app_glue)

add_subdirectory(../base)

add_library(native-app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

add_library(native-lib SHARED 
    triangle.cpp
)

target_link_libraries(
    native-lib
    native-app-glue
    android
    log
    z
)
