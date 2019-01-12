#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main()
{
	if (glfwInit() != GLFW_TRUE) {
		printf("GLFW init error.\n");
		return 1;
	}
	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(800, 600, "1 HelloWindow", nullptr, nullptr);

	if (window == nullptr) {
		printf("GLFW create window error.\n");
		return 1;
	}

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	printf("Vulkan extensions supported:%ud\n", extensionCount);

	glm::mat4 matrix4;
	glm::vec4 vec4;
	auto test = matrix4 * vec4;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}