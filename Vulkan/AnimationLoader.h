#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <filesystem>
#include <random>
#include "VulkanRenderer.h"
#include "Mesh.h"

class VulkanRenderer;

class AnimationLoader
{
private:
	std::filesystem::path m_path;
	VulkanRenderer* renderer;
	std::mt19937 mtRand;
	std::random_device randomDevice;
	std::uniform_real_distribution<float> distribution;
	long long rangeMin, rangeMax;

public:
	AnimationLoader(const std::string& path, VulkanRenderer* renderer);
	AnimationLoader(const std::string& path, unsigned int rangedMin, unsigned int rangeMax, VulkanRenderer* renderer);
	std::vector<Mesh> Load();
};
