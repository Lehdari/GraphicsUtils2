//
// Project: GraphicsUtils2
// File: backend.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "gu2_os/backend.hpp"

#include <vulkan/vulkan.h>


namespace gu2 {

/// Wrapper for getting required backend Vulkan instance extensions
std::vector<const char*> getVulkanInstanceExtensions();

/// Wrapper for creating Vulkan surface for the given window
bool createWindowVulkanSurface(
    WindowObject& window,
    VkInstance instance,
    const VkAllocationCallbacks *allocator,
    VkSurfaceKHR* surface);

void getWindowFramebufferSize(WindowObject& window, int* w, int* h);

} // namespace gu2
