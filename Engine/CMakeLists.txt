set(Monkey_Configuration_HDRS
	Monkey/Configuration/PerPlatformCppDefines.h
	Monkey/Configuration/Platform.h
)
set(Monkey_Configuration_SRCS

)

set(Monkey_Common_HDRS
	Monkey/Common/Common.h
	Monkey/Common/Log.h
)
set(Monkey_Common_SRCS
	
)

set(Monkey_Platform_GLFW_HDRS
	Monkey/Platform/glfw/GLFWApplicationExample.h
)
set(Monkey_Platform_GLFW_SRCS
	Monkey/Platform/glfw/GLFWApplicationExample.cpp
)

set(Monkey_App_HDRS
	Monkey/Sample.h
	Monkey/Application.h
	Monkey/Engine.h
)
set(Monkey_App_SRCS
	Monkey/Sample.cpp
	Monkey/Application.cpp
	Monkey/Engine.cpp
)

set(Monkey_Launch_HDRS
	
)
set(Monkey_Launch_SRCS
	Monkey/Launch/Launch.cpp
)

set(Monkey_Application_HDRS
	Monkey/Application/GenericWindow.h
	Monkey/Application/GenericApplication.h
	Monkey/Application/GenericApplicationMessageHandler.h
	Monkey/Application/SlateApplication.h
)
set(Monkey_Application_SRCS
	Monkey/Application/GenericWindow.cpp
	Monkey/Application/GenericApplication.cpp
	Monkey/Application/SlateApplication.cpp
)

set(Monkey_Math_HDRS
	Monkey/Math/Math.h
	Monkey/Math/Vector2D.h
	Monkey/Math/IntPoint.h
	Monkey/Math/GenericPlatformMath.h
	Monkey/Math/PlatformMath.h
)
set(Monkey_Math_SRCS
	Monkey/Math/Math.cpp
	Monkey/Math/GenericPlatformMath.cpp
)

set(Monkey_Utils_SRCS
	Monkey/Utils/StringUtils.h
)
set(Monkey_Utils_HDRS
	
)

set(Monkey_Core_HDRS
	Monkey/Core/PixelFormat.h
)
set(Monkey_Core_SRCS
	Monkey/Core/PixelFormat.cpp
)

set(Monkey_Application_Desktop_HDRS
	Monkey/Application/Desktop/GLFWWrapper.h
	Monkey/Application/Desktop/GLFWWindow.h
	Monkey/Application/Desktop/GLFWApplication.h
)
set(Monkey_Application_Desktop_SRCS
	Monkey/Application/Desktop/GLFWWindow.cpp
	Monkey/Application/Desktop/GLFWApplication.cpp
)

set(Monkey_Vulkan_SRCS
	Monkey/Vulkan/VulkanRHI.cpp
	Monkey/Vulkan/VulkanLayers.cpp
	Monkey/Vulkan/VulkanQueue.cpp
	Monkey/Vulkan/VulkanDevice.cpp
	Monkey/Vulkan/VulkanDebug.cpp
	Monkey/Vulkan/VulkanSwapChain.cpp
	Monkey/Vulkan/VulkanMemory.cpp
)
set(Monkey_Vulkan_HDRS
	Monkey/Vulkan/VulkanGenericPlatform.h
	Monkey/Vulkan/VulkanGlobals.h
	Monkey/Vulkan/VulkanRHI.h
	Monkey/Vulkan/VulkanPlatform.h
	Monkey/Vulkan/RHIDefinitions.h
	Monkey/Vulkan/VulkanCommon.h
	Monkey/Vulkan/VulkanQueue.h
	Monkey/Vulkan/VulkanDevice.h
	Monkey/Vulkan/VulkanSwapChain.h
	Monkey/Vulkan/VulkanMemory.h
)

set(Monkey_HAL_HDRS
	Monkey/HAL/PlatformAtomics.h
	Monkey/HAL/GenericPlatformAtomics.h
	Monkey/HAL/ThreadSafeCounter.h
)
set(Monkey_HAL_SRCS
	
)

if (WIN32)
	set(Monkey_Math_Windows_HDRS
		Monkey/Math/Windows/WindowsPlatformMath.h
	)
	set(Monkey_Math_Windows_SRCS
		
	)

	set(Monkey_Vulkan_Windows_HDRS
		Monkey/Vulkan/Windows/VulkanWindowsPlatform.h
		Monkey/Vulkan/Windows/VulkanPlatformDefines.h
	)
	set(Monkey_Vulkan_Windows_SRCS
		Monkey/Vulkan/Windows/VulkanWindowsPlatform.cpp
	)

	set(Monkey_HAL_Windows_HDRS
		Monkey/HAL/Windows/WindowsPlatformAtomics.h
	)
	set(Monkey_HAL_Windows_SRCS

	)
elseif (APPLE)
	set(Monkey_Math_Mac_HDRS
		Monkey/Math/Mac/MacPlatformMath.h
	)
	set(Monkey_Math_Mac_SRCS

	)

	set(Monkey_Vulkan_Mac_HDRS
		Monkey/Vulkan/Mac/VulkanPlatformDefines.h
		Monkey/Vulkan/Mac/VulkanMacPlatform.h
	)
	set(Monkey_Vulkan_Mac_SRCS
		Monkey/Vulkan/Mac/VulkanMacPlatform.cpp
	)

	set(Monkey_HAL_HDRS
		${Monkey_HAL_HDRS}
		Monkey/HAL/ClangPlatformAtomics.h
	)
	set(Monkey_HAL_Mac_HDRS
		Monkey/HAL/Mac/MacPlatformAtomics.h
	)
	set(Monkey_HAL_Mac_SRCS

	)
elseif (UNIX AND NOT APPLE)
	set(Monkey_Math_Unix_HDRS
		Monkey/Math/Unix/UnixPlatformMath.h
	)
	set(Monkey_Math_Unix_SRCS

	)

	set(Monkey_Vulkan_Unix_HDRS
		Monkey/Vulkan/Unix/VulkanPlatformDefines.h
		Monkey/Vulkan/Unix/VulkanUnixPlatform.h
	)
	set(Monkey_Vulkan_Unix_SRCS
		Monkey/Vulkan/Unix/VulkanUnixPlatform.cpp
	)

	set(Monkey_HAL_HDRS
		${Monkey_HAL_HDRS}
		Monkey/HAL/ClangPlatformAtomics.h
	)
	set(Monkey_HAL_Unix_HDRS
		Monkey/HAL/Unix/UnixPlatformAtomics.h
	)
	set(Monkey_HAL_Unix_SRCS

	)
endif ()

add_library(Monkey STATIC
	${Monkey_Configuration_HDRS}
	${Monkey_Configuration_SRCS}

	${Monkey_Common_HDRS}
	${Monkey_Common_SRCS}

	${Monkey_App_HDRS}
	${Monkey_App_SRCS}

	${Monkey_Platform_GLFW_HDRS} 
	${Monkey_Platform_GLFW_SRCS}

	${Monkey_Launch_HDRS}
	${Monkey_Launch_SRCS}

	${Monkey_Application_SRCS}
	${Monkey_Application_HDRS}

	${Monkey_Application_Desktop_HDRS}
	${Monkey_Application_Desktop_SRCS}

	${Monkey_Math_HDRS}
	${Monkey_Math_SRCS}
	${Monkey_Math_Windows_HDRS}
	${Monkey_Math_Windows_SRCS}
	${Monkey_Math_Mac_HDRS}
	${Monkey_Math_Mac_SRCS}
	${Monkey_Math_Unix_HDRS}
	${Monkey_Math_Unix_SRCS}

	${Monkey_Utils_HDRS}
	${Monkey_Utils_SRCS}
	
	${Monkey_Core_HDRS}
	${Monkey_Core_SRCS}

	${Monkey_Vulkan_HDRS}
	${Monkey_Vulkan_SRCS}
	${Monkey_Vulkan_Windows_SRCS}
	${Monkey_Vulkan_Windows_HDRS}
	${Monkey_Vulkan_Mac_SRCS}
	${Monkey_Vulkan_Mac_HDRS}
	${Monkey_Vulkan_Unix_HDRS}
	${Monkey_Vulkan_Unix_SRCS}

	${Monkey_HAL_HDRS}
	${Monkey_HAL_SRCS}
	${Monkey_HAL_Windows_HDRS}
	${Monkey_HAL_Windows_SRCS}
	${Monkey_HAL_Mac_HDRS}
	${Monkey_HAL_Mac_SRCS}
	${Monkey_HAL_Unix_HDRS}
	${Monkey_HAL_Unix_SRCS}
)

source_group(Monkey FILES ${Monkey_App_HDRS} ${Monkey_App_SRCS})
source_group(Monkey\\Core FILES ${Monkey_Core_HDRS} ${Monkey_Core_SRCS})

source_group(Monkey\\Vulkan FILES ${Monkey_Vulkan_HDRS} ${Monkey_Vulkan_SRCS})
source_group(Monkey\\Vulkan\\Windows FILES ${Monkey_Vulkan_Windows_SRCS} ${Monkey_Vulkan_Windows_HDRS})
source_group(Monkey\\Vulkan\\Mac FILES ${Monkey_Vulkan_Mac_SRCS} ${Monkey_Vulkan_Mac_HDRS})
source_group(Monkey\\Vulkan\\Unix FILES ${Monkey_Vulkan_Unix_SRCS} ${Monkey_Vulkan_Unix_HDRS})

source_group(Monkey\\Utils FILES ${Monkey_Utils_HDRS} ${Monkey_Utils_SRCS})

source_group(Monkey\\Math FILES ${Monkey_Math_HDRS} ${Monkey_Math_SRCS})
source_group(Monkey\\Math\\Windows FILES ${Monkey_Math_Windows_HDRS} ${Monkey_Math_Windows_SRCS})
source_group(Monkey\\Math\\Mac FILES ${Monkey_Math_Mac_HDRS} ${Monkey_Math_Mac_SRCS})
source_group(Monkey\\Math\\Unix FILES ${Monkey_Math_Unix_HDRS} ${Monkey_Math_Unix_SRCS})

source_group(Monkey\\Common FILES ${Monkey_Common_HDRS} ${Monkey_Common_SRCS})
source_group(Monkey\\Platform\\glfw FILES ${Monkey_Platform_GLFW_HDRS} ${Monkey_Platform_GLFW_SRCS})
source_group(Monkey\\Configuration\\ FILES ${Monkey_Configuration_HDRS} ${Monkey_Configuration_SRCS})
source_group(Monkey\\Launch\\ FILES ${Monkey_Launch_HDRS} ${Monkey_Launch_SRCS})

source_group(Monkey\\Application\\ FILES ${Monkey_Application_HDRS} ${Monkey_Application_SRCS})
source_group(Monkey\\Application\\Desktop FILES ${Monkey_Application_Desktop_HDRS} ${Monkey_Application_Desktop_SRCS})

source_group(Monkey\\HAL FILES ${Monkey_HAL_HDRS} ${Monkey_HAL_SRCS})
source_group(Monkey\\HAL\\Windows FILES ${Monkey_HAL_Windows_HDRS} ${Monkey_HAL_Windows_SRCS})
source_group(Monkey\\HAL\\Mac FILES ${Monkey_HAL_Mac_HDRS} ${Monkey_HAL_Mac_SRCS})
source_group(Monkey\\HAL\\Unix FILES ${Monkey_HAL_Unix_SRCS} ${Monkey_HAL_Unix_HDRS})
