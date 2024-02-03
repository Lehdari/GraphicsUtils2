//
// Project: GraphicsUtils2
// File: Texture.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "gu2_util/Typedef.hpp"

#include <vulkan/vulkan.h>


namespace gu2 {


class Texture {
public:
    Texture(
        VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkQueue queue,
        const Path& filename);
    ~Texture();

    void createTextureImage(VkCommandPool commandPool, VkQueue queue, const Path& filename);
    void createTextureImageView();
    void createTextureSampler();

    VkImageView getImageView() const;
    VkSampler getSampler() const;

private:
    // TODO Subject to relocation
    VkPhysicalDevice                _physicalDevice;
    VkPhysicalDeviceProperties      _physicalDeviceProperties;
    VkDevice                        _device;

    VkImage                         _image;
    VkDeviceMemory                  _imageMemory;
    VkImageView                     _imageView;
    uint32_t                        _imageMipLevels;
    VkSampler                       _sampler;
};


} // namespace gu2
