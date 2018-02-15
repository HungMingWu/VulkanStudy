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
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription desc{};
		desc.binding = 0;
		desc.stride = sizeof(Vertex);
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return desc;
	}
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> desc;
		desc[0].binding = 0;
		desc[0].location = 0;
		desc[0].format = VK_FORMAT_R32G32_SFLOAT;
		desc[0].offset = offsetof(Vertex, pos);
		desc[1].binding = 0;
		desc[1].location = 1;
		desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[1].offset = offsetof(Vertex, color);
		return desc;
	}
};

const std::vector<Vertex> vertices = {
	{ { 0.0f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
	{ { -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } }
};
