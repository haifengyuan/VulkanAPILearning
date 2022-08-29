#pragma once
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<vector>
#include"Utilities.h"

struct Model
{
	glm::mat4 model;
};
class Mesh
{
public:
	Mesh();
	Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice,
		VkQueue transferQueue,VkCommandPool transferCommandPool,
		std::vector<Vertex>* verttices, std::vector<uint32_t>* indices,
		int newTexId);

	void setModel(glm::mat4 newModel);
	Model &getModel();

	int getTextId() {return texId;};

	int getVertexCount() { return vertexCount; };
	VkBuffer getVertexBuffer() { return vertexBuffer; };

	int getIndexCount() { return indexCount; };
	VkBuffer getIndexBuffer() { return indexBuffer; };
	void destroyBuffers();
	~Mesh();

private:

	Model model;

	int texId;

	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	int indexCount;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	void createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices);
	void createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices);
};

