#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include "Utilites.h"

class Mesh
{
public:
	Mesh(VkPhysicalDevice device, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> * vertices, std::vector<uint32_t>* indices);
	Mesh() {};
	~Mesh() {};
	int GetVertexCount();

	VkBuffer GetVertexBuffer();
	void DestroyBuffer();

	int GetIndexCount() { return indexCount; };
	VkBuffer GetIndexBuffer() { return indexBuffer; };

private:
	int vertexCount;
	VkBuffer vertexBuffer;
	VkPhysicalDevice physicalDevice;
	VkDeviceMemory vertexBufferMemory;
	VkDevice device;
	int indexCount;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	void CreateVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
	void CreateIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);
};

