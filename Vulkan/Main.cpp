#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

#include <iostream>

#include "VulkanRenderer.h"


GLFWwindow* window;
VulkanRenderer renderer;
void InitWindow(std::string name = "Test window", const int width = 600, const int height = 600)
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

	float angle = 0.0f;
	float deltaTime = 0.0f;
	float lastTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		float now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		angle += 10.0f * deltaTime;
		if (angle > 360.0f) { angle -= 360.0f; }

		glm::mat4 firstModel(1.0f);
		glm::mat4 secondModel(1.0f);

		firstModel = glm::translate(firstModel, glm::vec3(0.0f, 0.0f, -2.5f));
		firstModel = glm::rotate(firstModel, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));

		secondModel = glm::translate(secondModel, glm::vec3(0.0f, 0.0f, -3.0f));
		secondModel = glm::rotate(secondModel, glm::radians(-angle * 10), glm::vec3(0.0f, 0.0f, 1.0f));


		renderer.UpdateModel(firstModel, 0);
	    renderer.UpdateModel(secondModel, 1);

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

