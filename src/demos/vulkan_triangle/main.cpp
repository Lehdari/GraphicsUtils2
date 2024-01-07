//
// Project: GraphicsUtils2
// File: main.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include <gu2_os/App.hpp>
#include <gu2_os/Window.hpp>
#include <gu2_vulkan/backend.hpp>
#include <gu2_vulkan/QueryWrapper.hpp>

#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <set>


bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
    std::vector<VkLayerProperties> availableLayers = gu2::vkEnumerateInstanceLayerProperties();

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) != 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {

    printf("Validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


struct VulkanSettings {
    bool                        enableValidationLayers  {true};
    std::vector<const char*>    validationLayers        {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*>    deviceExtensions        {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

struct VulkanQueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct VulkanSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};


class VulkanWindow : public gu2::Window<VulkanWindow> {
public:
    VulkanWindow(const gu2::WindowSettings& windowSettings, const VulkanSettings& vulkanSettings) :
        Window<VulkanWindow>    (windowSettings),
        _vulkanSettings         (vulkanSettings),
        _vulkanPhysicalDevice   (VK_NULL_HANDLE)
    {
        initVulkan();
    }

    ~VulkanWindow()
    {
        for (auto& imageView : _vulkanSwapChainImageViews) {
            vkDestroyImageView(_vulkanDevice, imageView, nullptr);
        }
        vkDestroySwapchainKHR(_vulkanDevice, _vulkanSwapChain, nullptr);
        vkDestroyDevice(_vulkanDevice, nullptr);
        if (_vulkanSettings.enableValidationLayers)
            DestroyDebugUtilsMessengerEXT(_vulkanInstance, _vulkanDebugMessenger, nullptr);
        vkDestroySurfaceKHR(_vulkanInstance, _vulkanSurface, nullptr);
        vkDestroyInstance(_vulkanInstance, nullptr);
    }

    void initVulkan()
    {
        if (_vulkanSettings.enableValidationLayers && !checkValidationLayerSupport(_vulkanSettings.validationLayers)) {
            throw std::runtime_error("Requested Vulkan validation layers not supported!");
        }
        createInstance();
        setupDebugMessenger();
        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
    }

    void createInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "GraphicsUtils2 Vulkan Triangle Demo";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "GraphicsUtils2";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Setup instance extensions
        auto extensions = gu2::getVulkanInstanceExtensions();
        #ifdef VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif
        if (_vulkanSettings.enableValidationLayers) // enable debug utils if validation layers are requested
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Setup validation layers in case they are requested
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (_vulkanSettings.enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_vulkanSettings.validationLayers.size());
            createInfo.ppEnabledLayerNames = _vulkanSettings.validationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        auto createStatus = vkCreateInstance(&createInfo, nullptr, &_vulkanInstance);
        if (createStatus != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
    }

    void setupDebugMessenger()
    {
        if (!_vulkanSettings.enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(_vulkanInstance, &createInfo, nullptr, &_vulkanDebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    void createSurface()
    {
        if (!gu2::createWindowVulkanSurface(_window, _vulkanInstance, nullptr, &_vulkanSurface)) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void selectPhysicalDevice()
    {
        auto devices = gu2::vkEnumeratePhysicalDevices(_vulkanInstance);
        if (devices.empty())
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");

        // Select the most suitable device
        uint32_t maxSuitability = 0;
        for (const auto& device : devices) {
            uint32_t suitability = deviceSuitability(device);
            if (suitability > maxSuitability) {
                _vulkanPhysicalDevice = device;
                maxSuitability = suitability;
            }
        }

        if (_vulkanPhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    uint32_t deviceSuitability(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // Rank the device according to its properties and features
        uint32_t suitability = 1; // minimal suitability
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            suitability += 1000;

        auto familyIndices = findQueueFamilies(device);
        if (!familyIndices.isComplete())
            return 0; // not suitable

        bool extensionsSupported = checkDeviceExtensionSupport(device);
        if (!extensionsSupported)
            return 0; // not suitable

        auto swapChainSupport =  querySwapChainSupport(device);
        bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        if (!swapChainAdequate)
            return 0; // not suitable

        return suitability;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        auto availableExtensions = gu2::vkEnumerateDeviceExtensionProperties(device, nullptr);

        std::set<std::string> requiredExtensions(_vulkanSettings.deviceExtensions.begin(),
            _vulkanSettings.deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty(); // all required extensions found
    }

    VulkanQueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        VulkanQueueFamilyIndices indices;

        auto queueFamilies = gu2::vkGetPhysicalDeviceQueueFamilyProperties(device);

        VkBool32 presentSupport = false;
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _vulkanSurface, &presentSupport);
            if (presentSupport)
                indices.presentFamily = i; // might be the same as graphicsFamily, see https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface

            if (indices.isComplete())
                break;

            ++i;
        }

        return indices;
    }

    VulkanSwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        VulkanSwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _vulkanSurface, &details.capabilities);
        details.formats = gu2::vkGetPhysicalDeviceSurfaceFormatsKHR(device, _vulkanSurface);
        details.presentModes = gu2::vkGetPhysicalDeviceSurfacePresentModesKHR(device, _vulkanSurface);
        return details;
    }

    void createLogicalDevice()
    {
        auto familyIndices = findQueueFamilies(_vulkanPhysicalDevice);

        // Graphics queue creation info
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Device features to be used
        VkPhysicalDeviceFeatures deviceFeatures{};

        // Logical device creation info
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(_vulkanSettings.deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = _vulkanSettings.deviceExtensions.data();
        if (_vulkanSettings.enableValidationLayers) { // ignored in modern vulkan implementations
            createInfo.enabledLayerCount = static_cast<uint32_t>(_vulkanSettings.validationLayers.size());
            createInfo.ppEnabledLayerNames = _vulkanSettings.validationLayers.data();
        }
        else
            createInfo.enabledLayerCount = 0;

        if (vkCreateDevice(_vulkanPhysicalDevice, &createInfo, nullptr, &_vulkanDevice) != VK_SUCCESS)
            throw std::runtime_error("failed to create logical device!");

        vkGetDeviceQueue(_vulkanDevice, familyIndices.graphicsFamily.value(), 0, &_vulkanGraphicsQueue);
        vkGetDeviceQueue(_vulkanDevice, familyIndices.presentFamily.value(), 0, &_vulkanPresentQueue);
    }

    VkSurfaceFormatKHR selectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR selectSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // use triple buffering if possible
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR; // double buffering (vsync)
    }

    VkExtent2D selectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            getWindowFramebufferSize(_window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    void createSwapChain() {
        VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(_vulkanPhysicalDevice);

        auto surfaceFormat = selectSwapSurfaceFormat(swapChainSupport.formats);
        auto presentMode = selectSwapPresentMode(swapChainSupport.presentModes);
        auto extent = selectSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = _vulkanSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto indices = findQueueFamilies(_vulkanPhysicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(_vulkanDevice, &createInfo, nullptr, &_vulkanSwapChain) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain!");
        }

        _vulkanSwapChainImages = gu2::vkGetSwapchainImagesKHR(_vulkanDevice, _vulkanSwapChain);
        _vulkanSwapChainImageFormat = surfaceFormat.format;
        _vulkanSwapChainExtent = extent;
    }

    void createImageViews() {
        _vulkanSwapChainImageViews.resize(_vulkanSwapChainImages.size());
        for (size_t i = 0; i<_vulkanSwapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = _vulkanSwapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = _vulkanSwapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_vulkanDevice, &createInfo, nullptr, &_vulkanSwapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }
        }
    }

    void handleEvent(const gu2::Event& event)
    {
        switch (event.type) {
            case gu2::Event::WINDOW:
                switch (event.window.event) {
                    case gu2::WindowEventID::CLOSE: close(); return;
                    default: break;
                }
                break;
            case gu2::Event::KEY:
                switch (event.key.sym.scancode) {
                    case gu2::ScanCode::ESCAPE: close(); return;
                    default: break;
                }
                break;
            default: break;
        }
    }

    void render()
    {
        // TODO
    }

private:
    VulkanSettings              _vulkanSettings;
    VkInstance                  _vulkanInstance;
    VkDebugUtilsMessengerEXT    _vulkanDebugMessenger;
    VkSurfaceKHR                _vulkanSurface;
    VkPhysicalDevice            _vulkanPhysicalDevice;
    VkDevice                    _vulkanDevice;
    VkQueue                     _vulkanGraphicsQueue;
    VkQueue                     _vulkanPresentQueue;
    VkSwapchainKHR              _vulkanSwapChain;
    std::vector<VkImage>        _vulkanSwapChainImages;
    std::vector<VkImageView>    _vulkanSwapChainImageViews;
    VkFormat                    _vulkanSwapChainImageFormat;
    VkExtent2D                  _vulkanSwapChainExtent;
};


int main(void)
{
    gu2::WindowSettings windowSettings;
    windowSettings.name = "Hello Vulkan Triangle!";
    windowSettings.w = 800;
    windowSettings.h = 600;

    VulkanSettings vulkanSettings;
    #ifdef NDEBUG
    vulkanSettings.enableValidationLayers = false;
    #else
    vulkanSettings.enableValidationLayers = true;
    #endif

    VulkanWindow window(windowSettings, vulkanSettings);

    gu2::App::addWindow(&window);

    // Main loop
    try {
        while (gu2::App::update());
    }
    catch (const std::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}