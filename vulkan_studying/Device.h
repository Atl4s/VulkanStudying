#pragma once
#include "Headers.h"
#include "Cleaner.h"
#include <vulkan/vulkan.h>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Device
{
public:
	struct
	{
		uint32_t graphicsFamily;
		uint32_t computeFamily;
		uint32_t transferFamily;
	} queueFamilyIndices;

	void init(std::vector<VkPhysicalDevice>& physicalDevices);

	VkPhysicalDevice& getPhysicalDevice() { return m_physicalDevice; }
	Cleaner<VkDevice>& getLogicalDevice() { return m_logicalDevice; }
	VkCommandPool& getCommandPool() { return m_commandPool; }
	VkQueue& getGraphicsQueue() { return m_graphicsQueue; }
	VkQueue& getComputeQueue() { return m_computeQueue; }

	uint32_t getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags);
	uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlag);

	VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	void createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags propertyFlags,
		Cleaner<VkBuffer>& buffer,
		Cleaner<VkDeviceMemory>& bufferMemory,
		void *data = nullptr);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


	Device();
	~Device();

	const std::vector<const char*> validationLayers = {
		//"VK_LAYER_LUNARG_core_validation"
	};
private:
	void selectPhysicalDevice(std::vector<VkPhysicalDevice> physicalDevices);
	void createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

	bool isPhysicalDeviceSuitable(VkPhysicalDevice);
	bool checkPhysicalDeviceExctensionSupport(VkPhysicalDevice device);

	Cleaner<VkDevice> m_logicalDevice{ vkDestroyDevice };

	VkCommandPool m_commandPool = VK_NULL_HANDLE;

	VkPhysicalDevice m_physicalDevice;
	VkPhysicalDeviceProperties m_physicalDeviceProperties;
	VkPhysicalDeviceFeatures m_physicalDeviceFeatures;
	VkPhysicalDeviceFeatures m_enabledFeatures = {};
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties;
	VkQueue m_graphicsQueue;
	VkQueue m_computeQueue;

	std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;

	std::vector<VkExtensionProperties> m_supportedExtensions;

	std::vector<const char*> m_enabledExtentions = 
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
};

