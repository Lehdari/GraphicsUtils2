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

#include "Util.hpp"
#include "gu2_util/Typedef.hpp"

#include <vulkan/vulkan.h>


namespace gu2 {


template <typename T_Data>
class Image;


struct TextureSettings {
    VkPhysicalDevice    physicalDevice  {nullptr};
    VkDevice            device          {nullptr};
};


class Texture {
public:
    Texture(TextureSettings settings = TextureSettings{});
    Texture(const Texture&) = delete;
    Texture(Texture&&) = default;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) = default;
    ~Texture();

    void create(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImageAspectFlags aspectFlags
    );
    void createFromFile(VkCommandPool commandPool, VkQueue queue, const Path& filename);
    template<class T_Data>
    void createFromImage(VkCommandPool commandPool, VkQueue queue, const Image<T_Data>& image);
    void createTextureImageView();
    void createTextureSampler();

    VkImageView getImageView() const;
    VkSampler getSampler() const;

private:
    // TODO Subject to relocation
    TextureSettings             _settings;
    VkPhysicalDeviceProperties  _physicalDeviceProperties;

    VkImage                     _image;
    VkDeviceMemory              _imageMemory;
    VkImageView                 _imageView;
    uint32_t                    _imageMipLevels;
    VkSampler                   _sampler;
};


} // namespace gu2
