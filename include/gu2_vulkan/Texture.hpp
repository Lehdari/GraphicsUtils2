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

    void loadFromFile(VkCommandPool commandPool, VkQueue queue, const Path& filename);
    template<class T_Data>
    void createFromImage(VkCommandPool commandPool, VkQueue queue, const Image<T_Data>& image);
    void createTextureImage(VkCommandPool commandPool, VkQueue queue, const Path& filename);
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


template<class T_Data>
void Texture::createFromImage(VkCommandPool commandPool, VkQueue queue, const Image<T_Data>& image)
{
    _imageMipLevels = std::floor(std::log2(std::max(image.width(), image.height()))) + 1;
    VkDeviceSize imageSize = image.nElements() * sizeof(T_Data);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(_settings.physicalDevice, _settings.device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(_settings.device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, image.data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(_settings.device, stagingBufferMemory);

    #pragma omp critical
    {
        gu2::createImage(_settings.physicalDevice, _settings.device,
            image.width(), image.height(), _imageMipLevels,
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _image, _imageMemory);

        gu2::transitionImageLayout(_settings.device, commandPool, queue, _image, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _imageMipLevels);
        gu2::copyBufferToImage(_settings.device, commandPool, queue, stagingBuffer, _image,
            static_cast<uint32_t>(image.width()), static_cast<uint32_t>(image.height()));
        gu2::generateMipmaps(_settings.physicalDevice, _settings.device, commandPool, queue, _image, VK_FORMAT_R8G8B8A8_SRGB,
            image.width(), image.height(), _imageMipLevels);
    }

    vkDestroyBuffer(_settings.device, stagingBuffer, nullptr);
    vkFreeMemory(_settings.device, stagingBufferMemory, nullptr);
}


} // namespace gu2
