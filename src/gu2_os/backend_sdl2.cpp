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


uint64_t gu2::detail::nActiveWindows = 0;


WindowObject detail::createWindowObject(const WindowSettings& settings)
{
    return {settings.name.c_str(), settings.x, settings.y, settings.w, settings.h, SDL_WINDOW_VULKAN};
}


void gu2::sleep(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void gu2::cleanupBackend()
{
    if (gu2::detail::nActiveWindows > 0)
        throw std::runtime_error("cleanupBackend() called with active windows (" +
            std::to_string(gu2::detail::nActiveWindows) + ")");

    SDL_Quit();
}
