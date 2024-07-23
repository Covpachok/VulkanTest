#include "main.h"

#include "pch/glm.h"
#include "pch/stdlib.h"
#include "utils/logger.h"
#include "vulkan/vulkan_core.h"
//
#include <cstdlib>
#include <cstring>
//
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/**************************************
*       CONST DATA            
**************************************/

constexpr std::array<const char *, 1> kValidationLayers = {
		"VK_LAYER_KHRONOS_validation",
};

constexpr std::array<const char *, 1> kDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

#if NDEBUG
constexpr bool kEnableValidationLayers = false;
#else
constexpr bool kEnableValidationLayers = true;
#endif


/**************************************
*       MISC FUNCTIONS            
**************************************/

bool CheckExtensionsSupport(u32 glfwExtensionCount, const char **glfwExtensions)
{
	u32 supportedExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);

	if (supportedExtensionCount < glfwExtensionCount)
	{
		CLOG_WARN("Not all included extensions are supported by your driver.");
	}

	std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
	vkEnumerateInstanceExtensionProperties(
			nullptr, &supportedExtensionCount, supportedExtensions.data()
	);

	u32 unsupportedCount = 0;
	for (u32 glfwExtensionId = 0; glfwExtensionId < glfwExtensionCount; ++glfwExtensionId)
	{
		const char *glfwExtension      = glfwExtensions[glfwExtensionId];
		bool        extensionSupported = false;
		for (VkExtensionProperties &supportedExtension : supportedExtensions)
		{
			// Skip already checked extension
			if (supportedExtension.extensionName[0] == '\0')
			{
				continue;
			}

			if (strcmp(supportedExtension.extensionName, glfwExtension) == 0)
			{
				supportedExtension.extensionName[0] = '\0';
				extensionSupported                  = true;
				break;
			}
		}

		if (!extensionSupported)
		{
			++unsupportedCount;
			CLOG_ERR("Extension is unsupported: ", glfwExtension);
		}
	}

	return unsupportedCount == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

bool CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	u32 extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
			device, nullptr, &extensionCount, availableExtensions.data()
	);

	std::set<std::string> requiredExtensions(kDeviceExtensions.begin(), kDeviceExtensions.end());
	for (const auto &extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool CheckValidationLayerSupport()
{
	u32 layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char *validationLayer : kValidationLayers)
	{
		bool layerFound = false;
		for (VkLayerProperties &availableLayer : availableLayers)
		{
			if (strcmp(validationLayer, availableLayer.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			CLOG_ERR("Layer not found: ", validationLayer);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

std::vector<const char *> GetRequiredExtensions()
{
	u32          glfwExtensionCount = 0;
	const char **glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (kEnableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT             messageType,
		const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
		void                                       *pUserData
)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		CLOG_WARN("Validation layer: ", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		CLOG_ERR("Validation layer: ", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		CLOG_INFO("Validation layer: ", pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		CLOG_DEBUG("Validation layer: ", pCallbackData->pMessage);
	}

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
		VkInstance                                instance,
		const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
		const VkAllocationCallbacks              *pAllocator,
		VkDebugUtilsMessengerEXT                 *pDebugMessenger
)
{
	auto func = PFN_vkCreateDebugUtilsMessengerEXT(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
	);

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(
		VkInstance                   instance,
		VkDebugUtilsMessengerEXT     debugMessenger,
		const VkAllocationCallbacks *pAllocator
)
{
	auto func = PFN_vkDestroyDebugUtilsMessengerEXT(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
	);

	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
							   // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
							   // | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
							   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
						   | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
						   | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData       = nullptr;
}

struct QueueFamilyIndices
{
	std::optional<u32> graphicsFamily;
	std::optional<u32> presentFamily;

	[[nodiscard]] bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices = {};

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const VkQueueFamilyProperties &queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		++i;
	}

	return indices;
}

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR        capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>   presentModes;
};

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details = {};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	u32 formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	u32 presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
				device, surface, &formatCount, details.presentModes.data()
		);
	}

	return details;
}

bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	VkPhysicalDeviceProperties deviceProperties = {};
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = FindQueueFamilies(device, surface);

	const bool extensionsSupported = CheckDeviceExtensionSupport(device);
	bool       swapChainAdequate   = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);
		swapChainAdequate =
				!swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	const bool result = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
					 && deviceFeatures.geometryShader && indices.IsComplete() && extensionsSupported
					 && swapChainAdequate;
	return result;
}

VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	assert(!availableFormats.empty());

	for (const auto &availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	assert(!availablePresentModes.empty());

	for (const auto &availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D ChooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
	{
		return capabilities.currentExtent;
	}

	i32 width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	VkExtent2D actualExtent = {static_cast<u32>(width), static_cast<u32>(height)};

	actualExtent.width = std::clamp(
			actualExtent.width,               //
			capabilities.minImageExtent.width,//
			capabilities.maxImageExtent.width //
	);

	actualExtent.height = std::clamp(
			actualExtent.height,               //
			capabilities.minImageExtent.height,//
			capabilities.maxImageExtent.height //
	);

	return actualExtent;
}

/**************************************
*       APPLICATION            
**************************************/

bool Application::Run()
{
	CLOG_INFO("Starting...");

	if (InitWindow() == EXIT_FAILURE)
	{
		CLOG_ERR("Failed to initialize window.");
		return EXIT_FAILURE;
	}

	if (InitVulkan() == EXIT_FAILURE)
	{
		CLOG_ERR("Failed to initialize vulkan.");
		return EXIT_FAILURE;
	}

	MainLoop();
	Cleanup();

	return EXIT_SUCCESS;
}

bool Application::InitWindow()
{
	if (glfwInit() != GLFW_TRUE)
	{
		CLOG_ERR("glfwInit failed.");
		return EXIT_FAILURE;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	mWindow = glfwCreateWindow(kWindowWidth, kWindowHeight, "VulkanTest", nullptr, nullptr);
	if (!mWindow)
	{
		CLOG_ERR("glfwCreateWindow failed.");
		return EXIT_FAILURE;
	}

	CLOG_INFO("Window initialized successfully.");
	return EXIT_SUCCESS;
}

bool Application::InitVulkan()
{
	if (CreateInstance() == EXIT_FAILURE)
	{
		CLOG_ERR("CreateInstance failed.");
		return EXIT_FAILURE;
	}

	if (SetupDebugMessenger() == EXIT_FAILURE)
	{
		CLOG_ERR("SetupDebugMessenger failed.");
		return EXIT_FAILURE;
	}

	if (CreateSurface() == EXIT_FAILURE)
	{
		CLOG_ERR("CreateSurface failed.");
		return EXIT_FAILURE;
	}

	if (PickPhysicalDevice() == EXIT_FAILURE)
	{
		CLOG_ERR("PickPhysycalDevice failed.");
		return EXIT_FAILURE;
	}

	if (CreateLogicalDevice() == EXIT_FAILURE)
	{
		CLOG_ERR("CreateLogicalDevice failed.");
		return EXIT_FAILURE;
	}

	if (CreateSwapChain() == EXIT_FAILURE)
	{
		CLOG_ERR("CreateSwapChain failed.");
		return EXIT_FAILURE;
	}

	if (CreateImageViews() == EXIT_FAILURE)
	{
		CLOG_ERR("CreateImageViews failed.");
		return EXIT_FAILURE;
	}

	if (CreateGraphicsPipeline() == EXIT_FAILURE)
	{
		CLOG_ERR("CreateGraphicsPipeline failed.");
		return EXIT_FAILURE;
	}

	CLOG_INFO("Vulkan initialized successfully.");
	return EXIT_SUCCESS;
}

bool Application::CreateInstance()
{
	if (kEnableValidationLayers && CheckValidationLayerSupport() == EXIT_FAILURE)
	{
		CLOG_ERR("Validation layers are enabled, but not available.");
		return EXIT_FAILURE;
	}

	VkApplicationInfo appInfo  = {};
	appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName   = "VulkanTest";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName        = "No Engine";
	appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion         = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo     = &appInfo;

	std::vector<const char *> glfwExtensions = GetRequiredExtensions();

	if (CheckExtensionsSupport(glfwExtensions.size(), glfwExtensions.data()) == EXIT_FAILURE)
	{
		CLOG_ERR("Not all extensions are included.");
		return EXIT_FAILURE;
	}

	createInfo.enabledExtensionCount   = glfwExtensions.size();
	createInfo.ppEnabledExtensionNames = glfwExtensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	if (kEnableValidationLayers)
	{
		createInfo.enabledLayerCount   = kValidationLayers.size();
		createInfo.ppEnabledLayerNames = kValidationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}


	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
	{
		CLOG_ERR("Vulkan instance creation failed.");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

bool Application::SetupDebugMessenger()
{
	if (!kEnableValidationLayers)
	{
		return EXIT_SUCCESS;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(mInstance, &createInfo, nullptr, &mDebugMessenger)
		!= VK_SUCCESS)
	{
		CLOG_ERR("Failed to create debug utils messenger.");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

bool Application::PickPhysicalDevice()
{
	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		CLOG_ERR("Failed to find any GPU.");
		return EXIT_FAILURE;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());

	for (VkPhysicalDevice &device : devices)
	{
		if (IsDeviceSuitable(device, mSurface))
		{
			mPhysicalDevice = device;
			break;
		}
	}

	if (mPhysicalDevice == VK_NULL_HANDLE)
	{
		CLOG_ERR("Failed to find suitable GPU.");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

bool Application::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice, mSurface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<u32>                        uniqueQueueFamilies = {
            indices.graphicsFamily.value(), indices.presentFamily.value()
    };

	float queuePriority = 1.0f;
	for (u32 queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex        = queueFamily;
		queueCreateInfo.queueCount              = 1;
		queueCreateInfo.pQueuePriorities        = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo   = {};
	createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos    = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = queueCreateInfos.size();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();
	createInfo.enabledExtensionCount   = kDeviceExtensions.size();

	if (kEnableValidationLayers)
	{
		createInfo.ppEnabledLayerNames = kValidationLayers.data();
		createInfo.enabledLayerCount   = kValidationLayers.size();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice))
	{
		CLOG_ERR("Failed to create logical device.");
		return EXIT_FAILURE;
	}


	vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, indices.presentFamily.value(), 0, &mPresentQueue);

	return EXIT_SUCCESS;
}

bool Application::CreateSurface()
{
	if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &mSurface) != VK_SUCCESS)
	{
		CLOG_ERR("Failed to create window surface.");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

bool Application::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(mPhysicalDevice, mSurface);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR   presentMode   = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D         extent        = ChooseSwapExtent(mWindow, swapChainSupport.capabilities);

	u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0
		&& imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};

	createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface          = mSurface;
	createInfo.minImageCount    = imageCount;
	createInfo.imageFormat      = surfaceFormat.format;
	createInfo.imageColorSpace  = surfaceFormat.colorSpace;
	createInfo.imageExtent      = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice, mSurface);
	u32 queueFamilyIndices[]   = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices   = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices   = nullptr;
	}

	createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode    = presentMode;
	createInfo.clipped        = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
	{
		return EXIT_FAILURE;
	}

	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, nullptr);
	mSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapChain, &imageCount, mSwapChainImages.data());

	mSwapChainImageFormat = surfaceFormat.format;
	mSwapChainExtent      = extent;

	return EXIT_SUCCESS;
}

bool Application::CreateImageViews()
{
	mSwapChainImageViews.resize(mSwapChainImages.size());
	for (u32 i = 0; i < mSwapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo = {};

		createInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image    = mSwapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format   = mSwapChainImageFormat;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel   = 0;
		createInfo.subresourceRange.levelCount     = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount     = 1;

		if (vkCreateImageView(mDevice, &createInfo, nullptr, &mSwapChainImageViews[i])
			!= VK_SUCCESS)
		{
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

bool Application::CreateGraphicsPipeline()
{
    return EXIT_SUCCESS;
}

void Application::MainLoop()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		glfwPollEvents();
	}
}

void Application::Cleanup()
{
	CLOG_INFO("Cleaning up...");

	for (auto *imageView : mSwapChainImageViews)
	{
		vkDestroyImageView(mDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);

	vkDestroyDevice(mDevice, nullptr);

	if (kEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

	vkDestroyInstance(mInstance, nullptr);

	glfwDestroyWindow(mWindow);

	glfwTerminate();

	CLOG_INFO("Clean up successfull");
}


/**************************************
*       MAIN            
**************************************/

i32 main()
{
	Application app = {};

	i32 exitCode = app.Run();

	return exitCode;
}
