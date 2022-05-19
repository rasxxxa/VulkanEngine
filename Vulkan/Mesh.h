#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include "Utilites.h"

struct UniformBufferObjectModel
{
	glm::mat4 m_model;
};

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

	void SetModel(glm::mat4 model) { this->model.m_model = model; };
	UniformBufferObjectModel GetModel() { return model; };

private:

	UniformBufferObjectModel model;

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

