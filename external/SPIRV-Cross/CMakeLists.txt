cmake_minimum_required(VERSION 2.8)
project(SPIRV-Cross)

set(SPIRV_Cross_HDRS
    GLSL.std.450.h
    spirv_common.hpp
    spirv.hpp
    spirv_cross.hpp
    spirv_parser.hpp
    spirv_cross_parsed_ir.hpp
    spirv_cfg.hpp
    spirv_glsl.hpp
    spirv_cpp.hpp
    spirv_reflect.hpp
    spirv_msl.hpp
    spirv_hlsl.hpp
    spirv_cross_util.hpp
)

set(SPIRV_Cross_SRCS
    spirv_cross.cpp
    spirv_parser.cpp
    spirv_cross_parsed_ir.cpp
    spirv_cfg.cpp
    spirv_glsl.cpp
    spirv_cpp.cpp
    spirv_reflect.cpp
    spirv_msl.cpp
    spirv_hlsl.cpp
    spirv_cross_util.cpp
)

add_library(SPIRV_Cross STATIC
	  ${SPIRV_Cross_HDRS}
	  ${SPIRV_Cross_SRCS}
)

source_group(src\\ FILES ${SPIRV_Cross_HDRS} ${SPIRV_Cross_SRCS})