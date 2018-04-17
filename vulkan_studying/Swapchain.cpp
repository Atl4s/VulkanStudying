#include "Swapchain.h"

#undef max
#undef min

Swapchain::Swapchain()
{
}


Swapchain::~Swapchain()
{
}

void Swapchain::createSurface(Window* window, Cleaner<VkInstance>& instance)
{
	SDL_SysWMinfo info = {};
	SDL_GetWindowWMInfo(window->getWindow(), &info);

#if defined(_WIN32)
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKhr = {};
	win32SurfaceCreateInfoKhr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKhr.hwnd = info.info.win.window;
	win32SurfaceCreateInfoKhr.hinstance = GetModuleHandle(nullptr);

	auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");

	m_surface = Cleaner<VkSurfaceKHR>{ instance, vkDestroySurfaceKHR };
	if (!CreateWin32SurfaceKHR || CreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfoKhr,
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

void Swapchain::createSwapchain(Device& device, uint32_t width, uint32_t height)
{
	setSwapchainSupportDetails(device.getPhysicalDevice());

	if(m_swapchain == nullptr)
	{
		m_swapchain = Cleaner<VkSwapchainKHR>{ device.getLogicalDevice(), vkDestroySwapchainKHR };
	}

	VkSurfaceFormatKHR surfaceFormat = getSwapSurfaceFormat(swapchainSupportDetails.formats);
	VkPresentModeKHR presentMode = getSwapPresentMode(swapchainSupportDetails.presentModes);
	VkExtent2D extent = getSwapExtent(swapchainSupportDetails.capabilities, width, height);

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
	vkGetPhysicalDeviceSurfaceSupportKHR(device.getPhysicalDevice(), device.queueFamilyIndices.graphicsFamily, m_surface, &presentSupport);
	if (presentSupport != VK_TRUE)
	{
		throw std::runtime_error("physical device does not support presenting surfaces");
	}

	if (vkCreateSwapchainKHR(device.getLogicalDevice(), &swapChainCreateInfo, nullptr, &newSwapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain");
	}
	m_swapchain = newSwapchain;


	if (vkGetSwapchainImagesKHR(device.getLogicalDevice(), m_swapchain, &imageCount, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to get swapchain images count");
	}
	m_swapchainImages.resize(imageCount);
	if (vkGetSwapchainImagesKHR(device.getLogicalDevice(), m_swapchain, &imageCount, m_swapchainImages.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to get swapchain images");
	}

	m_swapchainImageFormat = surfaceFormat.format;
	m_swapchainExtent = extent;
}

void Swapchain::createImageViews(Device& device)
{
	//auto swapchainImages = m_swapchain.getSwapchainImages();
	m_swapchainImageViews.resize(m_swapchainImages.size(), Cleaner<VkImageView>{device.getLogicalDevice(), vkDestroyImageView});
	//auto imageFormat = m_swapchain.getSwapchainImageFormat();

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

		if (vkCreateImageView(device.getLogicalDevice(), &createInfo, nullptr, m_swapchainImageViews[i].data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views");
		}
	}
}

void Swapchain::setSwapchainSupportDetails(VkPhysicalDevice physicalDevice)
{
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &swapchainSupportDetails.capabilities) != VK_SUCCESS)
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
		swapchainSupportDetails.formats.resize(formatsCount);

		if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatsCount, swapchainSupportDetails.formats.data()) != VK_SUCCESS)
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
		swapchainSupportDetails.presentModes.resize(presentModesCount);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModesCount, swapchainSupportDetails.presentModes.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to retrieve present modes");
		}
	}
}

VkSurfaceFormatKHR Swapchain::getSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats)
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

VkPresentModeKHR Swapchain::getSwapPresentMode(const std::vector<VkPresentModeKHR>& surfacePresentModes)
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

VkExtent2D Swapchain::getSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { width, height };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
