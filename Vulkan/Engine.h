#pragma once
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>

#include <iostream>

#include "VulkanRenderer.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <string>
#include "AnimationLoader.h"
#include <memory>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

class Engine
{
private:
	
	ImGui_ImplVulkanH_Window m_MainWindowData;
	void InitImGui();


	GLFWwindow* m_window;
	bool CreateWindow(const int width, const int height);
	bool DestroyWindow();
	void RunWindow();

	std::unique_ptr<VulkanRenderer> m_renderer;
	void CreateRenderer();

	void ShutdownApplication();
	Engine() = default;

public:
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine& engine) = delete;
	Engine& operator=(const Engine&& engine) = delete;
	void InitProgram(int width = 800, int height = 600);

	static Engine& GetInstance();
};

