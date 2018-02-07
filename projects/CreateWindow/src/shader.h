#pragma once

#include <vulkan/vulkan.h>

#include <vector>
#include <fstream>
#include <cassert>

class Shader {

public:
	Shader(VkDevice device_);
	~Shader();
	void loadVertexShader(const std::string& filename);
	void loadFragmentShader(const std::string& filename);

	inline const VkShaderModule& getVertexShaderModule() const { return vertexShaderModule; }
	inline const VkShaderModule& getFragmentShaderModule() const { return fragmentShaderModule; }

private:
	VkDevice device;
	std::vector<char> vertexShaderCode;
	std::vector<char> fragmentShaderCode;
	VkShaderModule vertexShaderModule;
	VkShaderModule fragmentShaderModule;
};