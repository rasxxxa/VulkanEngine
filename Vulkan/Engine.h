#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include "VulkanRenderer.h"
#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <string>
#include "AnimationLoader.h"
#include <memory>
#include <condition_variable>
#include <atomic>
#include <unordered_map>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

class Engine
{
private:
	friend class VulkanRenderer;
	friend class Mesh;
	ImGui_ImplVulkanH_Window m_MainWindowData;
	void InitImGui();
	void RunInThread(int width, int height);

	GLFWwindow* m_window;
	bool CreateWindow(const int width, const int height);
	bool DestroyWindow();
	void RunWindow();

	std::unique_ptr<VulkanRenderer> m_renderer;
	void CreateRenderer();
	void ShutdownApplication();
	Engine() = default;
	~Engine() 
	{
	}

	std::thread threadRender;
	std::mutex mtx;
	std::atomic_bool shouldEnd = false;

	static std::unordered_map<unsigned long, std::weak_ptr<Mesh>> m_meshes;
	static unsigned long objectCreated;
	int GetTextureId(const std::string& path);
	VkQueue GetTransferQueue();
	VkCommandPool GetCommandPool();
public:
	Engine(const Engine&) = delete;
	Engine(Engine&&) = delete;
	Engine& operator=(const Engine& engine) = delete;
	Engine& operator=(const Engine&& engine) = delete;
	void InitProgram(int width = 800, int height = 600);
	static Engine& GetInstance();
	std::shared_ptr<Mesh> CreateMash();
};

