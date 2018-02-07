#include "shader.h"

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		assert(0);
	}
	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		assert(0);
	}
	return shaderModule;
}

Shader::Shader(VkDevice device_) : device(device_) {
	//
}

Shader::~Shader() {
	vkDestroyShaderModule(device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
}

void Shader::loadVertexShader(const std::string& filename) {
	vertexShaderCode = std::move(readFile(filename));
	vertexShaderModule = createShaderModule(device, vertexShaderCode);
}

void Shader::loadFragmentShader(const std::string& filename) {
	fragmentShaderCode = std::move(readFile(filename));
	fragmentShaderModule = createShaderModule(device, fragmentShaderCode);
}