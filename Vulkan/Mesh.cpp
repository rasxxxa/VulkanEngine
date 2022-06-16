#include "Mesh.h"
#include <thread>
#include <stdio.h>
#include <string.h>

Mesh::Mesh(VkPhysicalDevice device, VkDevice newDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, int texId = -1)
{
	indexCount = indices->size();
	vertexCount = vertices->size();
	physicalDevice = device;
	this->device = newDevice;
	CreateVertexBuffer(transferQueue, transferCommandPool, vertices);
	CreateIndexBuffer(transferQueue, transferCommandPool, indices);
	model.m_model = glm::mat4(1.0f);
	posX = vertices[0][0].m_position.x;
	posY = vertices[0][0].m_position.y;
	width = vertices[0][3].m_position.x - vertices[0][0].m_position.x;
	height = vertices[0][0].m_position.y - vertices[0][1].m_position.y;
	textureId = texId;
}

int Mesh::GetVertexCount()
{
	return vertexCount;
}



VkBuffer Mesh::GetVertexBuffer() const
{
	return vertexBuffer;
}

void Mesh::DestroyBuffer()
{
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);

	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
}

// width, height
void Mesh::SetMeshSize(const std::pair<float, float>& size)
{
	width = size.first / width;
	height = size.second / height;
	model.m_model = glm::scale(model.m_model, glm::vec3(width,height, 1.0f));
	width = size.first;
	height = size.second;

}

void Mesh::SetMeshPosition(const std::pair<float, float>& position)
{
	posY = position.second - posY;
	posX = position.first - posX;
	model.m_model = glm::translate(model.m_model, glm::vec3(posX, posY, 0.0f));
	posX = position.first;
	posY = position.second;
}

void Mesh::AddVisual(const std::shared_ptr<Mesh>& visual)
{
	visual->m_parent = weak_from_this();
	m_children.push_back(visual);
}

void Mesh::SetTexture(const std::string& texturePath)
{
	DestroyBuffer();

	std::vector<Vertex> meshVertices =
	{
			{ { posX, posY, 1.0f },{ 0.0f, 0.0f, 0.0f }, {0.0f, 0.0f}, 1.0f},
			{ { posX, posY - height, 1.0f },{ 0.0f, 0.0f, 0.0f }, {0.0f, 1.0f}, 1.0f},
			{ { posX + width, posY - height, 1.0f },{ 0.0f, 0.0f, 0.0f }, {1.0f, 1.0f}, 1.0f },
			{ { posX + width, posY, 1.0f },{ 0.0f, 0.0f, 0.0f }, {1.0f, 0.0f}, 1.0f  },
	};

	int texId = Engine::GetInstance().GetTextureId(texturePath);
	this->textureId = texId;

	CreateVertexBuffer(Engine::GetInstance().GetTransferQueue(), Engine::GetInstance().GetCommandPool(), &meshVertices);
	CreateIndexBuffer(Engine::GetInstance().GetTransferQueue(), Engine::GetInstance().GetCommandPool(), &MESH_INDICES);
}

void Mesh::CreateVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices)
{
	
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();
	
	// Temporary buffer to stage vertex data before transfering to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	
	CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO VERTEX BUFFER
	void* data;														               // 1. Create pointer to a point in memory
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);              // 2. "MAP" the vertex buffer memory to that point
	memcpy(data, vertices->data(), static_cast<size_t>(bufferSize));               // 3. Copy memory from vertices vector to the point
	vkUnmapMemory(device, stagingBufferMemory);                                     // Unmap the vertex buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also Vertex buffer)
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host)
	CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

	// Copy staging buffer to vertex buffer on GPU
	CopyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, vertexBuffer, bufferSize);

	// Clean up staging buffer parts
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}



void Mesh::CreateIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices)
{
	// Get size of buffer needed for indices
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

	// Temporary buffer to stage vertex data before transfering to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices->data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(device, stagingBufferMemory);

	// Create buffer for INDEX data on GPU access only area
	CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

	// Copy from staging buffer to GPU acces buffer
	CopyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}