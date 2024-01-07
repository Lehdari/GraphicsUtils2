//
// Project: GraphicsUtils2
// File: QueryWrapper.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>


// Macro for creating predefined vulkan query wrappers with the original vulkan function name
#define GU2_VULKAN_QUERY_WRAPPER(RETURN_TYPE, VULKAN_FUNCTION)                      \
template <typename... T_Args>                                                       \
inline std::vector<RETURN_TYPE> VULKAN_FUNCTION(T_Args&&... args)                   \
{                                                                                   \
    return vkQuery<RETURN_TYPE>(::VULKAN_FUNCTION, std::forward<T_Args>(args)...);  \
}


namespace gu2 {

// vkQuery provides a convenient C++ wrapper for performing the standard Vulkan query operations
template <typename T_Data, typename T_QueryFunction, typename... T_Args>
inline std::vector<T_Data> vkQuery(T_QueryFunction query, T_Args&&... args)
{
    uint32_t count;
    query(args..., &count, nullptr);
    if (count == 0)
        return {};
    std::vector<T_Data> data(count);
    query(std::forward<T_Args>(args)..., &count, data.data());
    return data;
}


// Predefined query wrappers
GU2_VULKAN_QUERY_WRAPPER(VkExtensionProperties, vkEnumerateDeviceExtensionProperties)
GU2_VULKAN_QUERY_WRAPPER(VkLayerProperties, vkEnumerateInstanceLayerProperties)
GU2_VULKAN_QUERY_WRAPPER(VkPhysicalDevice, vkEnumeratePhysicalDevices)
GU2_VULKAN_QUERY_WRAPPER(VkQueueFamilyProperties, vkGetPhysicalDeviceQueueFamilyProperties)
GU2_VULKAN_QUERY_WRAPPER(VkSurfaceFormatKHR, vkGetPhysicalDeviceSurfaceFormatsKHR)
GU2_VULKAN_QUERY_WRAPPER(VkPresentModeKHR, vkGetPhysicalDeviceSurfacePresentModesKHR)
GU2_VULKAN_QUERY_WRAPPER(VkImage, vkGetSwapchainImagesKHR)

} // namespace gu2
