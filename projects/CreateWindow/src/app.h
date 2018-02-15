#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

// vkCreateXXX -> vkDestroyXXX
// vkAllocateXXX -> vkFreeXXX

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;
	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Application {
	
public:
	void run();

private:
	void initWindow();
	void initVulkan();
	void mainLoop();
	void drawFrame();
	void destroy();

	static void onWindowResized(GLFWwindow* window, int width, int height);

	void createVkInstance();
	bool checkValidationLayerSupport();
	void getRequiredExtensions(std::vector<const char*>& extensions);
	void setupDebugCallback();
	void createSurface();
	void pickPhysicalDevice();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physDevice);
	bool isDeviceSuitable(VkPhysicalDevice physDevice);
	bool checkDeviceExtensionSupport(VkPhysicalDevice physDevice);
	void createLogicalDevice();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physDevice);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createVertexBuffer();
	void createCommandBuffers();
	void createSemaphores();
	void cleanupSwapChain();
	void recreateSwapChain();

private:
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

private:
	GLFWwindow* window = nullptr;
	VkInstance instance;
	VkDebugReportCallbackEXT callback;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkPipelineLayout pipelineLayout;
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;

	// TODO: move to 3d entity class
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

#ifdef _DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif

};