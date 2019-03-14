set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -DNOMINMAX=1 -DMONKEY_DEBUG=1 -DPLATFORM_ANDROID=1")

include_directories(
	../../Engine/Monkey/
	../../external/glm/include/
	../../external/imgui/
	../../external/SPIRV-Cross/
)

add_subdirectory(../../Engine)
add_subdirectory(../../external/imgui)
add_subdirectory(../../external/SPIRV-Cross)

set(ALL_LIBS
	imgui
	SPIRV_Cross
	Monkey
)