#pragma once

#include <vulkan/vulkan.h>
#include <assert.h>

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format);

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
