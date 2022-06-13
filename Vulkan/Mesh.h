#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include "Utilites.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

struct Model
{
	glm::mat4 m_model;
};

class Mesh
{
public:
	Mesh(VkPhysicalDevice device, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> * vertices, std::vector<uint32_t>* indices, int texId);
	Mesh() {};
	~Mesh() {};
	int GetVertexCount();

	VkBuffer GetVertexBuffer();
	void DestroyBuffer();
	void SetMeshSize(const std::pair<float, float>& size);
	void SetMeshPosition(const std::pair<float, float>& position);

	int GetIndexCount() { return indexCount; };
	VkBuffer GetIndexBuffer() { return indexBuffer; };

	void SetModel(glm::mat4 model) { this->model.m_model = model; };
	Model& GetModel() { return model; };
	inline int GetTexId() const { return textureId; }

private:
	Model model;
	float posX, posY;
	float width, height;
	int textureId;
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

