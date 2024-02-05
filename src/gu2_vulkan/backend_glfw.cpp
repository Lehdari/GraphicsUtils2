//
// Project: GraphicsUtils2
// File: backend_glfw.cpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#define GLFW_INCLUDE_VULKAN
#include "backend.hpp"

#include <thread>


using namespace gu2;


std::vector<const char*> gu2::getVulkanInstanceExtensions()
{
    uint32_t count;
    auto* extensionsRaw = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char*> extensions(count);
    for (uint32_t i=0; i<count; ++i)
        extensions[i] = extensionsRaw[i];

    #ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    #endif

    return extensions;
}

bool gu2::createWindowVulkanSurface(
    WindowObject& window,
    VkInstance instance,
    const VkAllocationCallbacks* allocator,
    VkSurfaceKHR* surface)
{
    return glfwCreateWindowSurface(instance, window.get(), allocator, surface) == VK_SUCCESS;
}

void gu2::getWindowFramebufferSize(WindowObject* window, int* w, int* h)
{
    glfwGetFramebufferSize(window->get(), w, h);
}
