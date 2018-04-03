#pragma once

// warning level 4
// glm uses nameless structs and unions
#pragma warning(disable : 4201)

#include "glm/glm.hpp"
#include "vulkan/vulkan.h"
#include <vector>
#include <array>

struct Vertex {
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 texcoord;
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription desc{};
		desc.binding = 0;
		desc.stride = sizeof(Vertex);
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return desc;
	}
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> desc;
		desc[0].binding = 0;
		desc[0].location = 0;
		desc[0].format = VK_FORMAT_R32G32_SFLOAT;
		desc[0].offset = offsetof(Vertex, pos);
		desc[1].binding = 0;
		desc[1].location = 1;
		desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[1].offset = offsetof(Vertex, color);
		desc[2].binding = 0;
		desc[2].location = 2;
		desc[2].format = VK_FORMAT_R32G32_SFLOAT;
		desc[2].offset = offsetof(Vertex, texcoord);
		return desc;
	}
};

class Triangle {
public:
	void initialize(VkPhysicalDevice physDevice, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkDescriptorSetLayout);
	void updateUniformBuffer();
	void commitCommands(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	void destroy();
private:
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	VkDevice device;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
};