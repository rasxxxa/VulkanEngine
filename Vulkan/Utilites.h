#pragma once

#include <fstream>
#include <glm/glm.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const int MAX_FRAME_DRAWS = 2;

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// indices (location) of queue families (if they exists)
struct QueueFamilyIndices
{
	int graphicsFamily = -1; // locations of graphics queue family
	int presentationFamily = -1; // location of presentation queue family
	// check if queue families are valid
	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;     // Surface properties e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;          // Surface image formats e.g. RGBA
	std::vector<VkPresentModeKHR> presentationModes;  // How images should be presented to screen
};


struct SwapChainImage
{
	VkImage image;
	VkImageView imageView;
};


static std::vector<char> readFile(const std::string& filename)
{
	// Open stream from given file 
	// std::ios::binary - read as binary
	// std::ios::ate - read from end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	// Check if file stream successfully opened

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open a file");
	}

	// Get current read position and used to resize a buffer
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> fileBuffer(fileSize);

	// Seek for 0 position
	file.seekg(0);
	// read the file data into buffer in total
	file.read(fileBuffer.data(), fileSize);

	// Close file
	file.close();

	return fileBuffer;
}


struct Vertex
{
	glm::vec3 m_position;
	glm::vec3 m_color;
};


static uint32_t FindMemoryTypeIndex(uint32_t allowedTypes, VkMemoryPropertyFlags propertyFlags, VkPhysicalDevice device)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(device, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i))
			&& ((memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
			)       // Index of memory type must match corresponding bit in allowedTypes
		{
			return i;
		}
	}

	throw std::runtime_error("No memory type index");

}

static void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory * bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = bufferUsage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Similar to swap chain images, can share vertex buffers

	VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, buffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Vertex Buffer!");
	}

	// Get buffer memory requirements
	VkMemoryRequirements memoryRequirements = {};
	vkGetBufferMemoryRequirements(device, *buffer, &memoryRequirements);

	// ALLOCATE MEMORY TO BUFFER
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = FindMemoryTypeIndex(memoryRequirements.memoryTypeBits, bufferProperties, physicalDevice);		// Index of memory type on physical device that has required bit flags
																																		// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT cpu can interact with memory
																																		// VK_MEMORY_PROPERTY_HOST_COHERENT_BIT allows placement of data straight into buffer after mapping
	// Allocate memory to vkDeviceMemory
	result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, bufferMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Cannot allocate vertex buffer memory!");
	}

	// Allocate memory to given vertex buffer
	vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

static void CopyBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
{
	// Command buffer to hold transfer commands
	VkCommandBuffer transferCommandBuffer;

	// Command buffer details
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = transferCommandPool;
	allocInfo.commandBufferCount = 1;

	// Allocate command buffer from pool
	vkAllocateCommandBuffers(device, &allocInfo, &transferCommandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	//Begin recording transfer commands
	vkBeginCommandBuffer(transferCommandBuffer, &beginInfo);

	// Region of data to copy from and to
	VkBufferCopy bufferCopyRegion = {};
	bufferCopyRegion.srcOffset = 0;
	bufferCopyRegion.dstOffset = 0;
	bufferCopyRegion.size = bufferSize;

	// Command to copy src buffer to dst buffer
	vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

	vkEndCommandBuffer(transferCommandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &transferCommandBuffer;

	// Submit transfer command to transfre queue and wait until it finishes
	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);

	// Free temporary command buffer back to pool
	vkFreeCommandBuffers(device, transferCommandPool, 1, &transferCommandBuffer);
}