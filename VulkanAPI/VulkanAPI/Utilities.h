#pragma once

#include<fstream>
#define GLFW_INCLUED_VULKAN
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
const int MAX_FRAME_DRAWS = 2;
const int MAX_OBJECTS = 2;
const std::vector<const char*>deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};


struct Vertex
{
	glm::vec3 pos;		//Vertex Position
	glm::vec3 color;	//Vertex Color (r,g,b)
};

//Indices (location) of Quenue Families (if they exitat all)
struct QueueFamilyIndices 
{
	int graphicsFamily = -1;	//location of Graphics Queue Famliy
	int presentationFamily = -1;		//location of Presention Queue Family

	// check if queue are vaild
	bool isValid()
	{
		return graphicsFamily >= 0 && presentationFamily >=0;
	}
};

struct SwapChainDetails
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;		//Surface properties, e.g. image size/extent
	std::vector<VkSurfaceFormatKHR> formats;			//Surface image formats, e.g. RGBA and size of each color
	std::vector<VkPresentModeKHR> presentationModes;		//How images should be presented to screen
};

struct SwapChainImage
{
	VkImage image;
	VkImageView imageView;
};

static std::vector<char> readFile(const std::string &filename)
{
	//open stream from given file
	//std::ios::binary tells stream to read file as binary
	//std::ios::ate tells stream to start reading from end of file
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	//check if file stream successfully opened
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open a file!");
	}

	// Get current read position and use to resize file buffer
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> filebuffer(fileSize);

	//Move read position (seek to) the start of the file
	file.seekg(0);

	//read the file data to the buffer (stream "fileSize" in total)
	file.read(filebuffer.data(), fileSize);

	//close stream
	file.close();

	return filebuffer;
}

static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice,uint32_t allowedTypes, VkMemoryPropertyFlags properties)
{
	//Get Properties of physical device memory
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (size_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((allowedTypes & (1 << i))																									//Index of memory type must match corresponding bit in allowed Type
			&& (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)		//Desired properties bit flages are part of memory type's property f
		{
			//This memory type is vaild, so return its index
			return i;
		}
	}
}

static void createBuffer(VkPhysicalDevice physicalDevice,VkDevice device,VkDeviceSize bufferSize,VkBufferUsageFlags bufferUsage,
	VkMemoryPropertyFlags bufferProperties,VkBuffer *buffer, VkDeviceMemory * bufferMemory)
{
	//Create Vertex Buffer
//Information to crate a buffer (does not include assignimg memory)
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = bufferSize;		//Size of buffer(size of 1 vertex * number of vertices)
	bufferCreateInfo.usage = bufferUsage;		//Multiple types of buffer possible
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		//Similar to Swap Chain images, can share vertex buffers

	VkResult result = vkCreateBuffer(device, &bufferCreateInfo, nullptr, buffer);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Vertex Buffer!");
	}

	//Get buffer memory requirements
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

	//Allocate memory to Buffer
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice,memRequirements.memoryTypeBits,		//Index of memory type on Physical Device that has required bit falgs
		bufferProperties);																																								
																																																
	//Allocate memory to given VkDeciceMemory
	result = vkAllocateMemory(device, &memoryAllocateInfo, nullptr, bufferMemory);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allcoate Vertex Buffer memory!");
	}

	//Allocate memory to given  buffer
	vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

static VkCommandBuffer beginCommandBuffer(VkDevice device, VkCommandPool commandPool)
{
	//Command buffer to hold transfer commands
	VkCommandBuffer commandBuffer;

	//Command Buffer details
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	//Allocate command buffer from pool
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	//Information to begin the command buffer record
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;		//We're only using the command buffer once

	//Begin recording transfer commands
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

static void endAndSubmitCommandBuffer(VkDevice device,VkCommandPool commandPool,VkQueue queue,VkCommandBuffer commandBuffer)
{
	//End commands
	vkEndCommandBuffer(commandBuffer);

	//Queue submission information
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	//Submit transfer command to transfer queue and wait until it finishes
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	//Free temporary command buffer back to pool
	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

static void copyBuffer(VkDevice device,VkQueue transferQueue,VkCommandPool transferCommandPool,
	VkBuffer srcBuffer,VkBuffer dstBuffer,VkDeviceSize bufferSize)
{
	//Create buffer
	VkCommandBuffer transferCommandBuffer = beginCommandBuffer(device, transferCommandPool);
	
	//Regin of data to copy from and to
	VkBufferCopy bufferCopyRegin = {};
	bufferCopyRegin.srcOffset = 0;
	bufferCopyRegin.dstOffset = 0;
	bufferCopyRegin.size = bufferSize;

	//Command to copy src buffer to dst buffer
	vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer,1,&bufferCopyRegin);

	endAndSubmitCommandBuffer(device, transferCommandPool, transferQueue, transferCommandBuffer);
}

static void copyImageBuffer(VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool,
	VkBuffer scrBuffer, VkImage image, uint32_t width, uint32_t height)
{
	//Create Buffer
	VkCommandBuffer transferCommandBuffer = beginCommandBuffer(device, transferCommandPool);

	VkBufferImageCopy imageRegin = {};
	imageRegin.bufferOffset = 0;		//Offset into data
	imageRegin.bufferRowLength = 0; //Row legth of data to calculate data spacing
	imageRegin.bufferImageHeight = 0;	//image height to calculate data spacing
	imageRegin.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		//Which aspect of image to copy
	imageRegin.imageSubresource.mipLevel = 0;		//Mipmap level to copy
	imageRegin.imageSubresource.baseArrayLayer = 0;		//Starting array layer (if array)
	imageRegin.imageSubresource.layerCount = 1;			//Number of layer to copy starting at baseArrayLayer
	imageRegin.imageOffset = { 0,0,0 };
	imageRegin.imageExtent = { width,height,1 };

	//Copy buffer to given image
	vkCmdCopyBufferToImage(transferCommandBuffer, scrBuffer, image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1,& imageRegin);

	endAndSubmitCommandBuffer(device, transferCommandPool, transferQueue,transferCommandBuffer);
}

static void transitionImageLayout(VkDevice device,VkQueue queue,VkCommandPool commandPool, VkImage image,VkImageLayout oldLayout,VkImageLayout newLayout)
{
	//Create buffer
	VkCommandBuffer commandBuffer = beginCommandBuffer(device,commandPool);

	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;		//image being accessed and modified as part of barriers
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;


	VkPipelineStageFlags srcStage;
	VkPipelineStageFlags dstStage;

	// If transition from new image to image ready to receive data...
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;																	//Memory access stage transition must after...
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		//Memory access stage transition must before...

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	// If transition from transfer destination to shader readable...
	else if (oldLayout== VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL&& newLayout==VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{

		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;		

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		srcStage, dstStage,												//Pipeline stages (match to src and dst AccessMasks)
		0,													//Dependency flags
		0, nullptr,										//Memory Barrier count+data
		0, nullptr,										//Buffer Memory Barrier count + data
		1, &imageMemoryBarrier);			//Image Memory Barrier count + data

	endAndSubmitCommandBuffer(device, commandPool, queue, commandBuffer);

}
