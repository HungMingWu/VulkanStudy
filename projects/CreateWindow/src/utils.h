#pragma once

#include <vulkan/vulkan.h>
#include <assert.h>
#include <vector>

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

void endSingleTimeCommands(
	VkDevice device, VkCommandPool commandPool,
	VkQueue graphicsQueue, VkCommandBuffer commandBuffer);

uint32_t findMemoryType(
	VkPhysicalDevice physicalDevice,
	uint32_t typeFilter,
	VkMemoryPropertyFlags properties);

void createBuffer(VkPhysicalDevice physDevice, VkDevice device,
	VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	VkBuffer& buffer, VkDeviceMemory& bufferMemory);

void copyBuffer(
	VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,
	VkBuffer src, VkBuffer dst, VkDeviceSize size);

VkFormat findSupportedFormat(VkPhysicalDevice physDevice,
	const std::vector<VkFormat>& candidates,
	VkImageTiling tiling, VkFormatFeatureFlags features);

VkFormat findDepthFormat(VkPhysicalDevice physDevice);

bool hasStencilComponent(VkFormat format);

void createImage(VkPhysicalDevice physDevice, VkDevice device,
	uint32_t width, uint32_t height,
	VkFormat format, VkImageTiling tiling,
	VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
	VkImage& image, VkDeviceMemory& imageMemory);

void transitionImageLayout(
	VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue,
	VkImage image, VkFormat format,
	VkImageLayout oldLayout, VkImageLayout newLayout);