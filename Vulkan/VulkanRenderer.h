#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <vector>
#include <algorithm>
#include <array>
#include <random>
#include "stb_image.h"

#include <iostream>
#include <set>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "Utilites.h"

#include "Mesh.h"
#include "AnimationLoader.h"
#include "Utilites.h"
#include <unordered_map>
#include <assert.h>
#include "Engine.h"

class VulkanRenderer
{
private:
	// Main
	struct
	{
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	} mainDevice;

	friend class AnimationLoader;
	friend class Engine;
	long long unsigned int memoryUsed;

	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;

	//VkDeviceSize minUniformBUfferOffset;
	//size_t modelUniformAlignment;


	// Scene settings
	struct ViewProjection
	{
		glm::mat4 m_projection;
		glm::mat4 m_view;
	} modelviewprojection;

	std::vector<SwapChainImage> swapChainImages;
	std::vector<VkFramebuffer> swapChainFrameBuffers;
	std::vector<VkCommandBuffer> commandBuffers;


	VkImage depthBufferImage;
	VkDeviceMemory depthBufferImageMemory;
	VkImageView depthBufferImageView;
	VkFormat depthFormat;

	// Descriptors
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSetLayout samplerSetLayout;
	VkPushConstantRange pushConstantRange;

	VkDescriptorPool descriptorPool;
	VkDescriptorPool samplerDescriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
	std::vector<VkDescriptorSet> samplerDescriptorSets;

	std::vector<VkBuffer> uniformBuffer;
	std::vector<VkDeviceMemory> uniformBufferMemory;

	std::vector<VkBuffer> modelUniformBuffer;
	std::vector<VkDeviceMemory> modelUniformBufferMemory;



	//Model* modelTransferSpace;

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

	void CreateDepthBufferImage();

	void CreateFramebuffers();

	void CreateCommandPool();

	void CreateCommandBuffers();

	// Support functions
	VkFormat ChooseSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);
	bool CheckInstanceExtensionSupport(std::vector<const char*>* checkExtensions);

	bool CheckValidationLayerSupport();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

	// Checkers functions
	void GetPhysicalDevice();


	void AllocateDynamicBuffer();

	bool CheckDeviceSuitable(VkPhysicalDevice device);

	// Getter functions
	QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device);
	void CreateSurface();
	SwapChainDetails GetSwapChainDetails(VkPhysicalDevice device);

	// Record functions

	void RecordCommands(uint32_t currentImage);

	//  Choose functions

	VkSurfaceFormatKHR ChooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR ChooseBestPresentationMode(const std::vector<VkPresentModeKHR>& presentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
    
	// Create functions
	VkImage CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceMemory* imageMemory);
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& code);
	void CreateSynchronisation();
	void CreateDescriptorSetLayout();
	void CreatePushConstantRange();

	void CreateUniformBuffers();
	void CreateDescriptorPool();
	void CreateDescriptorSets();

	int CreateTextureImage(std::string fileName);
	void CreateTextureSampler();
	int CreateTextureDescriptor(VkImageView textureImage);


	void UpdateUniformBuffer(uint32_t imageIndex);

	// Assets
	VkSampler sampler;
	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;
	std::vector<VkImageView> textureImageViews;

	// PIPELINE
	VkPipelineLayout pipelineLayout;

	// - Sync
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;

	// Loader function
	stbi_uc* LoadTextureFile(std::string fileName, int* width, int* height, VkDeviceSize* imageSize);

	std::mt19937 mt;
	std::uniform_real_distribution<float> distribution;
	std::uniform_real_distribution<float> colorDistribution;
	ImGui_ImplVulkanH_Window* wd;
public:
	int CreateTexture(std::string fileName);
	VulkanRenderer();
	virtual ~VulkanRenderer();
	int Init(GLFWwindow* newWindow);
	void CleanUp();
	void Draw();
	void SetupImgui(ImGui_ImplVulkanH_Window* wd);
	void InitForVulkan();
	VkDevice GetLogicalDevice() { return mainDevice.logicalDevice; }
	VkQueue GetGraphicsQueue() { return graphicsQueue; };
	inline long long unsigned int GetDeviceMemory() const { return memoryUsed; }
	static std::unordered_map<std::string, int> imagesID;
};

