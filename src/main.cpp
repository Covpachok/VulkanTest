#include "main.h"

#include "GLFW/glfw3.h"
#include "pch/glm.h"
#include "pch/stdlib.h"
#include "utils/logger.h"
#include "vulkan/vulkan_core.h"

#include <climits>
#include <cstdlib>
#include <cstring>

/**************************************
*       CONST DATA            
**************************************/

constexpr std::array<const char *, 1> kValidationLayers = {
		"VK_LAYER_KHRONOS_validation",
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

	bool IsComplete() const
	{
		return graphicsFamily.has_value();
	}
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
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

		if (indices.IsComplete())
		{
			break;
		}

		++i;
	}

	return indices;
}

bool IsDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties = {};
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = FindQueueFamilies(device);

	bool result = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
			   && deviceFeatures.geometryShader && indices.IsComplete();
	return result;
}


/**************************************
*       APPLICATION            
**************************************/

bool Application::Run()
{
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
		if (IsDeviceSuitable(device))
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
	QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex        = indices.graphicsFamily.value();
	queueCreateInfo.queueCount              = 1;

	float queuePriority              = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo   = {};
	createInfo.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos    = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = 0;

	if (kEnableValidationLayers)
	{
		createInfo.enabledLayerCount   = kValidationLayers.size();
		createInfo.ppEnabledLayerNames = kValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

    if(vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice))
    {
        CLOG_ERR("Failed to create logical device.");
        return EXIT_FAILURE;
    }


    vkGetDeviceQueue(mDevice, indices.graphicsFamily.value(), 0, &mGraphicsQueue);

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
    vkDestroyDevice(mDevice, nullptr);

	if (kEnableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(mInstance, mDebugMessenger, nullptr);
	}

	vkDestroyInstance(mInstance, nullptr);

	glfwDestroyWindow(mWindow);

	glfwTerminate();
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
