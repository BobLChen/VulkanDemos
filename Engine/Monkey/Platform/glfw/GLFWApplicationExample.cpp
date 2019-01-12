#include "GLFWApplicationExample.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>

#include "Sample.h"

GLFWApplicationExample::~GLFWApplicationExample()
{
    
}

bool GLFWApplicationExample::InitWindow()
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

bool GLFWApplicationExample::InitVulkanSurface()
{
    if (glfwCreateWindowSurface(m_VkInstance, m_Window, nullptr, &m_VKSurface) != VK_SUCCESS) {
        printf("Failed to create window surface.\n");
        return false;
    }
    return true;
}

void GLFWApplicationExample::OnLoop()
{
	while (!glfwWindowShouldClose(m_Window)) {
		glfwPollEvents();
		OnUpdate();
	}
}

void GLFWApplicationExample::OnUpdate()
{
	m_Sample->OnUpdate();
	m_Sample->OnRender();
}

void GLFWApplicationExample::OnDestory()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void GLFWApplicationExample::OnKeyDown(int key)
{

}

void GLFWApplicationExample::OnKeyUP(int key)
{

}

void GLFWApplicationExample::OnMouseMove(int x, int y)
{

}

void GLFWApplicationExample::OnMouseDown(int x, int y, int type)
{

}

void GLFWApplicationExample::OnMouseUP(int x, int y, int type)
{
    
}

