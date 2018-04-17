#pragma once

#include "Window.h"
#include "Cleaner.h"
#include "SDL_syswm.h"

#include "Device.h"
#include "Swapchain.h"
#include "Vertex.h"
#include "Camera.h"


inline VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

inline void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class BaseApplication
{
public:
	BaseApplication();
	~BaseApplication();

	void run();

private:
	void createWindow();
	void initVulkan();
	void createVulkanInstance();
	void setupDebugCallback();
	void createSurface();
	void setupDevice();
	void createSwapchain();
	void createImageViews();
	void createRenderPass();
	void createDescriptionSetLayout();
	void createGraphicsPipeline();
	void createShaderModule(const std::vector<char>& code, Cleaner<VkShaderModule>& shaderModule);
	void createFrameBuffers();
	//void createCommandPool();

	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffer();
	void createDescriptorPool();
	void createDescriptorSet();

	void createCommandBuffers();
	void createSemaphores();

	void checkExtensions();
	void recreateSwapchain();

	/*void createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags propertyFlags,
		Cleaner<VkBuffer>& buffer,
		Cleaner<VkDeviceMemory>& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);*/

	void loop();
	void draw();

	bool checkValidationLayerSupport();
	//bool checkPhysicalDeviceExctensionSupport(VkPhysicalDevice physicalDevice);
	//bool checkSwapchainAdequacy(VkPhysicalDevice physicalDevice);
	//bool isPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice);

	//int ratePhysicalDeviceSuitability();

	//uint32_t getMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags);

	SwapchainSupportDetails getSwapchainSupportDetails(VkPhysicalDevice physicalDevice);
	VkSurfaceFormatKHR getSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats);
	VkPresentModeKHR getSwapchainPresentMode(const std::vector<VkPresentModeKHR>& surfacePresentModes);
	VkExtent2D getSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	Window* m_window;
	bool m_running = true;
	SDL_SysWMinfo m_info;
	Cleaner<VkInstance> m_instance{ vkDestroyInstance };
	Cleaner<VkDebugReportCallbackEXT> m_callback{ m_instance, DestroyDebugReportCallbackEXT};
	///Swapchain m_swapchain;
	Cleaner<VkSurfaceKHR> m_surface{ m_instance, vkDestroySurfaceKHR };
	//VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	//Cleaner<VkDevice> m_logicalDevice{ vkDestroyDevice };
	Device m_device;
	
	Cleaner<VkSwapchainKHR> m_swapchain{ m_device.getLogicalDevice(), vkDestroySwapchainKHR };
	//VkQueue m_graphicsQueue;
	//VkQueue m_presentQueue;
	std::vector<VkImage> m_swapchainImages;
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent;
	std::vector<Cleaner<VkImageView>> m_swapchainImageViews;
	Cleaner<VkShaderModule> m_vertexShaderModule{ m_device.getLogicalDevice(), vkDestroyShaderModule };
	Cleaner<VkShaderModule> m_fragmentShaderModule{ m_device.getLogicalDevice(), vkDestroyShaderModule };
	Cleaner<VkDescriptorSetLayout> m_descriptorSetLayout{ m_device.getLogicalDevice(), vkDestroyDescriptorSetLayout};
	Cleaner<VkPipelineLayout> m_pipelineLayout{ m_device.getLogicalDevice(), vkDestroyPipelineLayout };
	Cleaner<VkRenderPass> m_renderPass{ m_device.getLogicalDevice(), vkDestroyRenderPass };
	Cleaner<VkPipeline> m_graphicsPipeline{ m_device.getLogicalDevice(), vkDestroyPipeline };
	std::vector<Cleaner<VkFramebuffer>> m_swapchainFramebuffers;
	//Cleaner<VkCommandPool> m_commandPool{ m_device.getLogicalDevice(), vkDestroyCommandPool };

	Cleaner<VkBuffer> m_vertexBuffer{ m_device.getLogicalDevice(), vkDestroyBuffer };
	Cleaner<VkDeviceMemory> m_vertexBufferMemory{ m_device.getLogicalDevice(), vkFreeMemory };
	Cleaner<VkBuffer> m_indexBuffer{ m_device.getLogicalDevice(), vkDestroyBuffer };
	Cleaner<VkDeviceMemory> m_indexBufferMemory{ m_device.getLogicalDevice(), vkFreeMemory };
	Cleaner<VkBuffer> m_uniformStagingBuffer{ m_device.getLogicalDevice(), vkDestroyBuffer };
	Cleaner<VkDeviceMemory> m_uniformStagingBufferMemory{ m_device.getLogicalDevice(), vkFreeMemory };
	Cleaner<VkBuffer> m_uniformBuffer{ m_device.getLogicalDevice(), vkDestroyBuffer };
	Cleaner<VkDeviceMemory> m_uniformBufferMemory{ m_device.getLogicalDevice(), vkFreeMemory };
	

	Cleaner<VkDescriptorPool> m_descriptorPool{ m_device.getLogicalDevice(), vkDestroyDescriptorPool };
	VkDescriptorSet m_descriptorSet;

	std::vector<VkCommandBuffer> m_commandBuffers;
	Cleaner<VkSemaphore> m_imageAvailableSemaphore{ m_device.getLogicalDevice(), vkDestroySemaphore };
	Cleaner<VkSemaphore> m_renderFinishedSemaphore{ m_device.getLogicalDevice(), vkDestroySemaphore };

	std::vector<Vertex> m_vertices = 
	{
		{ { -0.5f, -0.5f, 0.0f },{ 1.0f, 0.0f, 1.0f, 1.0f } },
		{ { 0.5f, -0.5f, 0.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { 0.5f, 0.5f, 0.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } },
		{ { -0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f, 1.0f } }
	};
	std::vector<uint16_t> m_indices = 
	{
		0, 1, 2, 2, 3, 0
	};

	std::vector<Camera*> m_cameras;

	const std::vector<const char*> m_validationLayers = {
		"VK_LAYER_LUNARG_core_validation"
	};

	std::vector<const char*> m_extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	};

	//std::vector<const char*> m_physicalDeviceExctentions = {
	//	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	//};

	static std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();
		return buffer;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData) {

		std::string tags;

		switch (flags)
		{
		case VK_DEBUG_REPORT_ERROR_BIT_EXT:
			tags += "[ERROR]";
			break;
		case VK_DEBUG_REPORT_WARNING_BIT_EXT:
			tags += "[WARN]";
			break;
		case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
			tags += "[PERF]";
			break;
		case VK_DEBUG_REPORT_INFORMATION_BIT_EXT:
			tags += "[INFO]";
			break;
		case VK_DEBUG_REPORT_DEBUG_BIT_EXT:
			tags += "[DEBUG]";
			break;
		default:
			tags += "[?]";
		}

		switch (objType)
		{
		case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT:
			tags += "[UNKNOWN]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT:
			tags += "[INSTANCE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT:
			tags += "[PHYSICAL_DEVICE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT:
			tags += "[DEVICE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT:
			tags += "[QUEUE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT:
			tags += "[SEMAPHORE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT:
			tags += "[COMMAND_BUFFER]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT:
			tags += "[FENCE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT:
			tags += "[DEVICE_MEMORY]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT:
			tags += "[BUFFER]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT:
			tags += "[IMAGE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT:
			tags += "[EVENT]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT:
			tags += "[QUERY_POOL]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT:
			tags += "[BUFFER_VIEW]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT:
			tags += "[IMAGE_VIEW]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT:
			tags += "[SHADER_MODULE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT:
			tags += "[PIPELINE_CACHE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT:
			tags += "[PIPELINE_LAYOUT]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT:
			tags += "[RENDER_PASS]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT:
			tags += "[PIPELINE]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT:
			tags += "[DESCRIPTOR_SET_LAYOUT]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT:
			tags += "[SAMPLER]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT:
			tags += "[DESCRIPTOR_POOL]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT:
			tags += "[DESCRIPTOR_SET]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT:
			tags += "[FRAMEBUFFER]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT:
			tags += "[COMMAND_POOL]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT:
			tags += "[SURFACE_KHR]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT:
			tags += "[SWAPCHAIN_KHR]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT:
			tags += "[DEBUG_REPORT]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT:
			tags += "[DISPLAY_KHR]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT:
			tags += "[DISPLAY_MODE_KHR]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT:
			tags += "[OBJECT_TABLE_NVX]";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT:
			tags += "[INDIRECT_COMMANDS_LAYOUT_NVX]";
			break;
		default:
			tags += "[?]";
		}

		std::cout << tags << "[" << obj << "][" << location << "][" << code << "][" << layerPrefix << "] " << msg << std::endl;

		return VK_FALSE;
	}
};

