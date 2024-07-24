#ifndef HEADER_MAIN_H
#define HEADER_MAIN_H

#include "definitions.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


class Application
{
public:
	static constexpr u32 kWindowWidth  = 1280;
	static constexpr u32 kWindowHeight = 720;

public:
	bool Run();

private:
	bool InitWindow();

	bool InitVulkan();

	bool CreateInstance();

	bool SetupDebugMessenger();

	bool PickPhysicalDevice();

	bool CreateLogicalDevice();

	bool CreateSurface();

	bool CreateSwapChain();

	bool CreateImageViews();

	bool CreateGraphicsPipeline();

	bool CreateRenderPass();

	void MainLoop();

	void Cleanup();

private:
	GLFWwindow *mWindow;

	VkInstance               mInstance;
	VkPhysicalDevice         mPhysicalDevice;
	VkDevice                 mDevice;
	VkQueue                  mGraphicsQueue;
	VkQueue                  mPresentQueue;
	VkDebugUtilsMessengerEXT mDebugMessenger;
	VkSurfaceKHR             mSurface;

	VkSwapchainKHR       mSwapChain;
	std::vector<VkImage> mSwapChainImages;
	VkFormat             mSwapChainImageFormat;
	VkExtent2D           mSwapChainExtent;

	std::vector<VkImageView> mSwapChainImageViews;

	VkRenderPass     mRenderPass;
	VkPipelineLayout mPipelineLayout;
};


#endif// HEADER_MAIN_H
