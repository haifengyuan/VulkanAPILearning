#pragma once

#include<fstream>

const std::vector<const char*>deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

	// Get curent read position and use to resize file buffer
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