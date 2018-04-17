#include "Device.h"


uint32_t Device::getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags)
{
	for (uint32_t i = 0; i < m_physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (m_physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type");
}

uint32_t Device::getQueueFamilyIndex(VkQueueFlagBits queueFlag)
{
	//std::cout << static_cast<uint32_t>(m_queueFamilyProperties.size()) << std::endl;

	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queueFlag & VK_QUEUE_COMPUTE_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
		{
			if ((m_queueFamilyProperties[i].queueFlags & queueFlag) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				return i;
				break;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	if (queueFlag & VK_QUEUE_TRANSFER_BIT)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
		{
			if ((m_queueFamilyProperties[i].queueFlags & queueFlag) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				return i;
				break;
			}
		}
	}

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
	for (uint32_t i = 0; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
	{
		if (m_queueFamilyProperties[i].queueFlags & queueFlag)
		{
			return i;
			break;
		}
	}

	throw std::runtime_error("Could not find a matching queue family index");
}

VkCommandPool Device::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
	cmdPoolInfo.flags = createFlags;
	VkCommandPool cmdPool;
	if(vkCreateCommandPool(m_logicalDevice, &cmdPoolInfo, nullptr, &cmdPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool");
	}
	return cmdPool;
}

void Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags propertyFlags,
	Cleaner<VkBuffer>& buffer, Cleaner<VkDeviceMemory>& bufferMemory, void* data)
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = usageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_logicalDevice, &bufferCreateInfo, nullptr, buffer.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer");
	}

	VkMemoryRequirements memoryRequirments;
	vkGetBufferMemoryRequirements(m_logicalDevice, buffer, &memoryRequirments);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirments.size;
	memoryAllocateInfo.memoryTypeIndex = getMemoryType(memoryRequirments.memoryTypeBits, propertyFlags);

	if (vkAllocateMemory(m_logicalDevice, &memoryAllocateInfo, nullptr, bufferMemory.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory");
	}

	// If a pointer to the buffer data has been passed, map the buffer and copy over the data
	if (data != nullptr)
	{
		void *mapped;
		if(vkMapMemory(m_logicalDevice, bufferMemory, 0, size, 0, &mapped)!=VK_SUCCESS)
		{
			throw std::runtime_error("failed to map memory");
		}
		memcpy(mapped, data, size);
		// If host coherency hasn't been requested, do a manual flush to make writes visible
		if ((propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = bufferMemory;
			mappedRange.offset = 0;
			mappedRange.size = size;
			vkFlushMappedMemoryRanges(m_logicalDevice, 1, &mappedRange);
		}
		vkUnmapMemory(m_logicalDevice, bufferMemory);
	}

	if (vkBindBufferMemory(m_logicalDevice, buffer, bufferMemory, 0) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to bind buffer memory");
	}
}

void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = m_commandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(m_logicalDevice, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
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

	if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit queue");
	}
	if (vkQueueWaitIdle(m_graphicsQueue) != VK_SUCCESS)
	{
		throw std::runtime_error("failed at wait idle");
	}

	vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &commandBuffer);
}

Device::Device()
{
}


void Device::init(std::vector<VkPhysicalDevice>& physicalDevices)
{
	selectPhysicalDevice(physicalDevices);
	createLogicalDevice(m_enabledFeatures, m_enabledExtentions);
}

Device::~Device()
{
}

/*Device::QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamiliesCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamilies.data());
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		VkBool32 presentSupport = false;
		if(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to get physical device surface support");
		}
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

/*Device::SwapchainSupportDetails Device::getSwapchainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapchainSupportDetails details;

	if ( (device, surface, &details.capabilities) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to retrieve capabilities");
	}

	uint32_t formatsCount;

	if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to retrieve formats count");
	}

	if (formatsCount != 0)
	{
		details.formats.resize(formatsCount);

		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, details.formats.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to retrieve formats");
		}
	}

	uint32_t presentModesCount;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to retrieve present modes count");
	}

	if (presentModesCount != 0)
	{
		details.presentModes.resize(presentModesCount);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, details.presentModes.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to retrieve present modes");
		}
	}

	return details;
}*/

void Device::selectPhysicalDevice(std::vector<VkPhysicalDevice> physicalDevices)
{
	for(const auto& physicalDevice: physicalDevices)
	{
		if(isPhysicalDeviceSuitable(physicalDevice))
		{
			m_physicalDevice = physicalDevice;
		}
	}

	assert(m_physicalDevice);

	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_physicalDeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);
	std::vector<VkQueueFamilyProperties> queueFamilyPropertieses(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyPropertieses.data());
	assert(!queueFamilyPropertieses.empty());
	m_queueFamilyProperties = queueFamilyPropertieses;

	#ifndef NDEBUG
		//std::cout << m_physicalDeviceProperties.deviceName << std::endl;
	#endif
}

void Device::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, bool useSwapChain, VkQueueFlags requestedQueueTypes)
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	//std::set<int> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily, queueFamilyIndices.presentFamily };

	float queuePriority = 1.0f;

	// Graphics queue
	if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
	{
		queueFamilyIndices.graphicsFamily = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueInfo);
	}
	else
	{
		queueFamilyIndices.graphicsFamily = VK_NULL_HANDLE;
	}

	// Dedicated compute queue
	if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
	{
		queueFamilyIndices.computeFamily = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
		if (queueFamilyIndices.computeFamily != queueFamilyIndices.graphicsFamily)
		{
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.computeFamily;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	}
	else
	{
		// Else we use the same queue
		queueFamilyIndices.computeFamily = queueFamilyIndices.graphicsFamily;
	}

	// Dedicated transfer queue
	if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
	{
		queueFamilyIndices.transferFamily = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
		if ((queueFamilyIndices.transferFamily != queueFamilyIndices.graphicsFamily) && (queueFamilyIndices.transferFamily != queueFamilyIndices.computeFamily))
		{
			// If compute family index differs, we need an additional queue create info for the compute queue
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.transferFamily;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
	}
	else
	{
		// Else we use the same queue
		queueFamilyIndices.transferFamily = queueFamilyIndices.graphicsFamily;
	}

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();

	deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

	deviceCreateInfo.enabledExtensionCount = enabledExtensions.size();
	deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();

	if (enableValidationLayers) {
		deviceCreateInfo.enabledLayerCount = validationLayers.size();
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, m_logicalDevice.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device");
	}	

	m_commandPool = createCommandPool(queueFamilyIndices.graphicsFamily);

	vkGetDeviceQueue(m_logicalDevice, queueFamilyIndices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, queueFamilyIndices.computeFamily, 0, &m_computeQueue);
}

bool Device::isPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice)
{
	//TODO: maybe add more checkings
	
	return checkPhysicalDeviceExctensionSupport(physicalDevice);
}

bool Device::checkPhysicalDeviceExctensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	m_supportedExtensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, m_supportedExtensions.data());

	std::set<std::string> requiredExtensions(m_enabledExtentions.begin(), m_enabledExtentions.end());

#ifndef NDEBUG
	std::cout << "Physical device extensions supported:" << std::endl;
#endif

	for (const auto& extension : m_supportedExtensions) {

#ifndef NDEBUG
		std::cout << "\t" << extension.extensionName << std::endl;
#endif 

		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}
