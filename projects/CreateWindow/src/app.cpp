#include "app.h"
#include "shader.h"
#include "mesh.h"
#include "log.h"
#include "utils.h"
#include <cassert>
#include <vector>
#include <set>
#include <iostream>
#include <algorithm>

#pragma comment(lib, "vulkan-1.lib")

const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	static_cast<void>(flags);
	static_cast<void>(objType);
	static_cast<void>(obj);
	static_cast<void>(location);
	static_cast<void>(code);
	static_cast<void>(layerPrefix);
	static_cast<void>(userData);

	std::cerr << "validation layer: " << msg << std::endl;
	return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugReportCallbackEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugReportCallbackEXT(
	VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

void Application::run() {
	FUNCNAME()
	initWindow();
	initVulkan();
	mainLoop();
	destroy();
}

void Application::initWindow() {
	FUNCNAME()
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(1600, 900, "Title", nullptr, nullptr);

	if (glfwVulkanSupported() != GLFW_TRUE) {
		assert(0);
	}
	LOG("- a GLFW window has been initialized");

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, Application::onWindowResized);
}

void Application::onWindowResized(GLFWwindow* window, int width, int height) {
	static_cast<void>(width);
	static_cast<void>(height);
	Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->recreateSwapChain();
}

void Application::initVulkan() {
	FUNCNAME()
	createVkInstance();
	setupDebugCallback();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createCommandPool();
	createDepthResources();
	createFramebuffers();
	create3DModels();
	createCommandBuffers();
	createSemaphores();
}

void Application::mainLoop() {
	static bool first = true;
	if (first) {
		FUNCNAME()
		first = false;
		LOG("- Drawing something...")
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(device);
}

void Application::destroy() {
	FUNCNAME();
	cleanupSwapChain();
	{
		triangle.destroy();
	}
	vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
	DestroyDebugReportCallbackEXT(instance, callback, nullptr);
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::getRequiredExtensions(std::vector<const char*>& extensions) {
	FUNCNAME()
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	extensions.assign(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
}

void Application::createVkInstance() {
	FUNCNAME()
	LOG("- call vkCreateInstance() with VkApplicationInfo, VkInstanceCreateInfo to create a VkInstance")
	if (enableValidationLayers && !checkValidationLayerSupport()) assert(0);

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "DrawTriangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> extensions;
	getRequiredExtensions(extensions);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	assert(result == VK_SUCCESS);
}

bool Application::checkValidationLayerSupport() {
	FUNCNAME()
	LOG("- call vkEnumerateInstanceLayerProperties() to get a list of VkLayerProperties")
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}
	return true;
}

void Application::setupDebugCallback() {
	FUNCNAME()
	LOG("- call CreateDebugReportCallbackEXT()")
	if (!enableValidationLayers) return;

	VkDebugReportCallbackCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback) != VK_SUCCESS) {
		assert(0);
	}
}

QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice physDevice) {
	FUNCNAME()
	LOG("- call vkGetPhysicalDeviceQueueFamilyProperties() to get a list of VkQueueFamilyProperties")
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, queueFamilies.data());
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}
		VkBool32 presentSupport = false;
		LOG("- call vkGetPhysicalDeviceSurfaceSupportKHR() to check present support")
		vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}
		if (indices.isComplete()) {
			break;
		}
		++i;
	}
	return indices;
}

bool Application::isDeviceSuitable(VkPhysicalDevice physDevice) {
	FUNCNAME()
	QueueFamilyIndices indices = findQueueFamilies(physDevice);
	
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(physDevice, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(physDevice, &deviceFeatures);
	
	bool extensionsSupported = checkDeviceExtensionSupport(physDevice);
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physDevice);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	return indices.isComplete() && extensionsSupported
		&& swapChainAdequate && deviceFeatures.samplerAnisotropy;
}

bool Application::checkDeviceExtensionSupport(VkPhysicalDevice physDevice) {
	FUNCNAME()
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount, availableExtensions.data());
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}

void Application::pickPhysicalDevice() {
	FUNCNAME()
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		assert(0);
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	for (const auto& physDevice : devices) {
		if (isDeviceSuitable(physDevice)) {
			physicalDevice = physDevice;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE) {
		assert(0);
	}
}

void Application::createLogicalDevice() {
	FUNCNAME()
	LOG("1. find queue families")
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };
	
	float queuePriority = 1.0f; // in range 0.0 ~ 1.0
	for (int queueFamily : uniqueQueueFamilies) {
		// Describes # of queues we want for a single queue family
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	LOG("2. call vkCreateDevice() with VkPhysicalDevice, VkDeviceCreateInfo to create a VkDevice")
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		assert(0);
	}

	LOG("3. call vkGetDeviceQueue() to get a desired queue from a queue families")
	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);
}

void Application::createSurface() {
	FUNCNAME()
	LOG("- call glfwCreateWindowSurface() to create a VkSurfaceKHR")
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		assert(0);
	}
}

SwapChainSupportDetails Application::querySwapChainSupport(VkPhysicalDevice physDevice) {
	FUNCNAME()
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice, surface, &details.capabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physDevice, surface, &formatCount, details.formats.data());
	}
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physDevice, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	FUNCNAME()
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	FUNCNAME()
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	FUNCNAME()
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		int WIDTH, HEIGHT;
		glfwGetWindowSize(window, &WIDTH, &HEIGHT);
		const auto& minExtent = capabilities.minImageExtent;
		const auto& maxExtent = capabilities.maxImageExtent;
		VkExtent2D actualExtent{ static_cast<uint32_t>(WIDTH), static_cast<uint32_t>(HEIGHT) };
		actualExtent.width = std::max(minExtent.width, std::min(maxExtent.width, actualExtent.width));
		actualExtent.height = std::max(minExtent.height, std::min(maxExtent.height, actualExtent.height));
		return actualExtent;
	}
}

void Application::createSwapChain() {
	FUNCNAME()
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	// maxImageCount = 0 means there's no limit besides memory requirements
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	// 1 unless developming a stereoscopic 3D application
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(indices.graphicsFamily), static_cast<uint32_t>(indices.presentFamily) };
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		// best performance. an image is owned by one queue family at a time
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		assert(0);
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void Application::createImageViews() {
	FUNCNAME()
	LOG("- create image views (VkImageView) for swap chain images (VkImage)")
	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); ++i) {
		swapChainImageViews[i]
			= createImageView(device, swapChainImages[i], swapChainImageFormat,
				VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void Application::createRenderPass() {
	FUNCNAME()

		VkAttachmentDescription colorAttachment {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat(physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = {
		colorAttachment, depthAttachment
	};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		assert(0);
	}
}

void Application::createFramebuffers() {
	FUNCNAME()
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); ++i) {
		std::array<VkImageView, 2> attachments = { swapChainImageViews[i], depthImageView };
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			assert(0);
		}
	}
}

void Application::createCommandPool() {
	FUNCNAME()
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = 0;
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		assert(0);
	}
}

void Application::create3DModels(bool isRecreate) {
	FUNCNAME()
	if (isRecreate) {
		triangle.recreate(swapChainExtent, renderPass);
	} else {
		triangle.initialize(physicalDevice, device,
			commandPool, graphicsQueue, swapChainExtent, renderPass);
	}
}

void Application::createCommandBuffers() {
	FUNCNAME()
	commandBuffers.resize(swapChainFramebuffers.size());
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		assert(0);
	}
	for (size_t i = 0; i < commandBuffers.size(); ++i) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;
		vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = swapChainExtent;
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			triangle.commitCommands(commandBuffers[i]);
		}
		vkCmdEndRenderPass(commandBuffers[i]);
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			assert(0);
		}
	}
}

void Application::createSemaphores() {
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS) {
		assert(0);
	}
	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {
		assert(0);
	}
}

void Application::drawFrame() {
	// TODO: update logic here
	triangle.updateUniformBuffer(swapChainExtent);

	vkQueueWaitIdle(presentQueue);

	// 1. Acquire an image from the swap chain
	// 2. Execute the command buffer with that image as attachment in the framebuffer
	// 3. Return the image to the swap chain for presentation
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, std::numeric_limits<uint64_t>::max(),
		imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		assert(0);
	}
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		assert(0);
	}
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;
	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapChain();
	} else if (result != VK_SUCCESS) {
		assert(0);
	}
	vkQueueWaitIdle(presentQueue);
}

void Application::recreateSwapChain() {
	FUNCNAME()
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (width == 0 || height == 0) return;
	vkDeviceWaitIdle(device);
	cleanupSwapChain();
	createSwapChain();
	createImageViews();
	createRenderPass();
	//createGraphicsPipeline();
	create3DModels(true);
	createDepthResources();
	createFramebuffers();
	createCommandBuffers();
}

void Application::cleanupSwapChain() {
	FUNCNAME()
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	vkDestroyRenderPass(device, renderPass, nullptr);
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}
	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void Application::createDepthResources() {
	VkFormat depthFormat = findDepthFormat(physicalDevice);
	createImage(physicalDevice, device, swapChainExtent.width, swapChainExtent.height,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage, depthImageMemory);
	depthImageView = createImageView(device, depthImage, depthFormat,
		VK_IMAGE_ASPECT_DEPTH_BIT);
	transitionImageLayout(
		device, commandPool, graphicsQueue,
		depthImage, depthFormat,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}