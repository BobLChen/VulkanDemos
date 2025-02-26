cmake_minimum_required(VERSION 3.10.0)

project(imgui)

set(IMGUI_HDRS
	imconfig.h
	imgui.h
	imgui_internal.h
	imstb_rectpack.h
	imstb_textedit.h
	imstb_truetype.h
	imgui_impl_vulkan.h
)

set(IMGUI_SRCS
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_widgets.cpp
	imgui_impl_vulkan.cpp
)

add_library(imgui STATIC
	${IMGUI_HDRS}
	${IMGUI_SRCS}
)

source_group(src\\ FILES ${IMGUI_HDRS} ${IMGUI_SRCS})