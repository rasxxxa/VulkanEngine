#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include "Utilites.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <memory>
#include "Engine.h"

struct Model
{
	glm::mat4 m_model;
};

class Mesh : public std::enable_shared_from_this<Mesh>
{
public:
	Mesh(VkPhysicalDevice device, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex> * vertices, std::vector<uint32_t>* indices, int texId);
	Mesh() {};
	~Mesh() {};
	int GetVertexCount();

	VkBuffer GetVertexBuffer() const;
	void DestroyBuffer();
	void SetMeshSize(const std::pair<float, float>& size);
	void SetMeshPosition(const std::pair<float, float>& position);

	int GetIndexCount() { return indexCount; };
	VkBuffer GetIndexBuffer() const { return indexBuffer; };

	void SetModel(glm::mat4 model) { this->model.m_model = model; };
	Model& GetModel() { return model; };
	inline int GetTexId() const { return textureId; }
	void AddVisual(const std::shared_ptr<Mesh>& visual);
	void SetTexture(const std::string& texturePath);

private:
	std::weak_ptr<Mesh> m_parent;
	std::vector<std::weak_ptr<Mesh>> m_children;
	bool m_visible;
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

