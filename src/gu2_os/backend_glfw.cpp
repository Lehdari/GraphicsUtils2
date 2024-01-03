//
// Project: GraphicsUtils2
// File: backend_glfw.cpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "backend.hpp"

#include <thread>


using namespace gu2;


WindowObject detail::createWindowObject(const WindowSettings& settings)
{
    glfwInit();
    return {settings.w, settings.h, settings.name.c_str(), nullptr, nullptr};
}


std::vector<const char*> gu2::getVulkanInstanceExtensions()
{
    uint32_t count;
    auto* extensionsRaw = glfwGetRequiredInstanceExtensions(&count);

    std::vector<const char*> extensions(count);
    for (uint32_t i=0; i<count; ++i)
        extensions[i] = extensionsRaw[i];

    #ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    #endif

    return extensions;
}

void gu2::sleep(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
