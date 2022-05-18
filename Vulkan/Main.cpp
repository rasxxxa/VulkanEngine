#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

#include <iostream>

#include "VulkanRenderer.h"

GLFWwindow* window;
VulkanRenderer renderer;
void InitWindow(std::string name = "Test window", const int width = 1920, const int height = 1080)
{
	// Initialize GLFW
	glfwInit();

	// Set glfw to not work with opengl
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// Set to be resizible
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

}

void RunWindow()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		renderer.Draw();
	}
}

void Clear()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}


int main()
{
	InitWindow();

	// Create renderer
	if (renderer.Init(window) == EXIT_FAILURE)
		return EXIT_FAILURE;

	RunWindow();
	renderer.CleanUp();
	Clear();
	return 0;
}

