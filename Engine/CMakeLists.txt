# demo
set(Monkey_Demo_HDRS
	Monkey/Demo/DVKUtils.h
	Monkey/Demo/DemoBase.h
	Monkey/Demo/DVKBuffer.h
	Monkey/Demo/DVKCommand.h
	Monkey/Demo/DVKVertexBuffer.h
	Monkey/Demo/DVKIndexBuffer.h
	Monkey/Demo/DVKModel.h
	Monkey/Demo/DVKCommon.h
	Monkey/Demo/DVKPipeline.h
	Monkey/Demo/DVKTexture.h
	Monkey/Demo/DVKShader.h
	Monkey/Demo/DVKMaterial.h
	Monkey/Demo/DVKDefaultRes.h
	Monkey/Demo/DVKRenderTarget.h
	Monkey/Demo/DVKCamera.h
	Monkey/Demo/DVKCompute.h
	Monkey/Demo/FileManager.h
	Monkey/Demo/ImageGUIContext.h
)
set(Monkey_Demo_SRCS
	Monkey/Demo/DemoBase.cpp
	Monkey/Demo/DVKBuffer.cpp
	Monkey/Demo/DVKCommand.cpp
	Monkey/Demo/DVKVertexBuffer.cpp
	Monkey/Demo/DVKIndexBuffer.cpp
	Monkey/Demo/DVKModel.cpp
	Monkey/Demo/DVKPipeline.cpp
	Monkey/Demo/DVKTexture.cpp
	Monkey/Demo/DVKShader.cpp
	Monkey/Demo/DVKMaterial.cpp
	Monkey/Demo/DVKDefaultRes.cpp
	Monkey/Demo/DVKRenderTarget.cpp
	Monkey/Demo/DVKCamera.cpp
	Monkey/Demo/DVKCompute.cpp
	Monkey/Demo/FileManager.cpp
	Monkey/Demo/ImageGUIContext.cpp
)

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

set(Monkey_UI_HDRS
	
)
set(Monkey_UI_SRCS

)

set(Monkey_App_HDRS
	Monkey/Engine.h
)
set(Monkey_App_SRCS
	Monkey/Engine.cpp
)

set(Monkey_Launch_HDRS
	Monkey/Launch/Launch.h
)
set(Monkey_Launch_SRCS
	Monkey/Launch/Launch.cpp
)

set(Monkey_Application_HDRS
	Monkey/Application/GenericWindow.h
	Monkey/Application/GenericApplication.h
	Monkey/Application/GenericApplicationMessageHandler.h
	Monkey/Application/Application.h
	Monkey/Application/AppModuleBase.h
)
set(Monkey_Application_SRCS
	Monkey/Application/GenericWindow.cpp
	Monkey/Application/GenericApplication.cpp
	Monkey/Application/Application.cpp
)

set(Monkey_Math_HDRS
	Monkey/Math/Math.h
	Monkey/Math/Vector2.h
	Monkey/Math/IntPoint.h
	Monkey/Math/GenericPlatformMath.h
	Monkey/Math/PlatformMath.h
	Monkey/Math/Color.h
	Monkey/Math/Vector3.h
	Monkey/Math/IntVector.h
	Monkey/Math/Vector4.h
	Monkey/Math/Axis.h
	Monkey/Math/Plane.h
	Monkey/Math/Quat.h
	Monkey/Math/Rotator.h
	Monkey/Math/Matrix4x4.h
)
set(Monkey_Math_SRCS
	Monkey/Math/Math.cpp
	Monkey/Math/GenericPlatformMath.cpp
	Monkey/Math/Color.cpp
)

set(Monkey_Utils_SRCS
	Monkey/Utils/StringUtils.h
	Monkey/Utils/Alignment.h
	Monkey/Utils/SecureHash.h
	Monkey/Utils/Crc.h
)
set(Monkey_Utils_HDRS
	Monkey/Utils/SecureHash.cpp
	Monkey/Utils/Crc.cpp
)

set(Monkey_File_SRCS
)
set(Monkey_File_HDRS
)

set(Monkey_Core_HDRS
	Monkey/Core/PixelFormat.h
)
set(Monkey_Core_SRCS
	Monkey/Core/PixelFormat.cpp
)

set(Monkey_Vulkan_SRCS
	Monkey/Vulkan/VulkanRHI.cpp
	Monkey/Vulkan/VulkanLayers.cpp
	Monkey/Vulkan/VulkanQueue.cpp
	Monkey/Vulkan/VulkanDevice.cpp
	Monkey/Vulkan/VulkanDebug.cpp
	Monkey/Vulkan/VulkanSwapChain.cpp
	Monkey/Vulkan/VulkanMemory.cpp
	Monkey/Vulkan/VulkanFence.cpp
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
	Monkey/Vulkan/VulkanFence.h
)

set(Monkey_Loader_HDRS
	Monkey/Loader/stb_image.h
	Monkey/Loader/stb_image_resize.h
	Monkey/Loader/stb_image_write.h
	Monkey/Loader/ImageLoader.h
)
set(Monkey_Loader_SRCS
	Monkey/Loader/ImageLoader.cpp
)

set(Monkey_HAL_HDRS
	Monkey/HAL/PlatformAtomics.h
	Monkey/HAL/GenericPlatformAtomics.h
	Monkey/HAL/ThreadSafeCounter.h
)
set(Monkey_HAL_SRCS
	
)

set(Monkey_GenericPlatform_HDRS
	Monkey/GenericPlatform/GenericPlatformTime.h
	Monkey/GenericPlatform/InputManager.h
	Monkey/GenericPlatform/KeyboardTypes.h
	Monkey/GenericPlatform/MouseTypes.h
)
set(Monkey_GenericPlatform_SRCS
	Monkey/GenericPlatform/InputManager.cpp
)

if (WIN32)
	set(Monkey_Application_Windows_HDRS
		Monkey/Application/Windows/WinWindow.h
		Monkey/Application/Windows/WinApplication.h
	)
	set(Monkey_Application_Windows_SRCS
		Monkey/Application/Windows/WinWindow.cpp
		Monkey/Application/Windows/WinApplication.cpp
	)

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

	set(Monkey_GenericPlatform_Windows_HDRS
		Monkey/GenericPlatform/Windows/WindowsPlatformTime.h
	)
	set(Monkey_GenericPlatform_Windows_SRCS
		Monkey/GenericPlatform/Windows/WindowsPlatformTime.cpp
		Monkey/GenericPlatform/Windows/InputManagerWindows.cpp
	)
elseif (IOS)
	set(Monkey_Application_IOS_HDRS
		Monkey/Application/IOS/IOSWindow.h
		Monkey/Application/IOS/IOSApplication.h
		Monkey/Application/IOS/IOSViewController.h
		Monkey/Application/IOS/IOSAppDelegate.h
	)
	set(Monkey_Application_IOS_SRCS
		Monkey/Application/IOS/IOSWindow.cpp
		Monkey/Application/IOS/IOSApplication.cpp
		Monkey/Application/IOS/IOSViewController.cpp
		Monkey/Application/IOS/IOSAppDelegate.cpp
	)
	set(IOS_Objective_CPP
		${Monkey_Application_IOS_HDRS}
		${Monkey_Application_IOS_SRCS}
	)

	set_source_files_properties(
		${IOS_Objective_CPP}
		PROPERTIES 
		COMPILE_FLAGS "-x objective-c++"
	)

	set(Monkey_Math_IOS_HDRS
		Monkey/Math/IOS/IOSPlatformMath.h
	)
	set(Monkey_Math_IOS_SRCS

	)

	set(Monkey_Vulkan_IOS_HDRS
		Monkey/Vulkan/IOS/VulkanPlatformDefines.h
		Monkey/Vulkan/IOS/VulkanIOSPlatform.h
	)
	set(Monkey_Vulkan_IOS_SRCS
		Monkey/Vulkan/IOS/VulkanIOSPlatform.cpp
	)

	set(Monkey_HAL_IOS_HDRS
		Monkey/HAL/IOS/IOSPlatformAtomics.h
	)
	set(Monkey_HAL_IOS_SRCS

	)

	set(Monkey_GenericPlatform_IOS_HDRS
		Monkey/GenericPlatform/IOS/IOSPlatformTime.h
	)
	set(Monkey_GenericPlatform_IOS_SRCS
		Monkey/GenericPlatform/IOS/IOSPlatformTime.cpp
		Monkey/GenericPlatform/IOS/InputManagerIOS.cpp
	)
elseif (APPLE)
	set(Monkey_Application_Mac_HDRS
		Monkey/Application/Mac/MacWindow.h
		Monkey/Application/Mac/MacApplication.h
		Monkey/Application/Mac/CocoaWindow.h
	)
	set(Monkey_Application_Mac_SRCS
		Monkey/Application/Mac/MacWindow.cpp
		Monkey/Application/Mac/MacApplication.cpp
		Monkey/Application/Mac/CocoaWindow.cpp
	)
	set(Mac_Objective_CPP
		${Monkey_Application_Mac_HDRS}
		${Monkey_Application_Mac_SRCS}
	)

	set_source_files_properties(
		${Mac_Objective_CPP}
		PROPERTIES 
        COMPILE_FLAGS "-x objective-c++"
	)

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

	set(Monkey_GenericPlatform_Mac_HDRS
		Monkey/GenericPlatform/Mac/MacPlatformTime.h
	)
	set(Monkey_GenericPlatform_Mac_SRCS
		Monkey/GenericPlatform/Mac/MacPlatformTime.cpp
		Monkey/GenericPlatform/Mac/InputManagerMac.cpp
	)
elseif (ANDROID)
	set(Monkey_Application_Android_HDRS
		Monkey/Application/Android/AndroidWindow.h
		Monkey/Application/Android/AndroidApplication.h
	)
	set(Monkey_Application_Android_SRCS
		Monkey/Application/Android/AndroidWindow.cpp
		Monkey/Application/Android/AndroidApplication.cpp
	)
							
	set(Monkey_Math_Android_HDRS
		Monkey/Math/Android/AndroidPlatformMath.h
	)
	set(Monkey_Math_Android_SRCS

	)

	set(Monkey_Vulkan_Android_HDRS
		Monkey/Vulkan/Android/VulkanPlatformDefines.h
		Monkey/Vulkan/Android/VulkanAndroidPlatform.h
	)
	set(Monkey_Vulkan_Android_SRCS
		Monkey/Vulkan/Android/VulkanAndroidPlatform.cpp
	)

	set(Monkey_HAL_Android_HDRS
		Monkey/HAL/Android/AndroidPlatformAtomics.h
	)
	set(Monkey_HAL_Android_SRCS

	)

	set(Monkey_GenericPlatform_Android_HDRS
		Monkey/GenericPlatform/Android/AndroidPlatformTime.h
	)
	set(Monkey_GenericPlatform_Android_SRCS
		Monkey/GenericPlatform/Android/AndroidPlatformTime.cpp
		Monkey/GenericPlatform/Android/InputManagerAndroid.cpp
	)
elseif (UNIX)
	set(Monkey_Application_Linux_HDRS
		Monkey/Application/Linux/LinuxWindow.h
		Monkey/Application/Linux/LinuxApplication.h
	)
	set(Monkey_Application_Linux_SRCS
		Monkey/Application/Linux/LinuxWindow.cpp
		Monkey/Application/Linux/LinuxApplication.cpp
	)
							
	set(Monkey_Math_Linux_HDRS
		Monkey/Math/Linux/LinuxPlatformMath.h
	)
	set(Monkey_Math_Linux_SRCS

	)

	set(Monkey_Vulkan_Linux_HDRS
		Monkey/Vulkan/Linux/VulkanPlatformDefines.h
		Monkey/Vulkan/Linux/VulkanLinuxPlatform.h
	)
	set(Monkey_Vulkan_Linux_SRCS
		Monkey/Vulkan/Linux/VulkanLinuxPlatform.cpp
	)

	set(Monkey_HAL_HDRS
		${Monkey_HAL_HDRS}
		Monkey/HAL/ClangPlatformAtomics.h
	)
	set(Monkey_HAL_Linux_HDRS
		Monkey/HAL/Linux/LinuxPlatformAtomics.h
	)
	set(Monkey_HAL_Linux_SRCS

	)

	set(Monkey_GenericPlatform_Linux_HDRS
		Monkey/GenericPlatform/Linux/LinuxPlatformTime.h
	)
	set(Monkey_GenericPlatform_Linux_SRCS
		Monkey/GenericPlatform/Linux/LinuxPlatformTime.cpp
		Monkey/GenericPlatform/Linux/InputManagerLinux.cpp
	)
endif ()

add_library(Monkey STATIC
	# demo
	${Monkey_Demo_HDRS}
	${Monkey_Demo_SRCS}

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
	${Monkey_Application_Windows_HDRS}
	${Monkey_Application_Windows_SRCS}
	${Monkey_Application_Linux_HDRS}
	${Monkey_Application_Linux_SRCS}
	${Monkey_Application_Mac_HDRS}
	${Monkey_Application_Mac_SRCS}
	${Monkey_Application_IOS_HDRS}
	${Monkey_Application_IOS_SRCS}
	${Monkey_Application_Android_HDRS}
	${Monkey_Application_Android_SRCS}

	${Monkey_Math_HDRS}
	${Monkey_Math_SRCS}
	${Monkey_Math_Windows_HDRS}
	${Monkey_Math_Windows_SRCS}
	${Monkey_Math_Mac_HDRS}
	${Monkey_Math_Mac_SRCS}
	${Monkey_Math_IOS_HDRS}
	${Monkey_Math_IOS_SRCS}
	${Monkey_Math_Linux_HDRS}
	${Monkey_Math_Linux_SRCS}
	${Monkey_Math_Android_HDRS}
	${Monkey_Math_Android_SRCS}

	${Monkey_Utils_HDRS}
	${Monkey_Utils_SRCS}
	${Monkey_File_HDRS}
	${Monkey_File_SRCS}
	
	${Monkey_Core_HDRS}
	${Monkey_Core_SRCS}

	${Monkey_Vulkan_HDRS}
	${Monkey_Vulkan_SRCS}
	${Monkey_Vulkan_Windows_SRCS}
	${Monkey_Vulkan_Windows_HDRS}
	${Monkey_Vulkan_Mac_SRCS}
	${Monkey_Vulkan_Mac_HDRS}
	${Monkey_Vulkan_IOS_SRCS}
	${Monkey_Vulkan_IOS_HDRS}
	${Monkey_Vulkan_Linux_HDRS}
	${Monkey_Vulkan_Linux_SRCS}
	${Monkey_Vulkan_Android_HDRS}
	${Monkey_Vulkan_Android_SRCS}

	${Monkey_HAL_HDRS}
	${Monkey_HAL_SRCS}
	${Monkey_HAL_Windows_HDRS}
	${Monkey_HAL_Windows_SRCS}
	${Monkey_HAL_Mac_HDRS}
	${Monkey_HAL_Mac_SRCS}
	${Monkey_HAL_IOS_HDRS}
	${Monkey_HAL_IOS_SRCS}
	${Monkey_HAL_Linux_HDRS}
	${Monkey_HAL_Linux_SRCS}
	${Monkey_HAL_Android_HDRS}
	${Monkey_HAL_Android_SRCS}

	${Monkey_GenericPlatform_SRCS}
	${Monkey_GenericPlatform_HDRS}
	${Monkey_GenericPlatform_Windows_HDRS}
	${Monkey_GenericPlatform_Windows_SRCS}
	${Monkey_GenericPlatform_Mac_HDRS}
	${Monkey_GenericPlatform_Mac_SRCS}
	${Monkey_GenericPlatform_Linux_HDRS}
	${Monkey_GenericPlatform_Linux_SRCS}
	${Monkey_GenericPlatform_IOS_HDRS}
	${Monkey_GenericPlatform_IOS_SRCS}
	${Monkey_GenericPlatform_Android_HDRS}
	${Monkey_GenericPlatform_Android_SRCS}

	${Monkey_Loader_HDRS}
	${Monkey_Loader_SRCS}

	${Monkey_UI_HDRS}
	${Monkey_UI_SRCS}
)

# demo
source_group(Monkey\\Demo FILES ${Monkey_Demo_HDRS} ${Monkey_Demo_SRCS})

# engine
source_group(Monkey FILES ${Monkey_App_HDRS} ${Monkey_App_SRCS})
source_group(Monkey\\Core FILES ${Monkey_Core_HDRS} ${Monkey_Core_SRCS})

source_group(Monkey\\Vulkan FILES ${Monkey_Vulkan_HDRS} ${Monkey_Vulkan_SRCS})
source_group(Monkey\\Vulkan\\Windows FILES ${Monkey_Vulkan_Windows_SRCS} ${Monkey_Vulkan_Windows_HDRS})
source_group(Monkey\\Vulkan\\Mac FILES ${Monkey_Vulkan_Mac_SRCS} ${Monkey_Vulkan_Mac_HDRS})
source_group(Monkey\\Vulkan\\IOS FILES ${Monkey_Vulkan_IOS_SRCS} ${Monkey_Vulkan_IOS_HDRS})
source_group(Monkey\\Vulkan\\Linux FILES ${Monkey_Vulkan_Linux_SRCS} ${Monkey_Vulkan_Linux_HDRS})
source_group(Monkey\\Vulkan\\Android FILES ${Monkey_Vulkan_Android_SRCS} ${Monkey_Vulkan_Android_HDRS})

source_group(Monkey\\Utils FILES ${Monkey_Utils_HDRS} ${Monkey_Utils_SRCS})
source_group(Monkey\\File FILES ${Monkey_File_HDRS} ${Monkey_File_SRCS})

source_group(Monkey\\Math FILES ${Monkey_Math_HDRS} ${Monkey_Math_SRCS})
source_group(Monkey\\Math\\Windows FILES ${Monkey_Math_Windows_HDRS} ${Monkey_Math_Windows_SRCS})
source_group(Monkey\\Math\\Mac FILES ${Monkey_Math_Mac_HDRS} ${Monkey_Math_Mac_SRCS})
source_group(Monkey\\Math\\IOS FILES ${Monkey_Math_IOS_HDRS} ${Monkey_Math_IOS_SRCS})
source_group(Monkey\\Math\\Linux FILES ${Monkey_Math_Linux_HDRS} ${Monkey_Math_Linux_SRCS})
source_group(Monkey\\Math\\Android FILES ${Monkey_Math_Android_HDRS} ${Monkey_Math_Android_SRCS})

source_group(Monkey\\Common FILES ${Monkey_Common_HDRS} ${Monkey_Common_SRCS})
source_group(Monkey\\Platform\\glfw FILES ${Monkey_Platform_GLFW_HDRS} ${Monkey_Platform_GLFW_SRCS})
source_group(Monkey\\Configuration\\ FILES ${Monkey_Configuration_HDRS} ${Monkey_Configuration_SRCS})
source_group(Monkey\\Launch\\ FILES ${Monkey_Launch_HDRS} ${Monkey_Launch_SRCS})

source_group(Monkey\\Application\\ FILES ${Monkey_Application_HDRS} ${Monkey_Application_SRCS})
source_group(Monkey\\Application\\Desktop FILES ${Monkey_Application_Desktop_HDRS} ${Monkey_Application_Desktop_SRCS})
source_group(Monkey\\Application\\Windows FILES ${Monkey_Application_Windows_HDRS} ${Monkey_Application_Windows_SRCS})
source_group(Monkey\\Application\\Linux FILES ${Monkey_Application_Linux_HDRS} ${Monkey_Application_Linux_SRCS})
source_group(Monkey\\Application\\Mac FILES ${Monkey_Application_Mac_HDRS} ${Monkey_Application_Mac_SRCS})
source_group(Monkey\\Application\\IOS FILES ${Monkey_Application_IOS_HDRS} ${Monkey_Application_IOS_SRCS})
source_group(Monkey\\Application\\Android FILES ${Monkey_Application_Android_HDRS} ${Monkey_Application_Android_SRCS})

source_group(Monkey\\HAL FILES ${Monkey_HAL_HDRS} ${Monkey_HAL_SRCS})
source_group(Monkey\\HAL\\Windows FILES ${Monkey_HAL_Windows_HDRS} ${Monkey_HAL_Windows_SRCS})
source_group(Monkey\\HAL\\Mac FILES ${Monkey_HAL_Mac_HDRS} ${Monkey_HAL_Mac_SRCS})
source_group(Monkey\\HAL\\IOS FILES ${Monkey_HAL_IOS_HDRS} ${Monkey_HAL_IOS_SRCS})
source_group(Monkey\\HAL\\Linux FILES ${Monkey_HAL_Linux_SRCS} ${Monkey_HAL_Linux_HDRS})
source_group(Monkey\\HAL\\Android FILES ${Monkey_HAL_Android_SRCS} ${Monkey_HAL_Android_HDRS})

source_group(Monkey\\GenericPlatform FILES ${Monkey_GenericPlatform_HDRS} ${Monkey_GenericPlatform_SRCS})
source_group(Monkey\\GenericPlatform\\Windows FILES ${Monkey_GenericPlatform_Windows_HDRS} ${Monkey_GenericPlatform_Windows_SRCS})
source_group(Monkey\\GenericPlatform\\Mac FILES ${Monkey_GenericPlatform_Mac_HDRS} ${Monkey_GenericPlatform_Mac_SRCS})
source_group(Monkey\\GenericPlatform\\Linux FILES ${Monkey_GenericPlatform_Linux_HDRS} ${Monkey_GenericPlatform_Linux_SRCS})
source_group(Monkey\\GenericPlatform\\IOS FILES ${Monkey_GenericPlatform_IOS_HDRS} ${Monkey_GenericPlatform_IOS_SRCS})
source_group(Monkey\\GenericPlatform\\Android FILES ${Monkey_GenericPlatform_Android_HDRS} ${Monkey_GenericPlatform_Android_SRCS})

source_group(Monkey\\Loader FILES ${Monkey_Loader_HDRS} ${Monkey_Loader_SRCS})
source_group(Monkey\\UI FILES ${Monkey_UI_HDRS} ${Monkey_UI_SRCS})

source_group(Monkey\\Graphics\\Data FILES ${Monkey_Graphics_Data_HDRS} ${Monkey_Graphics_Data_SRCS})
source_group(Monkey\\Graphics\\Shader FILES ${Monkey_Graphics_Shader_HDRS} ${Monkey_Graphics_Shader_SRCS})
source_group(Monkey\\Graphics\\Material FILES ${Monkey_Graphics_Material_SRCS} ${Monkey_Graphics_Material_HDRS})
source_group(Monkey\\Graphics\\Renderer FILES ${Monkey_Graphics_Renderer_HDRS} ${Monkey_Graphics_Renderer_SRCS})
source_group(Monkey\\Graphics\\Texture FILES ${Monkey_Graphics_Texture_HDRS} ${Monkey_Graphics_Texture_SRCS})
source_group(Monkey\\Graphics\\Command FILES ${Monkey_Graphics_Command_HDRS} ${Monkey_Graphics_Command_SRCS})