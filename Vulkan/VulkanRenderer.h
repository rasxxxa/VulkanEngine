#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <vector>
#include <algorithm>
#include <array>

#include <iostream>
#include <set>
#include "Utilites.h"
#include "Mesh.h"


class VulkanRenderer
{
private:
	// Main
	struct
	{
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} mainDevice;

	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;

	// Scene objects
	std::vector<Mesh> meshList;

	std::vector<SwapChainImage> swapChainImages;
	std::vector<VkFramebuffer> swapChainFrameBuffers;
	std::vector<VkCommandBuffer> commandBuffers;

	// Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkCommandPool graphicsCommandPool;

	GLFWwindow* window;
	int currentFrame = 0;
	// Vulkan components
	VkInstance instance;
	
	// Debug messenger
	VkDebugUtilsMessengerEXT debugMessenger;

	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;

	// vulkan functions
	void CreateInstance();

	void CreateLogicalDevice();

	void SetupDebugMessenger();

	void CreateSwapChain();

	void CreateFramebuffers();

	void CreateCommandPool();

	void CreateCommandBuffers();

	// Support functions
	bool CheckInstanceExtensionSupport(std::vector<const char*>* checkExtensions);

	bool CheckValidationLayerSupport();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	// Checkers functions
	void GetPhysicalDevice();
	bool CheckDeviceSuitable(VkPhysicalDevice device);

	// Getter functions
	QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device);
	void CreateSurface();
	SwapChainDetails GetSwapChainDetails(VkPhysicalDevice device);

	// Record functions

	void RecordCommands();

	//  Choose functions

	VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
    
	// Create functions
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	void CreateSynchronisation();


	// PIPELINE
	VkPipelineLayout pipelineLayout;

	// - Sync
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;
	
public:
	VulkanRenderer();
	virtual ~VulkanRenderer();
	int Init(GLFWwindow* newWindow);
	void CleanUp();
	void Draw();

};

