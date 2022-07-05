#pragma once
//#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#include<stdexcept>
#include<set>
#include<vector>
#include<algorithm>
#include<array>
#include "VulkanValidation.h"
#include"Utilities.h"
class VulkanRender
{
public:
	VulkanRender();

	int init(GLFWwindow* newWindow);
	void draw();
	void cleanup();


	~VulkanRender();

private:
	GLFWwindow* window;

	//Vulkan components
	//-main
	VkInstance instance;
	VkDebugReportCallbackEXT callback;
	struct 
	{
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
	}mainDevice;

	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	std::vector<SwapChainImage> swapChainImages;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> commandbuffers;

	//-PipeLine
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;


	//-Pools
	VkCommandPool graphicsCommandPool;

	//-Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	//-Synchronisation
	VkSemaphore imageAvailable;
	VkSemaphore renderFinished;

	//Vulkan Functions
	//-Create Function
	void createInstance();
	//void createDebugCallback();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSynchronisation();

	//-Get Functions
	void recordCommands();

	//-Get Functions
	void getPhysicalDevice();

	//-Support Function
	//--check Function
	bool checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkValidationLayerSupport();
	bool checkDeviceSuitable(VkPhysicalDevice device);
	

	//--Getter Functions
	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);

	//--Choose Functions
	VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> foramts);
	VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);


	//--Create Functions
	VkImageView crateImageView(VkImage image,VkFormat format,VkImageAspectFlags aspectFlags);
	VkShaderModule createShaderModule(const std::vector<char>& code);
};

