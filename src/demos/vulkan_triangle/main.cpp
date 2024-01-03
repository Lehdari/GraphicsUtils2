//
// Project: GraphicsUtils2
// File: main.cpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include <gu2_os/App.hpp>
#include <gu2_os/Window.hpp>

#include <vulkan/vulkan.h>


class VulkanWindow : public gu2::Window<VulkanWindow> {
public:
    VulkanWindow(const gu2::WindowSettings& settings) :
        Window<VulkanWindow>    (settings)
    {
        initVulkan();
    }

    ~VulkanWindow()
    {
        vkDestroyInstance(_vulkanInstance, nullptr);
    }

    void initVulkan()
    {
        createInstance();
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

        auto requiredExtensions = gu2::getVulkanInstanceExtensions();

        #ifdef VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif

        createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();
        createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &_vulkanInstance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create vulkan instance!");
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
    VkInstance  _vulkanInstance;
};


int main(void)
{
    gu2::WindowSettings settings;
    settings.name = "Hello Vulkan Triangle!";
    settings.w = 800;
    settings.h = 600;
    VulkanWindow window(settings);

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