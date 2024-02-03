//
// Project: GraphicsUtils2
// File: VulkanSettings.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include <vulkan/vulkan.h>
#include <vector>


namespace gu2 {

struct VulkanSettings {
    bool                        enableValidationLayers  {true};
    std::vector<const char*>    validationLayers        {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*>    deviceExtensions        {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    int                         framesInFlight          {2};
    int                         nBoxes                  {3}; // TODO obviously this doesn't belong here
};

} // namespace gu2
