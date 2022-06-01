#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <vector>
#include <algorithm>
#include <array>
#include "stb_image.h"

#include <iostream>
#include <set>
#include "Utilites.h"
#include "Mesh.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

class VulkanRenderer
{
private:

	// IMGUI
	ImGui_ImplVulkanH_Window* imgWindow;

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

	//VkDeviceSize minUniformBUfferOffset;
	//size_t modelUniformAlignment;

	// Scene objects
	std::vector<Mesh> meshList;


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
	int CreateTexture(std::string fileName);
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


public:
	VulkanRenderer();
	virtual ~VulkanRenderer();
	int Init(ImGui_ImplVulkanH_Window* wd, GLFWwindow* newWindow);
	void CleanUp();
	void UpdateModel(glm::mat4 newModel, int modelId);
	void Draw();
	void CreateImgGui();
	void ImgVulkanInitInfo(ImGui_ImplVulkan_InitInfo& init_info);
	VkDevice GetLogicalDevice() { return mainDevice.logicalDevice; };
	VkPhysicalDevice GetPhysicalDeviceM() { return mainDevice.physicalDevice; };
};

