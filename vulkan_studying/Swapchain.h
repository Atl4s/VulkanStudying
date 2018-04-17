#pragma once

#include "Headers.h"
#include "Cleaner.h"
#include "Device.h"
#include <vulkan/vulkan.h>

class Swapchain
{
public:
	Swapchain();
	~Swapchain();

	struct
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	} swapchainSupportDetails;

	void createSurface(Window* window, Cleaner<VkInstance>& instance);
	void createSwapchain(Device& device, uint32_t width, uint32_t height);
	void createImageViews(Device& device);

	Cleaner<VkSurfaceKHR>& getSurface() { return m_surface; }
	Cleaner<VkSwapchainKHR>& getSwapchain() { return m_swapchain; }
	std::vector<VkImage>& getSwapchainImages() { return m_swapchainImages; }
	VkFormat& getSwapchainImageFormat() { return  m_swapchainImageFormat; }
	VkExtent2D& getSwapchainExtent() { return m_swapchainExtent; }
	std::vector<Cleaner<VkImageView>>& getSwapchainImageViews() { return m_swapchainImageViews; }

private:
	void setSwapchainSupportDetails(VkPhysicalDevice physicalDevice);
	VkSurfaceFormatKHR getSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& surfaceFormats);
	VkPresentModeKHR getSwapPresentMode(const std::vector<VkPresentModeKHR>& surfacePresentModes);
	VkExtent2D getSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

	Cleaner<VkSurfaceKHR> m_surface;
	Cleaner<VkSwapchainKHR> m_swapchain;
	std::vector<VkImage> m_swapchainImages;
	VkFormat m_swapchainImageFormat;
	VkExtent2D m_swapchainExtent;
	std::vector<Cleaner<VkImageView>> m_swapchainImageViews;
};

