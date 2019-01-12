set(Monkey_Common_HDRS
	Monkey/Common/Common.h
)
set(Monkey_Common_SRCS
	
)

set(Monkey_Platform_GLFW_HDRS
	Monkey/Platform/glfw/GLFWApplication.h
)
set(Monkey_Platform_GLFW_SRCS
	Monkey/Platform/glfw/GLFWApplication.cpp
)

set(Monkey_App_HDRS
	Monkey/Sample.h
	Monkey/Application.h
)
set(Monkey_App_SRCS
	Monkey/Sample.cpp
	Monkey/Application.cpp
)

add_library(Monkey STATIC
	${Monkey_Common_HDRS}
	${Monkey_Common_SRCS}

	${Monkey_App_HDRS}
	${Monkey_App_SRCS}

	${Monkey_Platform_GLFW_HDRS} 
	${Monkey_Platform_GLFW_SRCS}
)

source_group(Monkey FILES ${Monkey_App_HDRS} ${Monkey_App_SRCS})
source_group(Monkey\\Common FILES ${Monkey_Common_HDRS} ${Monkey_Common_SRCS})
source_group(Monkey\\Platform\\glfw FILES ${Monkey_Platform_GLFW_HDRS} ${Monkey_Platform_GLFW_SRCS})