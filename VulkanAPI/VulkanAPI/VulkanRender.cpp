#include "VulkanRender.h"
#include <iostream>

VulkanRender::VulkanRender()
{
}

int VulkanRender::init(GLFWwindow* newWindow)
{
	window = newWindow;

	try
	{
	
		createInstance();
		//createDebugCallback();
		createSurface();
		getPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createRenderPass();
		createGraphicsPipeline();
		
	}
	catch (const std::runtime_error &e)
	{
		printf("ERROR: %s \n", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}

void VulkanRender::cleanup()
{
	vkDestroyPipelineLayout(mainDevice.logicalDevice, pipelineLayout, nullptr);

	for (auto image : swapChainImages)
	{
		vkDestroyImageView(mainDevice.logicalDevice, image.imageView, nullptr);
	}
	vkDestroySwapchainKHR(mainDevice.logicalDevice,swapChain,nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(mainDevice.logicalDevice,nullptr);
	vkDestroyInstance(instance, nullptr);
	
}

VulkanRender::~VulkanRender()
{
}

void VulkanRender::createInstance()
{
	/*if (validationEnabled && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Required Validation Layers not supported!");
	}*/

	// Information abourt the applition itself
	// Most data here doesnot affect the programe and is for developer convenience
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan App";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	//Creation information for a VKInstance 
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//Create list to hold instance extensions
	std::vector<const char*> instanceExtensions = std::vector<const char*>();

	//Set up extensions Instance will use
	uint32_t glfwExtensionCount = 0;		//GLFW may require multiple extensions
	const char** glfwExtensions;				//Extensions passed as array of cstrings, so need pointer (the array) to pointer (the cstring)

	//Get GLFW extensions
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	//Add GLFW extensions to list of extensions
	for (size_t i = 0; i < glfwExtensionCount; i++)
	{
		instanceExtensions.push_back(glfwExtensions[i]);
	}

	std::cout << "instanceExtensions:\n";
	for (const auto& extension : instanceExtensions) {
		std::cout << '\t' << extension<< '\n';
	}

	//Check Instance Extensions supported...
	if (!checkInstanceExtensionSupport(&instanceExtensions))
	{
		throw std::runtime_error("Faild to create a Vulkan Instance!");
	}

	createInfo.enabledExtensionCount =static_cast<uint32_t>(instanceExtensions.size());
	createInfo.ppEnabledExtensionNames = instanceExtensions.data();

	//TODO: set up Validation Layers that Instance will use
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;

	//Create instance
	//vkCreateInstance(&createInfo,nullptr,&instance);
	//VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Faild to create a Vulkan Instance!");
	}
}

//void VulkanRender::createDebugCallback()
//{
//	// Only create callback if validation enabled
//	if (!validationEnabled) return;
//
//	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
//	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
//	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;	// Which validation reports should initiate callback
//	callbackCreateInfo.pfnCallback = debugCallback;												// Pointer to callback function itself
//
//	// Create debug callback with custom create function
//	VkResult result = CreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback);
//	if (result != VK_SUCCESS)
//	{
//		throw std::runtime_error("Failed to create Debug Callback!");
//	}
//}

void VulkanRender::createLogicalDevice()
{

	//Get the queue family indices for chosen Physical Device
	QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

	//Vector for queue creation information, and set for family indices
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndices = {indices.graphicsFamily,indices.presentationFamily};

	//Queue the logical device needs to create and info to do so
	for (int queueFamilyIndex : queueFamilyIndices)
	{
		VkDeviceQueueCreateInfo  queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;	//the index of the family to create a queue from
		queueCreateInfo.queueCount = 1;	//Number of queues to create
		float priority = 1.f;
		queueCreateInfo.pQueuePriorities = &priority;		//Vulkan needs to know hoe to handle multiple queues, so decide priority(1=highest priority)

		queueCreateInfos.push_back(queueCreateInfo);
	}

	//information to create logical device (sometimes called "device")
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount =static_cast<uint32_t>(queueCreateInfos.size());		//Number of Queue Create Infos
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();		//List of queue create infos so device can create required queue family
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());	//Number of enabled logical device extensions
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();		//list of enabled logical device extensions

	//create the logical device for the given physical device
	VkResult result = vkCreateDevice(mainDevice.physicalDevice, &deviceCreateInfo,nullptr,&mainDevice.logicalDevice);
	if (result!=VK_SUCCESS)
	{
		throw std::runtime_error("Failed to Create a Logical Device!");
	}

	//Queue are Created at the same time as the device
	// So we want handle to queues
	//From given logical device, of given Queue Family, of given Queue Index, place reference in given vkqueue
	vkGetDeviceQueue(mainDevice.logicalDevice,indices.graphicsFamily,0,&graphicsQueue);
	vkGetDeviceQueue(mainDevice.logicalDevice, indices.presentationFamily, 0, &presentationQueue);
}

void VulkanRender::createSurface()
{
	//Create Surface
	VkResult result=glfwCreateWindowSurface(instance,window,nullptr,&surface);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Faild to create a surface!");
	}
}

void VulkanRender::createSwapChain()
{
	//Get Swap chain details so we can pick best settings
	SwapChainDetails swapChainDetails= getSwapChainDetails(mainDevice.physicalDevice);


	//find optimal surface valuse for our swap chain
	//1.choose best suface format
	VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapChainDetails.formats);
	//2.choose best presentation mode
	VkPresentModeKHR presentationMode = chooseBestPresentationMode(swapChainDetails.presentationModes);
	//3.choose swap chain image resolition
	VkExtent2D extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities);

	//Get 1 more than the minimum to allow triple buffering
	uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;

	//if imageCount higher than max, then clamp down to max
	//if 0, the limitless
	if (swapChainDetails.surfaceCapabilities.maxImageCount > 0 && swapChainDetails.surfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
	}

	//creation inforamtion for swap chain
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = surface;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.presentMode = presentationMode;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.clipped = VK_TRUE;

	//get queue family indices
	QueueFamilyIndices indices = getQueueFamilies(mainDevice.physicalDevice);

	//if gracphics  and presentation families are different, then swapchain must be let images be shared between families
	if (indices.graphicsFamily != indices.presentationFamily)
	{

		uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily,(uint32_t)indices.presentationFamily };

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;		//image share handing
		swapChainCreateInfo.queueFamilyIndexCount = 2;		//number of queues to share images between
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;		//array of queue to share between
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	//if old swap chain been destroyed and this one replaces it, then link old one to quickly hand over responsilities
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(mainDevice.logicalDevice,&swapChainCreateInfo,nullptr,&swapChain);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a SwapChain!");
	}

	//store for later reference
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	//get swap chain images(first count, then value)
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapChain, &swapChainImageCount, nullptr);
	std::vector<VkImage> images(swapChainImageCount);
	vkGetSwapchainImagesKHR(mainDevice.logicalDevice, swapChain, &swapChainImageCount, images.data());

	for (VkImage image : images)
	{
		//store image handle
		SwapChainImage swapChainImage = {};
		swapChainImage.image = image;

		//create image view here
		swapChainImage.imageView = crateImageView(image,swapChainImageFormat,VK_IMAGE_ASPECT_COLOR_BIT);
		swapChainImages.push_back(swapChainImage);
	}
}

void VulkanRender::createRenderPass()
{
	//color attachment of render pass
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;	//format to use for attachment
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;	//number of samples to write for mulrisampling
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;		//describe what to do with attachment before rendering
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;		//describe what to do with attachment after rendering
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;		//describe waht to do with stencil before rendering
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	//describe waht to do with stencil aftert rendering


	//feamebuffer data will be stored as an image,but iamges can be given different data layouts
	//to give optimal use for certain operations
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		//Image data layout before render pass starts
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		//Image data layout after render pass(to change to)


	//Attachment reference use an attachment index that refers to index in the attachment list passed to renderPassCreateInfo

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		//Pipline type subpass is to be bound to

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
}

void VulkanRender::createGraphicsPipeline()
{
	//read in SPIR-V code of shaders
	auto vertexShaderCode = readFile("Shaders/vert.spv");
	auto fragmentShaderCode = readFile("Shaders/frag.spv");

	//Create Shader Modules
	VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

	//--SHADER STAGE CREATION INFORMATION --
	// Vertex Stage creation information
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
	vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	
	vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;		//shader stage name
	vertexShaderCreateInfo.module = vertexShaderModule;		//shader module to be used by stage
	vertexShaderCreateInfo.pName = "main";		//enter point in to shader

	//Fragement Stage creation information
	VkPipelineShaderStageCreateInfo fragementShaderCreateInfo = {};
	fragementShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragementShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;		//shader stage name
	fragementShaderCreateInfo.module = fragmentShaderModule;		//shader module to be used by stage
	fragementShaderCreateInfo.pName = "main";		//enter point in to shader


	//Put shader stage creation info into array
	//Graphics Pipline creation info requries array of shader stage creates
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo ,fragementShaderCreateInfo };

	//CREATE PIPELINE
	//--Vertex Input (TODO: Put in vertex descriptions when resources created)--

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;		//List of Vertex Binding Descriptions(data spacing/stride info)
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;		//List of Vertex Attribute Description(data format)

	//--Input Assembly--
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;		//Primitive type to assemble vertices as
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;		//Allow overriding of "strip" topology to start new primitives

	//--ViewPort & Scissor
	//Create a viewport info struct
	VkViewport viewPort = {};
	viewPort.x = 0.0f;
	viewPort.y = 0.0f;
	viewPort.width = (float)swapChainExtent.width;
	viewPort.height = (float)swapChainExtent.height;
	viewPort.minDepth = 0.0f;
	viewPort.maxDepth=1.0f;

	//Create a scissor info struct
	VkRect2D scissor = {};
	scissor.offset = { 0,0 };		//offset to use region from
	scissor.extent = swapChainExtent;		//extent to describe region to use, starting at offset

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewPort;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	//--Dynamic States--
	//Dynamic states to enable
	//std::vector<VkDynamicState> dynamicStateEnables;
	//dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);		//Dynamic Viewport: can resize in command buffer with vkCmdSetViewport(commanbuffer,0,1,&viewport)
	//dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);		//Dynamic Scissor: can resize in command buffer with ckCmdSetScissor(commandbuffer,0,1,&scissor)
	//
	////Dynamic State creation info
	//VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	//dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	//dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
	//dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();

	//--Resterizer
	VkPipelineRasterizationStateCreateInfo rasterizerCreateinfo = {};
	rasterizerCreateinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCreateinfo.depthClampEnable = VK_FALSE;		//change if fragements beyond near/far planes are clipped (default) or clamped to plane
	rasterizerCreateinfo.rasterizerDiscardEnable = VK_FALSE;		//Whether to discard data and skip rasterizer. Never creates fragments, only suitable for pipeline without famebuffer output
	rasterizerCreateinfo.polygonMode = VK_POLYGON_MODE_FILL;		//How to handle filling points between vertices
	rasterizerCreateinfo.lineWidth = 1.0f;		//How thick lines should be when drawn
	rasterizerCreateinfo.cullMode = VK_CULL_MODE_BACK_BIT;		//which face of a tri to cull
	rasterizerCreateinfo.frontFace = VK_FRONT_FACE_CLOCKWISE;		//winding to determine which side is front
	rasterizerCreateinfo.depthBiasEnable = VK_FALSE;		//Whether to add depth bias to fragments

	//--MultiSampling
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;		//enable multisample shading or not
	multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;		//Number of samples to use per fragment

	//--Blending--
	//Blending decides how to blend a new color being written to a fragment, with old value

	//Blend Attachment State (how blending is handled)
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT		
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;		//colors to apply blending to
	colorBlendAttachmentState.blendEnable = VK_TRUE;		//enable blending


	//Blending uses equation:(srcColorBlendFactor * new color)  color BlendOp (dstColorBlendFactor * old color)
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	//Sumarized: (new color alpha * new color)+((1- new color alpha)*old color)

	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//Summarized: (1* new color alpha)+(0* old alpha)=new alpha


	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
	colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;		//Alternative to calculation is to use logical operations
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
	

	//--Pipeline layout (TODO: apply future descriptor Set Layouts)--
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	//Create Pipeline Layout
	VkResult result = vkCreatePipelineLayout(mainDevice.logicalDevice, &pipelineLayoutCreateInfo,nullptr,&pipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to crate Pipeline Layout!");
	}


	//--Depth Stencil Testing--
	//TODO: Set up depth stencil testing

	//Destory Shader Module, no longer needed after Pipeline crated
	vkDestroyShaderModule(mainDevice.logicalDevice, vertexShaderModule,nullptr);
	vkDestroyShaderModule(mainDevice.logicalDevice, fragmentShaderModule, nullptr);
}


void VulkanRender::getPhysicalDevice()
{
	//Enumerate Physical devices the vkInstance can access
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance,&deviceCount,nullptr);


	// If no devices available, the none support Vulkan
	if (deviceCount == 0)
	{
		throw std::runtime_error("Cannot find GPUs that suppot Valkan Instance!");
	}

	//Get list of physical Devices
	std::vector<VkPhysicalDevice> deviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, deviceList.data());

	for (const auto &device: deviceList)
	{
		bool rightDeviceSuitable = false;
		rightDeviceSuitable = checkDeviceSuitable(device);
		if (rightDeviceSuitable)
		{
			mainDevice.physicalDevice = device;
			break;
		}
	}
	
}

bool VulkanRender::checkInstanceExtensionSupport(std::vector<const char*>* checkExtensions)
{
	// Need to get number of extensions to create array of correct size to hold extension
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	//Create a list of vkExtensionProperties using count
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "available extensions:\n";
	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << '\n';
	}

	//Check if given extensions are in list of available extensions
	for (const auto &checkExtension: *checkExtensions)
	{
		bool hasExtension = false;
		for (const auto &extension:extensions)
		{
			if (strcmp(checkExtension, extension.extensionName))
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

bool VulkanRender::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	//Get device extension count
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device,nullptr, &extensionCount,nullptr);

	//If no extensions found, return failure
	if (extensionCount == 0)
	{
		return false;
	}

	//Populate list of extensions
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

	//Check for extension
	for (const auto deviceExtension: deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto &extension:extensions)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}
		if (!hasExtension)
		{
			return false;
		}
	}

	return true;

}

bool VulkanRender::checkDeviceSuitable(VkPhysicalDevice device)
{
	////information about the device itself(id, name, type, vendor, stc)
	//VkPhysicalDeviceProperties deviceProperties;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);

	////information about what the device can do
	//VkPhysicalDeviceFeatures deviceFeature;
	//vkGetPhysicalDeviceFeatures(device, &deviceFeature);

	QueueFamilyIndices indices = getQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainVaild = false;
	if (extensionsSupported)
	{
		SwapChainDetails swapChainDetails = getSwapChainDetails(device);
		swapChainVaild = !swapChainDetails.presentationModes.empty() > 0 && !swapChainDetails.formats.empty();
	}
	
	return  indices.isValid()&& extensionsSupported && swapChainVaild;
}

bool VulkanRender::checkValidationLayerSupport()
{
	// Get number of validation layers to create vector of appropriate size
	uint32_t validationLayerCount;
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);

	// Check if no validation layers found AND we want at least 1 layer
	if (validationLayerCount == 0 && validationLayers.size() > 0)
	{
		return false;
	}

	std::vector<VkLayerProperties> availableLayers(validationLayerCount);
	vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());

	// Check if given Validation Layer is in list of given Validation Layers
	for (const auto& validationLayer : validationLayers)
	{
		bool hasLayer = false;
		for (const auto& availableLayer : availableLayers)
		{
			if (strcmp(validationLayer, availableLayer.layerName) == 0)
			{
				hasLayer = true;
				break;
			}
		}

		if (!hasLayer)
		{
			return false;
		}
	}

	return true;
}

QueueFamilyIndices VulkanRender::getQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	//get all queue Family Property info for the given device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

	//Go though each queue family and check if it has at least 1 of required types of queue
	int i = 0;
	for (const auto& queueFamily : queueFamilyList)
	{

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		//Check if Queue Family supports presentation
		VkBool32 presentionSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device,i,surface, &presentionSupport);
		//Check if queue is presentation type (can be both graphics and presentation)
		if (queueFamily.queueCount > 0 && presentionSupport)
		{
			indices.presentationFamily = i;
		}


		if (indices.isValid())
		{
			break;
		}

		i++;
	}

	return indices;
}

SwapChainDetails VulkanRender::getSwapChainDetails(VkPhysicalDevice device)
{
	SwapChainDetails swapChainDetails;

	//--capabilities--
	//Get the surface capabilities for given surface on the given physical device
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,surface, &swapChainDetails.surfaceCapabilities);

	//--formats--
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device,surface, &formatCount,nullptr);

	//If formats returned, get list of formats
	if (formatCount!=0)
	{
		swapChainDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainDetails.formats.data());
	}

	//--presentation modes--
	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device,surface, &presentationCount,nullptr);

	//if presentation modes returned,get list of presentation modes
	if (presentationCount != 0)
	{
		swapChainDetails.presentationModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount,swapChainDetails.presentationModes.data());
		
	}



	return swapChainDetails;
}

//Best format is subjective, but ours will be:
//format:VK_FORMAT_R8G8B8A8_UINT(VK_FORMAT_B8G8R8A8_UINT as backup)
//colorSpace:VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
VkSurfaceFormatKHR VulkanRender::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR> formats)
{
	//if only 1 foramt available and is undefined, then this means all formats are available (no restrictions)
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return{VK_FORMAT_R8G8B8A8_UINT, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	}

	//if restricted, searvh for optimal format
	for (const auto& format : formats)
	{
		if ((format.format== VK_FORMAT_R8G8B8A8_UINT || format.format == VK_FORMAT_B8G8R8A8_UINT) && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	//if can not find optimal format, then just return first format
	return formats[0];
}

VkPresentModeKHR VulkanRender::chooseBestPresentationMode(const std::vector<VkPresentModeKHR> presentationModes)
{
	//Look for MailBox presentation mode
	for (const auto& presentationMode : presentationModes)
	{
		if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentationMode;
		}
	}

	//If can not  find, use FIFO ad Vulkan spec says it must be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRender::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		// if value vary, need to set manually
		int with, height;
		glfwGetFramebufferSize(window, &with, &height);

		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(with);
		newExtent.height = static_cast<uint32_t>(height);

		//Surface also defines max and min, so within boundaries by clamping value
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}

VkImageView VulkanRender::crateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewImgeCreateInfo = {};
	viewImgeCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewImgeCreateInfo.image = image;		//image to create view for
	viewImgeCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;	//Type of image(1D, 2D, 3D, Cube, etc)
	viewImgeCreateInfo.format = format;		//format of image data
	viewImgeCreateInfo.components.r  = VK_COMPONENT_SWIZZLE_IDENTITY;		//allows remapping of rgba to other ragb valu
	viewImgeCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewImgeCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewImgeCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	//Subresources allow the view to view only a part of an image
	viewImgeCreateInfo.subresourceRange.aspectMask = aspectFlags;		//which sapect of image to view
	viewImgeCreateInfo.subresourceRange.baseMipLevel = 0;		//Start mipmap levels to view from
	viewImgeCreateInfo.subresourceRange.levelCount = 1;		//number of array levels to view
	viewImgeCreateInfo.subresourceRange.baseArrayLayer = 0;		//Start array to view from
	viewImgeCreateInfo.subresourceRange.layerCount = 1;		//number of array levels to view

	//ceate iamge view and return it
	VkImageView imageView;
	VkResult result = vkCreateImageView(mainDevice.logicalDevice, &viewImgeCreateInfo,nullptr, &imageView);
	if (!result == VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create an Image View!");
	}

	return imageView;
}

VkShaderModule VulkanRender::createShaderModule(const std::vector<char>& code)
{
	//Shader Module creation information
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(mainDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to craate a shader module!");
	}

	return shaderModule;
}
