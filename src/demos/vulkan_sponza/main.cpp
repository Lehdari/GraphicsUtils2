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
#include <gu2_vulkan/Pipeline.hpp>
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
                                 20, 21, 23, 20, 23, 22}
    {
        initVulkan();
    }

    ~VulkanWindow()
    {
        // Wait for the Vulkan device to finish its tasks
        vkDeviceWaitIdle(_vulkanDevice);
        _mesh.reset();
        _texture.reset();
        _pipeline.reset();
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
        _pipeline = std::make_unique<gu2::Pipeline>(_vulkanSettings, _vulkanPhysicalDevice, _vulkanDevice,
            _vulkanSurface, &_window);
        _pipeline->createSwapChain();
        _pipeline->createImageViews();
        _pipeline->createRenderPass();
        _pipeline->createCommandPool();
        _texture = std::make_unique<gu2::Texture>(_vulkanPhysicalDevice, _vulkanDevice, _pipeline->getCommandPool(),
            _vulkanGraphicsQueue, gu2::Path(ASSETS_DIR) / "textures/box.png");
        _mesh = std::make_unique<gu2::Mesh>(_vulkanSettings, _vulkanPhysicalDevice, _vulkanDevice);
        _mesh->addVertexAttribute(0, _vertexPositionData.data());
        _mesh->addVertexAttribute(1, _vertexColorData.data());
        _mesh->addVertexAttribute(2, _vertexTexCoordData.data());
        _mesh->setIndices(_indexData.data(), _indexData.size());
        _mesh->upload(_pipeline->getCommandPool(), _vulkanGraphicsQueue);
        _pipeline->createDescriptorSetLayout();

        // Shaders
        auto vertShaderCode = readFile("../shader/spir-v/vertex_simple.spv");
        auto fragShaderCode = readFile("../shader/spir-v/fragment_simple.spv");

        _pipeline->createGraphicsPipeline(
            createShaderModule(vertShaderCode),
            createShaderModule(fragShaderCode),
            _mesh->getVertexAttributesDescription().getPipelineVertexInputStateCreateInfo()
        );
        _pipeline->createDepthResources(_vulkanGraphicsQueue);
        _pipeline->createFramebuffers();
        _pipeline->createCommandBuffers();
        _pipeline->createSyncObjects();
        _pipeline->createDescriptorSets(*_texture);
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

        auto familyIndices = gu2::findQueueFamilies(device, _vulkanSurface);
        if (!familyIndices.isComplete())
            return 0; // not suitable

        bool extensionsSupported = checkDeviceExtensionSupport(device);
        if (!extensionsSupported)
            return 0; // not suitable

        auto swapChainSupport =  gu2::querySwapChainSupport(device, _vulkanSurface);
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

    void createLogicalDevice()
    {
        auto familyIndices = gu2::findQueueFamilies(_vulkanPhysicalDevice, _vulkanSurface);

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

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        _pipeline->recordRenderPassCommands(commandBuffer, imageIndex);

        _mesh->bind(commandBuffer);

        for (uint32_t boxId=0; boxId<_vulkanSettings.nBoxes; ++boxId) {
            _mesh->draw(commandBuffer, *_pipeline, _pipeline->getCurrentFrame(), boxId);
        }

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void handleEvent(const gu2::Event& event)
    {
        switch (event.type) {
            case gu2::Event::WINDOW:
                switch (event.window.action) {
                    case gu2::WindowEventAction::CLOSE: close(); return;
                    case gu2::WindowEventAction::RESIZE:
                        _pipeline->framebufferResized();
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
        VkCommandBuffer commandBuffer;
        uint32_t imageIndex;
        if (!_pipeline->beginRender(_vulkanGraphicsQueue, &commandBuffer, &imageIndex))
            return; // swap chain got resized
        recordCommandBuffer(commandBuffer, imageIndex);
        _pipeline->endRender(_vulkanGraphicsQueue, _vulkanPresentQueue, imageIndex);
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

    std::unique_ptr<gu2::Pipeline>  _pipeline;
    std::unique_ptr<gu2::Texture>   _texture;
    std::unique_ptr<gu2::Mesh>      _mesh;

    std::vector<gu2::Vec3f>         _vertexPositionData;
    std::vector<gu2::Vec3f>         _vertexColorData;
    std::vector<gu2::Vec2f>         _vertexTexCoordData;
    std::vector<uint16_t>           _indexData;
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