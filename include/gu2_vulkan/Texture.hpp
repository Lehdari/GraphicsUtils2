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


struct TextureProperties {
    uint32_t                width               {0};
    uint32_t                height              {0};
    VkFormat                format              {VK_FORMAT_MAX_ENUM};
    VkImageTiling           tiling              {VK_IMAGE_TILING_MAX_ENUM};
    VkImageUsageFlags       usage               {VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM};
    VkMemoryPropertyFlags   memoryProperties    {VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM};
    VkImageAspectFlags      aspectFlags         {VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM};
};


class Texture {
public:
    Texture(TextureSettings settings = TextureSettings{});
    Texture(const Texture&) = delete;
    Texture(Texture&&) = default;
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(Texture&&) = default;
    ~Texture();

    void create(TextureProperties properties);
    void createFromFile(VkCommandPool commandPool, VkQueue queue, const Path& filename);
    template<class T_Data>
    void createFromImage(VkCommandPool commandPool, VkQueue queue, const Image<T_Data>& image);
    void createTextureImageView();
    void createTextureSampler();

    inline const TextureProperties& getProperties() const noexcept { return _properties; }
    inline VkImage getImage() const noexcept { return _image; }
    inline VkImageView getImageView() const noexcept { return _imageView; }
    inline VkSampler getSampler() const noexcept { return _sampler; }

private:
    // TODO Subject to relocation
    TextureSettings             _settings;
    VkPhysicalDeviceProperties  _physicalDeviceProperties;
    TextureProperties           _properties;

    VkImage                     _image;
    VkDeviceMemory              _imageMemory;
    VkImageView                 _imageView;
    uint32_t                    _imageMipLevels;
    VkSampler                   _sampler;
};


} // namespace gu2
