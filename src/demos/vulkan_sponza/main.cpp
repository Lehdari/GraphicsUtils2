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
#include <gu2_util/GLTFLoader.hpp>
#include <gu2_util/MathTypes.hpp>
#include <gu2_util/Image.hpp>
#include <gu2_util/Typedef.hpp>
#include <gu2_vulkan/backend.hpp>
#include <gu2_vulkan/Mesh.hpp>
#include <gu2_vulkan/Texture.hpp>
#include <gu2_vulkan/QueryWrapper.hpp>
#include <gu2_vulkan/Util.hpp>
#include <gu2_vulkan/VulkanSettings.hpp>

#include <vulkan/vulkan.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <limits>
#include <optional>
#include <set>
#include <iostream>
#include <memory>


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

std::vector<char> readFile(const gu2::Path& filename)
{
    if (!std::filesystem::exists(filename))
        throw std::runtime_error("File " + filename.string() + " does not exist");
    if (!std::filesystem::is_regular_file(filename))
        throw std::runtime_error(filename.string() + " is not a file");

    FILE *f = fopen(GU2_PATH_TO_STRING(filename), "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    std::vector<char> buffer(fsize);
    fread(buffer.data(), fsize, 1, f);
    fclose(f);

    return buffer;
}


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
    VulkanWindow(const gu2::WindowSettings& windowSettings, const gu2::VulkanSettings& vulkanSettings) :
        Window<VulkanWindow>    (windowSettings),
        _vulkanSettings         (vulkanSettings),
        _vulkanPhysicalDevice   (VK_NULL_HANDLE),
        _vertexPositionData     {
                                    {-1.0f, -1.0f, -1.0f},
                                    {1.0f, -1.0f, -1.0f},
                                    {-1.0f, -1.0f, 1.0f},
                                    {1.0f, -1.0f, 1.0f},
                                    {1.0f, -1.0f, -1.0f},
                                    {1.0f, 1.0f, -1.0f},
                                    {1.0f, -1.0f, 1.0f},
                                    {1.0f, 1.0f, 1.0f},
                                    {1.0f, 1.0f, -1.0f},
                                    {-1.0f, 1.0f, -1.0f},
                                    {1.0f, 1.0f, 1.0f},
                                    {-1.0f, 1.0f, 1.0f},
                                    {-1.0f, 1.0f, -1.0f},
                                    {-1.0f, -1.0f, -1.0f},
                                    {-1.0f, 1.0f, 1.0f},
                                    {-1.0f, -1.0f, 1.0f},
                                    {-1.0f, -1.0f, 1.0f},
                                    {1.0f, -1.0f, 1.0f},
                                    {-1.0f, 1.0f, 1.0f},
                                    {1.0f, 1.0f, 1.0f},
                                    {1.0f, -1.0f, -1.0f},
                                    {-1.0f, -1.0f, -1.0f},
                                    {1.0f, 1.0f, -1.0f},
                                    {-1.0f, 1.0f, -1.0f}
                                },
        _vertexColorData        {
                                    {0.0f, 0.0f, 0.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {0.0f, 0.0f, 1.0f},
                                    {1.0f, 0.0f, 1.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {1.0f, 1.0f, 0.0f},
                                    {1.0f, 0.0f, 1.0f},
                                    {1.0f, 1.0f, 1.0f},
                                    {1.0f, 1.0f, 0.0f},
                                    {0.0f, 1.0f, 0.0f},
                                    {1.0f, 1.0f, 1.0f},
                                    {0.0f, 1.0f, 1.0f},
                                    {0.0f, 1.0f, 0.0f},
                                    {0.0f, 0.0f, 0.0f},
                                    {0.0f, 1.0f, 1.0f},
                                    {0.0f, 0.0f, 1.0f},
                                    {0.0f, 0.0f, 1.0f},
                                    {1.0f, 0.0f, 1.0f},
                                    {0.0f, 1.0f, 1.0f},
                                    {1.0f, 1.0f, 1.0f},
                                    {1.0f, 0.0f, 0.0f},
                                    {0.0f, 0.0f, 0.0f},
                                    {1.0f, 1.0f, 0.0f},
                                    {0.0f, 1.0f, 0.0f}
                                },
        _vertexTexCoordData     {
                                    {0.0f, 0.0f},
                                    {0.0f, 0.25f},
                                    {0.25f, 0.0f},
                                    {0.25f, 0.25f},
                                    {0.5f, 0.0f},
                                    {0.5f, 0.25f},
                                    {0.75f, 0.0f},
                                    {0.75f, 0.25f},
                                    {0.75f, 0.25f},
                                    {0.75f, 0.5f},
                                    {1.0f, 0.25f},
                                    {1.0f, 0.5f},
                                    {0.25f, 0.75f},
                                    {0.25f, 1.0f},
                                    {0.5f, 0.75f},
                                    {0.5f, 1.0f},
                                    {0.75f, 0.75f},
                                    {0.75f, 1.0f},
                                    {1.0f, 0.75f},
                                    {1.0f, 1.0f},
                                    {0.0f, 0.5f},
                                    {0.0f, 0.75f},
                                    {0.25f, 0.5f},
                                    {0.25f, 0.75f}
                                },
        _indexData              {0, 1, 3, 0, 3, 2,
                                 4, 5, 7, 4, 7, 6,
                                 8, 9, 11, 8, 11, 10,
                                 12, 13, 15, 12, 15, 14,
                                 16, 17, 19, 16, 19, 18,
                                 20, 21, 23, 20, 23, 22},
        _currentFrame           (0)
    {
        initVulkan();
    }

    ~VulkanWindow()
    {
        // Wait for the Vulkan device to finish its tasks
        vkDeviceWaitIdle(_vulkanDevice);
        _mesh.reset();
        _texture.reset();
        cleanupSwapChain();
        for (int i=0; i<_vulkanSettings.framesInFlight; ++i) {
            vkDestroySemaphore(_vulkanDevice, _vulkanImageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(_vulkanDevice, _vulkanRenderFinishedSemaphores[i], nullptr);
            vkDestroyFence(_vulkanDevice, _vulkanInFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(_vulkanDevice, _vulkanCommandPool, nullptr);
        vkDestroyPipeline(_vulkanDevice, _vulkanGraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(_vulkanDevice, _vulkanPipelineLayout, nullptr);
        vkDestroyRenderPass(_vulkanDevice, _vulkanRenderPass, nullptr);
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
        createRenderPass();
        createCommandPool();
        _texture = std::make_unique<gu2::Texture>(_vulkanPhysicalDevice, _vulkanDevice, _vulkanCommandPool,
            _vulkanGraphicsQueue, gu2::Path(ASSETS_DIR) / "textures/box.png");
        _mesh = std::make_unique<gu2::Mesh>(_vulkanSettings, _vulkanPhysicalDevice, _vulkanDevice);
        _mesh->addVertexAttribute(0, _vertexPositionData.data());
        _mesh->addVertexAttribute(1, _vertexColorData.data());
        _mesh->addVertexAttribute(2, _vertexTexCoordData.data());
        _mesh->setIndices(_indexData.data(), _indexData.size());
        _mesh->upload(_vulkanCommandPool, _vulkanGraphicsQueue);
        _mesh->createDescriptorSetLayout();
        createGraphicsPipeline();
        createDepthResources();
        createFramebuffers();
        createCommandBuffers();
        createSyncObjects();
        _mesh->createDescriptorSets(*_texture);
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

        // Store the device properties in local struct
        vkGetPhysicalDeviceProperties(_vulkanPhysicalDevice, &_vulkanPhysicalDeviceProperties);
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

        if (!deviceFeatures.samplerAnisotropy)
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
        deviceFeatures.samplerAnisotropy = VK_TRUE;

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

    void cleanupSwapChain()
    {
        vkDestroyImageView(_vulkanDevice, _vulkanDepthImageView, nullptr);
        vkDestroyImage(_vulkanDevice, _vulkanDepthImage, nullptr);
        vkFreeMemory(_vulkanDevice, _vulkanDepthImageMemory, nullptr);

        for (auto& framebuffer : _vulkanSwapChainFramebuffers) {
            vkDestroyFramebuffer(_vulkanDevice, framebuffer, nullptr);
        }

        for (auto& imageView : _vulkanSwapChainImageViews) {
            vkDestroyImageView(_vulkanDevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(_vulkanDevice, _vulkanSwapChain, nullptr);
    }

    void recreateSwapChain()
    {
        vkDeviceWaitIdle(_vulkanDevice);

        cleanupSwapChain();

        createSwapChain();
        createImageViews();
        createDepthResources();
        createFramebuffers();

        _framebufferResized = false;
    }

    void createImageViews() {
        _vulkanSwapChainImageViews.resize(_vulkanSwapChainImages.size());
        for (size_t i = 0; i<_vulkanSwapChainImages.size(); i++) {
            _vulkanSwapChainImageViews[i] = gu2::createImageView(_vulkanDevice, _vulkanSwapChainImages[i],
                _vulkanSwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        }
    }

    VkShaderModule createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(_vulkanDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shader module!");
        }

        return shaderModule;
    }

    void createRenderPass()
    {
        // Color attachment
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = _vulkanSwapChainImageFormat;
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

        // Depth attachment
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        // Add dependency to prevent image transitions before swap chain image is available
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // previous render pass (present)
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // wait for image copy to swapchain
        // image to be ready
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // prevent any modifications to the color
        // attachment: this prevents layout transitions from happening before color attachment output stage

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(_vulkanDevice, &renderPassInfo, nullptr, &_vulkanRenderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }

    void createGraphicsPipeline()
    {
        // Shaders
        auto vertShaderCode = readFile("../shader/spir-v/vertex_simple.spv");
        auto fragShaderCode = readFile("../shader/spir-v/fragment_simple.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        // Vertex input info
        auto vertexInputInfo = _mesh->getVertexAttributesDescription().getPipelineVertexInputStateCreateInfo();

        // Input assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Dynamic states
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // Viewport and scissor
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) _vulkanSwapChainExtent.width;
        viewport.height = (float) _vulkanSwapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = _vulkanSwapChainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        // Blend mode
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        auto descriptorSetLayout = _mesh->getDescriptorSetLayout();
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(_vulkanDevice, &pipelineLayoutInfo, nullptr, &_vulkanPipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");

        // Enable depth testing
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER; // We're using inverse depth buffer (1.0f at near and 0.0f and far plane)
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f; // Optional
        depthStencil.maxDepthBounds = 1.0f; // Optional
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {}; // Optional
        depthStencil.back = {}; // Optional

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = _vulkanPipelineLayout;
        pipelineInfo.renderPass = _vulkanRenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(_vulkanDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_vulkanGraphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }

        // Cleanup shader objects
        vkDestroyShaderModule(_vulkanDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(_vulkanDevice, vertShaderModule, nullptr);
    }

    void createFramebuffers()
    {
        _vulkanSwapChainFramebuffers.resize(_vulkanSwapChainImageViews.size());

        for (size_t i = 0; i < _vulkanSwapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                _vulkanSwapChainImageViews[i],
                _vulkanDepthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = _vulkanRenderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = _vulkanSwapChainExtent.width;
            framebufferInfo.height = _vulkanSwapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(_vulkanDevice, &framebufferInfo, nullptr, &_vulkanSwapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
    }

    void createCommandPool()
    {
        auto queueFamilyIndices = findQueueFamilies(_vulkanPhysicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(_vulkanDevice, &poolInfo, nullptr, &_vulkanCommandPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
        VkFormatFeatureFlags features)
    {
        for (const auto& format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(_vulkanPhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
                return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;

            throw std::runtime_error("Failed to find supported format!");
        }
    }

    VkFormat findDepthFormat()
    {
        return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void createDepthResources()
    {
        auto depthFormat = findDepthFormat();
        gu2::createImage(_vulkanPhysicalDevice, _vulkanDevice,
            _vulkanSwapChainExtent.width, _vulkanSwapChainExtent.height, 1, depthFormat,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            _vulkanDepthImage, _vulkanDepthImageMemory);
        _vulkanDepthImageView = gu2::createImageView(_vulkanDevice, _vulkanDepthImage, depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT, 1);

        gu2::transitionImageLayout(_vulkanDevice, _vulkanCommandPool, _vulkanGraphicsQueue, _vulkanDepthImage,
            depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
    }

    void createCommandBuffers()
    {
        _vulkanCommandBuffers.resize(_vulkanSettings.framesInFlight);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = _vulkanCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = _vulkanCommandBuffers.size();

        if (vkAllocateCommandBuffers(_vulkanDevice, &allocInfo, _vulkanCommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _vulkanRenderPass;
        renderPassInfo.framebuffer = _vulkanSwapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = _vulkanSwapChainExtent;
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {0.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vulkanGraphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_vulkanSwapChainExtent.width);
        viewport.height = static_cast<float>(_vulkanSwapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = _vulkanSwapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        _mesh->bind(commandBuffer);

        for (uint32_t boxId=0; boxId<_vulkanSettings.nBoxes; ++boxId) {
            _mesh->draw(commandBuffer, _vulkanPipelineLayout, _currentFrame, boxId);
        }

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void createSyncObjects()
    {
        _vulkanImageAvailableSemaphores.resize(_vulkanSettings.framesInFlight);
        _vulkanRenderFinishedSemaphores.resize(_vulkanSettings.framesInFlight);
        _vulkanInFlightFences.resize(_vulkanSettings.framesInFlight);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i=0; i<_vulkanSettings.framesInFlight; ++i) {
            if (vkCreateSemaphore(_vulkanDevice, &semaphoreInfo, nullptr, &_vulkanImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(_vulkanDevice, &semaphoreInfo, nullptr, &_vulkanRenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(_vulkanDevice, &fenceInfo, nullptr, &_vulkanInFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create synchronization structures!");
            }
        }
    }

    void handleEvent(const gu2::Event& event)
    {
        switch (event.type) {
            case gu2::Event::WINDOW:
                switch (event.window.action) {
                    case gu2::WindowEventAction::CLOSE: close(); return;
                    case gu2::WindowEventAction::RESIZE:
                        _framebufferResized = true;
                        printf("Resize to %d x %d\n", event.window.data1, event.window.data2); fflush(stdout);
                        return;
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
        vkWaitForFences(_vulkanDevice, 1, &_vulkanInFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

        // Check for swap chain obsolescence
        uint32_t imageIndex;
        auto nextImageStatus = vkAcquireNextImageKHR(_vulkanDevice, _vulkanSwapChain, UINT64_MAX,
            _vulkanImageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (nextImageStatus == VK_ERROR_OUT_OF_DATE_KHR || nextImageStatus == VK_SUBOPTIMAL_KHR) {
            recreateSwapChain();
            return;
        }
        else if (nextImageStatus != VK_SUCCESS)
            throw std::runtime_error("Failed to acquire swap chain image!");

        vkResetFences(_vulkanDevice, 1, &_vulkanInFlightFences[_currentFrame]);

        vkResetCommandBuffer(_vulkanCommandBuffers[_currentFrame], 0);
        recordCommandBuffer(_vulkanCommandBuffers[_currentFrame], imageIndex);

        _mesh->updateUniformBuffer(_vulkanSwapChainExtent, _currentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {_vulkanImageAvailableSemaphores[_currentFrame]}; // is this needed?
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &_vulkanCommandBuffers[_currentFrame];
        VkSemaphore signalSemaphores[] = {_vulkanRenderFinishedSemaphores[_currentFrame]}; // is this needed?
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(_vulkanGraphicsQueue, 1, &submitInfo, _vulkanInFlightFences[_currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {_vulkanSwapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        auto queuePresentStatus = vkQueuePresentKHR(_vulkanPresentQueue, &presentInfo);
        if (queuePresentStatus == VK_ERROR_OUT_OF_DATE_KHR || queuePresentStatus == VK_SUBOPTIMAL_KHR ||
            _framebufferResized) {
            recreateSwapChain();
        }
        else if (queuePresentStatus != VK_SUCCESS)
            throw std::runtime_error("Failed to present swap chain image!");

        _currentFrame = (_currentFrame+1) % _vulkanSettings.framesInFlight;
    }

private:
    const gu2::VulkanSettings       _vulkanSettings;

    VkInstance                      _vulkanInstance;
    VkDebugUtilsMessengerEXT        _vulkanDebugMessenger;
    VkSurfaceKHR                    _vulkanSurface;
    VkPhysicalDevice                _vulkanPhysicalDevice;
    VkPhysicalDeviceProperties      _vulkanPhysicalDeviceProperties;
    VkDevice                        _vulkanDevice;
    VkQueue                         _vulkanGraphicsQueue;
    VkQueue                         _vulkanPresentQueue;
    VkSwapchainKHR                  _vulkanSwapChain;
    std::vector<VkImage>            _vulkanSwapChainImages;
    std::vector<VkImageView>        _vulkanSwapChainImageViews;
    VkFormat                        _vulkanSwapChainImageFormat;
    VkExtent2D                      _vulkanSwapChainExtent;
    VkRenderPass                    _vulkanRenderPass;
    VkPipelineLayout                _vulkanPipelineLayout;
    VkPipeline                      _vulkanGraphicsPipeline;
    std::vector<VkFramebuffer>      _vulkanSwapChainFramebuffers;
    VkCommandPool                   _vulkanCommandPool;
    std::vector<VkCommandBuffer>    _vulkanCommandBuffers;
    std::vector<VkSemaphore>        _vulkanImageAvailableSemaphores;
    std::vector<VkSemaphore>        _vulkanRenderFinishedSemaphores;
    std::vector<VkFence>            _vulkanInFlightFences;

    VkImage                         _vulkanDepthImage;
    VkDeviceMemory                  _vulkanDepthImageMemory;
    VkImageView                     _vulkanDepthImageView;

    std::unique_ptr<gu2::Texture>   _texture;
    std::unique_ptr<gu2::Mesh>      _mesh;

    std::vector<gu2::Vec3f>         _vertexPositionData;
    std::vector<gu2::Vec3f>         _vertexColorData;
    std::vector<gu2::Vec2f>         _vertexTexCoordData;
    std::vector<uint16_t>           _indexData;
    uint64_t                        _currentFrame;
    bool                            _framebufferResized;
};


int main(void)
{
    gu2::GLTFLoader sponzaLoader;
    sponzaLoader.load(gu2::Path(ASSETS_DIR) / "sponza/Main.1_Sponza/NewSponza_Main_glTF_002.gltf");

    gu2::WindowSettings windowSettings;
    windowSettings.name = "Hello Vulkan Box!";
    windowSettings.w = 800;
    windowSettings.h = 600;

    gu2::VulkanSettings vulkanSettings;
    #ifdef NDEBUG
    vulkanSettings.enableValidationLayers = false;
    #else
    vulkanSettings.enableValidationLayers = true;
    #endif

    // Main loop
    try {
        VulkanWindow window(windowSettings, vulkanSettings);
        gu2::App::addWindow(&window);

        while (gu2::App::update());
    }
    catch (const std::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        return EXIT_FAILURE;
    }

    gu2::cleanupBackend();

    return EXIT_SUCCESS;
}