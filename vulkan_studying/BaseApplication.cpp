#include "BaseApplication.h"

#undef max
#undef min

BaseApplication::BaseApplication()
{
	m_cameras.push_back(new Camera);
}


BaseApplication::~BaseApplication()
{
	for (auto camera : m_cameras)
	{
		delete camera;
	}

	delete m_window;
	SDL_Quit();
}

void BaseApplication::run()
{
	createWindow();
	initVulkan();
	loop();
}

void BaseApplication::createWindow()
{
	m_window = new Window();
	SDL_VERSION(&m_info.version);
	if (SDL_GetWindowWMInfo(m_window->getWindow(), &m_info)) { /* the call returns true on success */
											  /* success */
		const char *subsystem = "an unknown system";
		switch (m_info.subsystem) {
		case SDL_SYSWM_UNKNOWN:
			break;
		case SDL_SYSWM_WINDOWS: 
			subsystem = "Microsoft Windows(TM)";
			break;
		case SDL_SYSWM_X11:      
			subsystem = "X Window System";       
			break;
#if SDL_VERSION_ATLEAST(2, 0, 3)
		case SDL_SYSWM_WINRT:    
			subsystem = "WinRT";                  
			break;
#endif
		case SDL_SYSWM_DIRECTFB:  
			subsystem = "DirectFB";               
			break;
		case SDL_SYSWM_COCOA:     
			subsystem = "Apple OS X";            
			break;
		case SDL_SYSWM_UIKIT:     
			subsystem = "UIKit";                  
			break;
#if SDL_VERSION_ATLEAST(2, 0, 2)
		case SDL_SYSWM_WAYLAND:   
			subsystem = "Wayland";            
			break;
		case SDL_SYSWM_MIR:     
			subsystem = "Mir";               
			break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
		case SDL_SYSWM_ANDROID:  
			subsystem = "Android";        
			break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 5)
		case SDL_SYSWM_VIVANTE:   
			subsystem = "Vivante";           
			break;
#endif
		}

		SDL_Log("This program is running SDL version %d.%d.%d on %s",
			static_cast<int>(m_info.version.major),
			static_cast<int>(m_info.version.minor),
			static_cast<int>(m_info.version.patch),
			subsystem);
	} else {
		/* call failed */
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't get window information: %s", SDL_GetError());
	}
}

void BaseApplication::initVulkan()
{
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available");
	}

	checkExtensions();
	createVulkanInstance();
	setupDebugCallback();
	createSurface();
	///m_swapchain.createSurface(m_window, m_instance);
	setupDevice();
	createSwapchain();
	createImageViews();
	///m_swapchain.createSwapchain(m_device, m_window->getWidth(), m_window->getHeight());
	///m_swapchain.createImageViews(m_device);
	createRenderPass();
	createDescriptionSetLayout();
	createGraphicsPipeline();
	createFrameBuffers();

	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffer();
	createDescriptorPool();
	createDescriptorSet();

	createCommandBuffers();
	createSemaphores();
}

void BaseApplication::createVulkanInstance()
{
	VkApplicationInfo appInfo = {}; // TODO: do something with that initialization
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "vulkan application";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = "vulkan engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	instanceCreateInfo.enabledExtensionCount = m_extensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = m_extensions.data();

	if (enableValidationLayers) {
		instanceCreateInfo.enabledLayerCount = m_device.validationLayers.size();
		instanceCreateInfo.ppEnabledLayerNames = m_device.validationLayers.data();
	}
	else {
		instanceCreateInfo.enabledLayerCount = 0;
		instanceCreateInfo.ppEnabledLayerNames = nullptr;
	}

	if (vkCreateInstance(&instanceCreateInfo, nullptr, m_instance.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vulkan instance");
	}
}

void BaseApplication::setupDebugCallback()
{
	if (!enableValidationLayers) return;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (CreateDebugReportCallbackEXT(m_instance, &createInfo, nullptr, m_callback.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug callback");
	}
}

void BaseApplication::createSurface()
{
	SDL_SysWMinfo info = {};
	SDL_GetWindowWMInfo(m_window->getWindow(), &info);

#if defined(_WIN32)
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKhr = {};
	win32SurfaceCreateInfoKhr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKhr.hwnd = info.info.win.window;
	win32SurfaceCreateInfoKhr.hinstance = GetModuleHandle(nullptr);

	auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkCreateWin32SurfaceKHR");

	if (!CreateWin32SurfaceKHR || CreateWin32SurfaceKHR(m_instance, &win32SurfaceCreateInfoKhr,
		nullptr, m_surface.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface");
	}
#elif defined(__ANDROID__)
	//TODO: create surface for this
#elif defined(_DIRECT2DISPLAY)
	//TODO: create surface for this
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	//TODO: create surface for this
#elif defined(__linux__)
	//TODO: create surface for this
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	//TODO: create surface for this
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	//TODO: create surface for this
#endif
}

void BaseApplication::setupDevice()
{
	uint32_t physicalDevicesCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &physicalDevicesCount, nullptr);

	if (physicalDevicesCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with VulkanAPI support");
	}

	std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
	vkEnumeratePhysicalDevices(m_instance, &physicalDevicesCount, physicalDevices.data());

	m_device.init(physicalDevices);
}

/*void BaseApplication::selectPhysicalDevice()
{
	uint32_t devicesCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &devicesCount, nullptr);

	if (devicesCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with VulkanAPI support");
	}

	std::vector<VkPhysicalDevice> devices(devicesCount);
	vkEnumeratePhysicalDevices(m_instance, &devicesCount, devices.data());

	for (const auto &device : devices)
	{
		if (isPhysicalDeviceSuitable(device))
		{
			std::cout << "suitable" << std::endl;
			m_physicalDevice = device;
			break;
		}
		else 
		{
			std::cout << "not suitable" << std::endl;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU");
	}
}*/

/*QueueFamilyIndices BaseApplication::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (indices.isComplete()) 
		{
			break;
		}

		i++;
	}

	return indices;
}*/

/*void BaseApplication::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

	float queuePriority = 1.0f;

	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCreateInfos.size();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = m_physicalDeviceExctentions.size();
	createInfo.ppEnabledExtensionNames = m_physicalDeviceExctentions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = m_validationLayers.size();
		createInfo.ppEnabledLayerNames = m_validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, m_logicalDevice.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device");
	}

	vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, indices.presentFamily, 0, &m_presentQueue);
}*/

void BaseApplication::createSwapchain()
{
	SwapchainSupportDetails swapchainSupportDetails = getSwapchainSupportDetails(m_device.getPhysicalDevice());

	VkSurfaceFormatKHR surfaceFormat = getSwapchainSurfaceFormat(swapchainSupportDetails.formats);
	VkPresentModeKHR presentMode = getSwapchainPresentMode(swapchainSupportDetails.presentModes);
	VkExtent2D extent = getSwapchainExtent(swapchainSupportDetails.capabilities);

	uint32_t imageCount = swapchainSupportDetails.capabilities.minImageCount + 1;
	if (swapchainSupportDetails.capabilities.maxImageCount > 0 && imageCount > swapchainSupportDetails.capabilities.maxImageCount) {
		imageCount = swapchainSupportDetails.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = m_surface;

	swapChainCreateInfo.minImageCount = imageCount;

	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = extent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.queueFamilyIndexCount = 0; // Optional
	swapChainCreateInfo.pQueueFamilyIndices = nullptr; // Optional

	swapChainCreateInfo.preTransform = swapchainSupportDetails.capabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.clipped = VK_TRUE;

	VkSwapchainKHR oldSwapchain = m_swapchain;
	swapChainCreateInfo.oldSwapchain = oldSwapchain;

	VkSwapchainKHR newSwapchain;

	VkBool32 presentSupport = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(m_device.getPhysicalDevice(), m_device.queueFamilyIndices.graphicsFamily, m_surface, &presentSupport);
	if (presentSupport != VK_TRUE)
	{
		throw std::runtime_error("physical device does not support presenting surfaces");
	}

	if (vkCreateSwapchainKHR(m_device.getLogicalDevice(), &swapChainCreateInfo, nullptr, &newSwapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain");
	}
	m_swapchain = newSwapchain;


	if(vkGetSwapchainImagesKHR(m_device.getLogicalDevice(), m_swapchain, &imageCount, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to get swapchain images count");
	}
	m_swapchainImages.resize(imageCount);
	if(vkGetSwapchainImagesKHR(m_device.getLogicalDevice(), m_swapchain, &imageCount, m_swapchainImages.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to get swapchain images");
	}

	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;
}

void BaseApplication::createImageViews()
{
	///auto swapchainImages = m_swapchain.getSwapchainImages();
	m_swapchainImageViews.resize(m_swapchainImages.size(), Cleaner<VkImageView>{m_device.getLogicalDevice(), vkDestroyImageView});
	///auto imageFormat = m_swapchain.getSwapchainImageFormat();

	for (uint32_t i = 0; i < m_swapchainImages.size(); i++) 
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapchainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device.getLogicalDevice(), &createInfo, nullptr, m_swapchainImageViews[i].data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views");
		}
	}
}

void BaseApplication::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_swapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpassDependency;

	if (vkCreateRenderPass(m_device.getLogicalDevice(), &renderPassInfo, nullptr, m_renderPass.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass");
	}
}

void BaseApplication::createDescriptionSetLayout()
{
	VkDescriptorSetLayoutBinding layoutBinding = {};
	layoutBinding.binding = 0;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;

	if (vkCreateDescriptorSetLayout(m_device.getLogicalDevice(), &layoutInfo, nullptr, m_descriptorSetLayout.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout");
	}
}

void BaseApplication::createGraphicsPipeline()
{
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	auto vertexShaderCode = readFile("../Shaders/vert.spv");
	auto fragmentShaderCode = readFile("../Shaders/frag.spv");

	createShaderModule(vertexShaderCode, m_vertexShaderModule);
	createShaderModule(fragmentShaderCode, m_fragmentShaderModule);

	VkPipelineShaderStageCreateInfo vertexShaderStageInfo = {};
	vertexShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageInfo.module = m_vertexShaderModule;
	vertexShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragmentShaderStageInfo = {};
	fragmentShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageInfo.module = m_fragmentShaderModule;
	fragmentShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageInfo, fragmentShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_swapchainExtent.width);
	viewport.height = static_cast<float>(m_swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissors = {};
	scissors.offset = {0, 0};
	scissors.extent = m_swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissors;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	VkDynamicState dynamicStates[] = 
	{
		VK_DYNAMIC_STATE_VIEWPORT, 
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkDescriptorSetLayout setLayouts[] = {m_descriptorSetLayout};
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = setLayouts;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	if (vkCreatePipelineLayout(m_device.getLogicalDevice(), &pipelineLayoutInfo, nullptr, m_pipelineLayout.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(m_device.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, m_graphicsPipeline.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline");
	}
}

void BaseApplication::createShaderModule(const std::vector<char>& code, Cleaner<VkShaderModule>& shaderModule)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = (uint32_t*)code.data();

	if (vkCreateShaderModule(m_device.getLogicalDevice(), &createInfo, nullptr, shaderModule.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module");
	}
}

void BaseApplication::createFrameBuffers()
{
	///auto swapchainImageViews = m_swapchain.getSwapchainImageViews();
	m_swapchainFramebuffers.resize(m_swapchainImageViews.size(), Cleaner<VkFramebuffer>{ m_device.getLogicalDevice(), vkDestroyFramebuffer });
	
	for (size_t i = 0; i < m_swapchainImageViews.size(); i++)
	{
		VkImageView attachments[] = { m_swapchainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_swapchainExtent.width;
		framebufferInfo.height = m_swapchainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device.getLogicalDevice(), &framebufferInfo, nullptr, m_swapchainFramebuffers[i].data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer");
		}
	}
	
}

/*void BaseApplication::createCommandPool()
{
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = m_device.queueFamilyIndices.graphicsFamily;
	commandPoolInfo.flags = 0;

	if (vkCreateCommandPool(m_device.getLogicalDevice(), &commandPoolInfo, nullptr, m_commandPool.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool");
	}
}*/

void BaseApplication::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

	Cleaner<VkBuffer> stagingBuffer{ m_device.getLogicalDevice(), vkDestroyBuffer };
	Cleaner<VkDeviceMemory> stagingBufferMemory{ m_device.getLogicalDevice(), vkFreeMemory };

	m_device.createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	void* data;
	if (vkMapMemory(m_device.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to map memory");
	}
	memcpy(data, m_vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(m_device.getLogicalDevice(), stagingBufferMemory);

	m_device.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_vertexBuffer,
		m_vertexBufferMemory);

	m_device.copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);
}

void BaseApplication::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(m_indices[0]) * m_indices.size();

	Cleaner<VkBuffer> stagingBuffer{ m_device.getLogicalDevice(), vkDestroyBuffer };
	Cleaner<VkDeviceMemory> stagingBufferMemory{ m_device.getLogicalDevice(), vkFreeMemory };

	m_device.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	void* data;
	if (vkMapMemory(m_device.getLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to map memory");
	}
	memcpy(data, m_indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device.getLogicalDevice(), stagingBufferMemory);

	m_device.createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_indexBuffer,
		m_indexBufferMemory);

	m_device.copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);
}

void BaseApplication::createUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformStagingBuffer, m_uniformStagingBufferMemory);
	m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_uniformBuffer, m_uniformBufferMemory);

	UniformBufferObject ubo = {};
	ubo.model = glm::mat4(1.0f);
	ubo.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.projection = glm::perspective(glm::radians(45.0f), m_swapchainExtent.width/static_cast<float>(m_swapchainExtent.height), 0.1f, 10.0f);
	ubo.projection[1][1] *= -1;

	void* data;
	vkMapMemory(m_device.getLogicalDevice(), m_uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(m_device.getLogicalDevice(), m_uniformStagingBufferMemory);

	m_device.copyBuffer(m_uniformStagingBuffer, m_uniformBuffer, sizeof(ubo));
}

void BaseApplication::createDescriptorPool()
{
	VkDescriptorPoolSize descriptorPoolSize = {};
	descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = 1;
	descriptorPoolInfo.pPoolSizes = &descriptorPoolSize;
	descriptorPoolInfo.maxSets = 1;
	descriptorPoolInfo.flags = 0;

	if (vkCreateDescriptorPool(m_device.getLogicalDevice(), &descriptorPoolInfo, nullptr, m_descriptorPool.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool");
	}
}

void BaseApplication::createDescriptorSet()
{
	VkDescriptorSetLayout descriptorSetLayouts[] = {m_descriptorSetLayout};
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = m_descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts;

	if (vkAllocateDescriptorSets(m_device.getLogicalDevice(), &descriptorSetAllocateInfo, &m_descriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor set");
	}

	VkDescriptorBufferInfo descriptorBufferInfo = {};
	descriptorBufferInfo.buffer = m_uniformBuffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(UniformBufferObject);

	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.dstSet = m_descriptorSet;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
	writeDescriptorSet.pImageInfo = nullptr;
	writeDescriptorSet.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(m_device.getLogicalDevice(), 1, &writeDescriptorSet, 0, nullptr);
}

/*void BaseApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags, Cleaner<VkBuffer>& buffer, Cleaner<VkDeviceMemory>& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_device.getLogicalDevice(), &bufferInfo, nullptr, buffer.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer");
	}

	VkMemoryRequirements memoryRequirments;
	vkGetBufferMemoryRequirements(m_device.getLogicalDevice(), buffer, &memoryRequirments);

	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirments.size;
	allocateInfo.memoryTypeIndex = m_device.getMemoryType(memoryRequirments.memoryTypeBits, propertyFlags);

	if (vkAllocateMemory(m_device.getLogicalDevice(), &allocateInfo, nullptr, bufferMemory.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory");
	}

	if (vkBindBufferMemory(m_device.getLogicalDevice(), buffer, bufferMemory, 0) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to bind buffer memory");
	}
}*/

/*void BaseApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandPool = m_commandPool;
	allocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(m_device.getLogicalDevice(), &allocateInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	if (vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin command buffer");
	}

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to end command buffer");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	
	VkQueue graphicsQueue;
	vkGetDeviceQueue(m_device.getLogicalDevice(), m_device.queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit queue");
	}
	if (vkQueueWaitIdle(graphicsQueue) != VK_SUCCESS)
	{
		throw std::runtime_error("failed at wait idle");
	}

	vkFreeCommandBuffers(m_device.getLogicalDevice(), m_commandPool, 1, &commandBuffer);
}*/

void BaseApplication::createCommandBuffers()
{
	if (m_commandBuffers.size() > 0)
	{
		vkFreeCommandBuffers(m_device.getLogicalDevice(), m_device.getCommandPool(), m_commandBuffers.size(), m_commandBuffers.data());
	}

	m_commandBuffers.resize(m_swapchainFramebuffers.size());

	VkCommandBufferAllocateInfo commandBufferAllocInfo = {};
	commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocInfo.commandPool = m_device.getCommandPool();
	commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

	if (vkAllocateCommandBuffers(m_device.getLogicalDevice(), &commandBufferAllocInfo, m_commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers");
	}

	for (size_t i = 0; i < m_commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(m_commandBuffers[i], &commandBufferBeginInfo);

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.framebuffer = m_swapchainFramebuffers[i];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = m_swapchainExtent;
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

		VkBuffer vertexBuffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);

		vkCmdDrawIndexed(m_commandBuffers[i], m_indices.size(), 1, 0, 0, 0);
		vkCmdEndRenderPass(m_commandBuffers[i]);

		if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer");
		}
	}

}

void BaseApplication::createSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr, m_imageAvailableSemaphore.data()) != VK_SUCCESS ||
		vkCreateSemaphore(m_device.getLogicalDevice(), &semaphoreInfo, nullptr, m_renderFinishedSemaphore.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create semaphores");
	}
}

void BaseApplication::recreateSwapchain()
{
	vkDeviceWaitIdle(m_device.getLogicalDevice());

	createSwapchain();
	createImageViews();
	///m_swapchain.createSwapchain(m_device, m_window->getWidth(), m_window->getHeight());
	///m_swapchain.createImageViews(m_device);
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommandBuffers();
}

void BaseApplication::loop()
{
	while (m_running)
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_KEYDOWN) m_running = false;
			if (event.type == SDL_QUIT) m_running = false;

			if (event.type == SDL_WINDOWEVENT_RESIZED)
			{
				recreateSwapchain();
			}
		}

		
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto time = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count() / 1000.0f;

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.projection = glm::perspective(glm::radians(45.0f), m_swapchainExtent.width / static_cast<float>(m_swapchainExtent.height), 0.1f, 10.0f);
		ubo.projection[1][1] *= -1;

		void* data;
		vkMapMemory(m_device.getLogicalDevice(), m_uniformStagingBufferMemory, 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(m_device.getLogicalDevice(), m_uniformStagingBufferMemory);

		m_device.copyBuffer(m_uniformStagingBuffer, m_uniformBuffer, sizeof(ubo));
		
		draw();
		//SDL_Delay(1);
	}

	vkDeviceWaitIdle(m_device.getLogicalDevice());
}

void BaseApplication::draw()
{
	uint32_t imageIndex;
	VkResult result;

	result = vkAcquireNextImageKHR(m_device.getLogicalDevice(), m_swapchain, std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to acquire swap chain image");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	//TODO: add something better
	

	if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { m_swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	//TODO: it will crashes here if queue does not support both graphics and present
	result = vkQueuePresentKHR(m_device.getGraphicsQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		recreateSwapchain();
	} 
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image");
	}
}

void BaseApplication::checkExtensions()
{
	uint32_t extensionsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionsCount);

	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

#ifndef NDEBUG
	std::cout << "Extensions supported:" << std::endl;

	for (const auto& extension : extensions)
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}
#endif

#if defined(_WIN32)
	m_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
	m_extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_DIRECT2DISPLAY)
	m_extensions.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	m_extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
	m_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_IOS_MVK)
	m_extensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	m_extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif
}

bool BaseApplication::checkValidationLayerSupport()
{
	uint32_t layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data());

#ifndef NDEBUG
	std::cout << "Validation layers supported:" << std::endl;

	for (const auto& layerProperties : availableLayers) {
		std::cout << "\t" << layerProperties.layerName << std::endl;
	}
#endif 

	for (auto layerName : m_device.validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

/*bool BaseApplication::checkPhysicalDeviceExctensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(m_physicalDeviceExctentions.begin(), m_physicalDeviceExctentions.end());

#ifndef NDEBUG
	std::cout << "Physical device extensions supported:" << std::endl;
#endif

	for (const auto& extension : availableExtensions) {

#ifndef NDEBUG
		std::cout << "\t" << extension.extensionName << std::endl;
#endif 

		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}*/

//bool BaseApplication::checkSwapchainAdequacy(VkPhysicalDevice device)
//{
//	SwapchainSupportDetails details = getSwapchainSupportDetails(device);
//	//TODO: do something better maybe
//	return !details.formats.empty() && !details.presentModes.empty();
//}


/*bool BaseApplication::isPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

#ifndef NDEBUG
	std::cout << deviceProperties.deviceName << std::endl;
#endif

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	return indices.isComplete() && checkPhysicalDeviceExctensionSupport(physicalDevice) && checkSwapchainAdequacy(physicalDevice);
}*/

//int BaseApplication::ratePhysicalDeviceSuitability()
//{
//	// TODO: calculate something for choosing GPU
//
//	return 0;
//}

/*uint32_t BaseApplication::getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type");
}*/

SwapchainSupportDetails BaseApplication::getSwapchainSupportDetails(VkPhysicalDevice physicalDevice)
{
	SwapchainSupportDetails details;

	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &details.capabilities) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to retrieve capabilities");
	}

	uint32_t formatsCount;

	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatsCount, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to retrieve formats count");
	}

	if (formatsCount != 0)
	{
		details.formats.resize(formatsCount);

		if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatsCount, details.formats.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to retrieve formats");
		}
	}

	uint32_t presentModesCount;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModesCount, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to retrieve present modes count");
	}

	if (presentModesCount != 0)
	{
		details.presentModes.resize(presentModesCount);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModesCount, details.presentModes.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to retrieve present modes");
		}
	}

	return details;
}

VkSurfaceFormatKHR BaseApplication::getSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats)
{
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& surfaceFormat : surfaceFormats) {
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return surfaceFormat;
		}
	}

	return surfaceFormats[0];
}

VkPresentModeKHR BaseApplication::getSwapchainPresentMode(const std::vector<VkPresentModeKHR>& surfacePresentModes)
{
	for (const auto& surfacePresentMode : surfacePresentModes)
	{
		if (surfacePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return surfacePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D BaseApplication::getSwapchainExtent(const VkSurfaceCapabilitiesKHR & capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { m_window->getWidth(), m_window->getHeight() };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
