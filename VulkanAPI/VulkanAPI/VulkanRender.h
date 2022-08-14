#pragma once
//#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include<stdexcept>
#include<set>
#include<vector>
#include<algorithm>
#include<array>
#include"stb_image.h"
#include "VulkanValidation.h"
#include"Utilities.h"
#include"Mesh.h"
class VulkanRender
{
public:
	VulkanRender();

	int init(GLFWwindow* newWindow);

	void updateModel(int modelId, glm::mat4 newModel);
	void draw();
	void cleanup();


	~VulkanRender();

private:
	GLFWwindow* window;

	int currentFrame = 0;

	//Scene Objects
	 std::vector<Mesh> meshList;
	
	//Scene Settings
	 struct UboViewProjection
	 {
		 glm::mat4 projection;
		 glm::mat4 view;
	 }uboViewProjection;

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

	VkImage depthBufferImage;
	VkDeviceMemory depthBufferImageMemory;
	VkImageView depthBufferImageView;

	// -Descriptors
	VkDescriptorSetLayout descriptorSetLayout;
	VkPushConstantRange pushConstantRange;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkBuffer> vpUniformBuffer;		//viewProjection uniform buffer
	std::vector<VkDeviceMemory> vpUniformBufferMemory;

	std::vector<VkBuffer> modelDUniformBuffer;//modle dynamic uniform buffer
	std::vector<VkDeviceMemory> modelDUniformBufferMemory;

	//VkDeviceSize minUniformBufferOffset;
	//size_t modelUniformAligment;
	//UboModel* modelTransferSpace;

	//-Assets
	std::vector<VkImage> textureImages;
	std::vector<VkDeviceMemory> textureImageMemory;
	std::vector<VkImageView> textureImageViews;

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
	std::vector<VkSemaphore> imageAvailable;
	std::vector<VkSemaphore> renderFinished;
	std::vector<VkFence> drawFences;

	//Vulkan Functions
	//-Create Function
	void createInstance();
	void createDebugCallback();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createPushConstantRange();
	void createGraphicsPipeline();
	void createDepthBufferImage();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSynchronisation();

	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();

	void updateUniformBuffers(uint32_t imageIndex);

	//-Record Functions
	void recordCommands(uint32_t currebtImage);

	//-Get Functions
	void getPhysicalDevice();

	//-Allocate Functions
	void allocateDynamicBufferTransferSpace();

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
	VkFormat chooseSupportedFormat(const std::vector<VkFormat> &formats,VkImageTiling tiling,VkFormatFeatureFlags featureFlags);

	//--Create Functions
	VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags useFlages,
		VkMemoryPropertyFlags propFlages, VkDeviceMemory* imageMemory);
	VkImageView crateImageView(VkImage image,VkFormat format,VkImageAspectFlags aspectFlags);
	VkShaderModule createShaderModule(const std::vector<char>& code);

	int createTextureImage(std::string fileName);
	int createTexture(std::string fileName);

	//--Loader Functions
	stbi_uc* loadTextureFile(std::string fileNmae, int * width, int* height, VkDeviceSize* imageSize);
};

