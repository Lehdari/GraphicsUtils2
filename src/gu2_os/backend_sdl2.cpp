//
// Project: GraphicsUtils2
// File: backend_sdl2.cpp
//
// Copyright (c) 2023 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "backend.hpp"

#include <SDL_vulkan.h>

#include <thread>


using namespace gu2;


WindowObject detail::createWindowObject(const WindowSettings& settings)
{
    return {settings.name.c_str(), settings.x, settings.y, settings.w, settings.h, SDL_WINDOW_VULKAN};
}


std::vector<const char*> gu2::getVulkanInstanceExtensions()
{
    auto* window  = SDL_CreateWindow("gu2_vulkan_dummy", 0, 0, 0, 0, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
    if (window == nullptr)
        throw std::runtime_error("Unable to create an SDL window when fetching required vulkan instance extensions");

    uint32_t count;
    SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);

    std::vector<const char*> extensions(count);
    if (SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data()) != SDL_TRUE) {
        SDL_DestroyWindow(window);
        std::string sdlError(SDL_GetError());
        throw std::runtime_error("Unable to fetch the required vulkan instance extensions: " + sdlError);
    }

    SDL_DestroyWindow(window);

    #ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    #endif

    return extensions;
}

void gu2::sleep(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
