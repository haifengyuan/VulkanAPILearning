
#include "Mesh.h"
Mesh::Mesh()
{
}
Mesh::Mesh(VkPhysicalDevice newPhysicalDevice, VkDevice newDevice,
	VkQueue transferQueue, VkCommandPool transferCommandPool, 
	std::vector<Vertex>* verttices,std::vector<uint32_t>* indices,int newTexId)
{
	vertexCount = verttices->size();
	indexCount = indices->size();
	physicalDevice = newPhysicalDevice;
	device = newDevice;
	createVertexBuffer(transferQueue, transferCommandPool,verttices);
	createIndexBuffer(transferQueue, transferCommandPool, indices);

	model.model = glm::mat4(1.0f);
	texId = newTexId;
}

void Mesh::setModel(glm::mat4 newModel)
{
	model.model = newModel;
}

Model& Mesh::getModel()
{
	return model;
}


void Mesh::destroyBuffers()
{
	vkDestroyBuffer(device,vertexBuffer,nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);

	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);
}

Mesh::~Mesh()
{
}

void Mesh::createVertexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<Vertex>* vertices)
{
	//Get size of buffer needed for vertices
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices->size();

	//Temporary buffer yo "stage" vertex data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	//Create Staging Buffer and Allocate Memory to it 
	createBuffer(physicalDevice,device, bufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,&stagingBuffer,&stagingBufferMemory);
	//VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT: CPU can interact with memory
    //VK_MEMORY_PROPERTY_HOST_COHERENT_BIT: Allow palcement of data straight into buffer after mapping(otherwise would have to specify manually)
	//Map memory to Vertex Buffer
	void* data;																													//1. Create pointer to a point in normal memory
	vkMapMemory(device, stagingBufferMemory,0, bufferSize,0, &data);						//2. "Map" the vertex buffer memory to that point
	memcpy(data, vertices->data(),(size_t)bufferSize);													//3. Copy memory from vertices vector to the point
	vkUnmapMemory(device, stagingBufferMemory);														//4. Unmap the vertex buffer memory
	

	//Create buffer with VK_BUFFER_USAGE_TRANSFER_DST_BIT to mark as recipient of trandfer data(also Vertex_Buffer)
	//Buffer memeory is to be DEVICE_LOCAL_BIT meanning memory is on the GPU and only accessible by it  and not CPU(host)
	createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

	copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer,vertexBuffer,bufferSize);

	//Clean up staging buffer parts
	vkDestroyBuffer(device, stagingBuffer,nullptr);
	vkFreeMemory(device, stagingBufferMemory,nullptr);
}

void Mesh::createIndexBuffer(VkQueue transferQueue, VkCommandPool transferCommandPool, std::vector<uint32_t>* indices)
{
	//Get size of buffer needed for vertices
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices->size();

	//Temporary buffer yo "stage" index data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer, &stagingBufferMemory);

	//Map memory to Vertex Buffer
	void* data;																													
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);						
	memcpy(data, indices->data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	//Create buffer for INDEX data on GPU access only area
	createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

	//Copy from staging buffer to GPU buffer
	copyBuffer(device, transferQueue, transferCommandPool, stagingBuffer, indexBuffer, bufferSize);

	//Destory + Release Staging Buffer resources
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}


