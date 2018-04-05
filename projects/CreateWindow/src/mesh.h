#pragma once

// warning level 4
// glm uses nameless structs and unions
#pragma warning(disable : 4201)

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "vulkan/vulkan.h"
#include <vector>
#include <array>

struct Vertex {
	glm::vec3 pos;
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
		desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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

class Mesh {
public:
	void initialize(
		VkPhysicalDevice physDevice,
		VkDevice device,
		VkCommandPool commandPool,
		VkQueue graphicsQueue,
		VkExtent2D swapChainExtent,
		VkRenderPass renderPass);
	void updateUniformBuffer(VkExtent2D swapChainExtent);
	void commitCommands(VkCommandBuffer commandBuffer);
	void destroy();
	void recreate(VkExtent2D swapChainExtent, VkRenderPass renderPass);
	inline VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
	void createPipeline(VkExtent2D swapChainExtent, VkRenderPass renderPass);
private:
	void createBuffers();
	void createTextureAndSampler();
	void createDescriptorSet();
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	// association
	VkPhysicalDevice physDevice;
	VkDevice device;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	// composition
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	// maybe should not be here?
	VkDescriptorPool descriptorPool;
};

/*
vkCreateGraphicsPipelines()
	VkGraphicsPipelineCreateInfo
		VkPipelineShaderStageCreateInfo
			VkShaderModule
				vkCreateShaderModule()
					VkShaderModuleCreateInfo
		VkPipelineVertexInputStateCreateInfo
			VkVertexInputBindingDescription
			VkVertexInputAttributeDescription
		VkPipelineInputAssemblyStateCreateInfo
		VkPipelineViewportStateCreateInfo
			VkViewport
			VkRect2D
		VkPipelineRasterizationStateCreateInfo
		VkPipelineMultisampleStateCreateInfo
		VkPipelineDepthStencilStateCreateInfo
		VkPipelineColorBlendStateCreateInfo
		VkPipelineDynamicStateCreateInfo
		VkPipelineLayout
			VkPipelineLayoutCreateInfo
		VkRenderPass
			vkCreateRenderPass()
				VkRenderPassCreateInfo
					VkAttachmentDescription
					VkSubpassDescription
						VkAttachmentReference
					VkSubpassDependency
		VkPipeline
*/