#include "GLFWApplication.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>

#include "Sample.h"

NS_MONKEY_BEGIN

GLFWApplication::~GLFWApplication()
{

}

bool GLFWApplication::InitWindow()
{
	if (glfwInit() != GLFW_TRUE) {
		printf("Failed init glfw.\n");
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
	if (m_Window == nullptr) {
		printf("Failed glfwCreateWindow.\n");
		return false;
	}

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	for (int i = 0; i < glfwExtensionCount; ++i) {
		AddInstanceExtension(std::string(glfwExtensionNames[i]));
	}
	
    AddPhysicalDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	return true;
}

bool GLFWApplication::InitVulkanSurface()
{
    if (glfwCreateWindowSurface(m_VkInstance, m_Window, nullptr, &m_VKSurface) != VK_SUCCESS) {
        printf("Failed to create window surface.\n");
        return false;
    }
    return true;
}

void GLFWApplication::OnLoop()
{
	while (!glfwWindowShouldClose(m_Window)) {
		glfwPollEvents();
		OnUpdate();
	}
}

void GLFWApplication::OnUpdate()
{
	m_Sample->OnUpdate();
	m_Sample->OnRender();
}

void GLFWApplication::OnDestory()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void GLFWApplication::OnKeyDown(int key)
{

}

void GLFWApplication::OnKeyUP(int key)
{

}

void GLFWApplication::OnMouseMove(int x, int y)
{

}

void GLFWApplication::OnMouseDown(int x, int y, int type)
{

}

void GLFWApplication::OnMouseUP(int x, int y, int type)
{

}

NS_MONKEY_END
