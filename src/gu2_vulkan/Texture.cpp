//
// Project: GraphicsUtils2
// File: Texture.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Texture.hpp"
#include "Util.hpp"
#include "gu2_util/Image.hpp"


using namespace gu2;


template<>
void Texture::createFromImage<uint8_t>(VkCommandPool commandPool, VkQueue queue, const Image<uint8_t>& image)
{
    _imageMipLevels = std::floor(std::log2(std::max(image.width(), image.height()))) + 1;
    VkDeviceSize imageSize = image.nElements() * sizeof(uint8_t);

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


Texture::Texture(TextureSettings settings) :
    _settings       (std::move(settings)),
    _image          (nullptr),
    _imageMemory    (nullptr),
    _imageView      (nullptr),
    _sampler        (nullptr)
{
    // Store the device properties in local struct
    vkGetPhysicalDeviceProperties(_settings.physicalDevice, &_physicalDeviceProperties);
}

Texture::~Texture()
{
    if (_sampler != nullptr)
        vkDestroySampler(_settings.device, _sampler, nullptr);
    if (_imageView != nullptr)
        vkDestroyImageView(_settings.device, _imageView, nullptr);
    if (_image != nullptr)
        vkDestroyImage(_settings.device, _image, nullptr);
    if (_imageMemory != nullptr)
        vkFreeMemory(_settings.device, _imageMemory, nullptr);
}

void Texture::createFromFile(VkCommandPool commandPool, VkQueue queue, const Path& filename)
{
#if 1   // TODO temporary toggle to prevent expensive conversion
    Image<uint8_t> image;
    gu2::convertImage(gu2::readImageFromFile<uint8_t>(filename), image, gu2::ImageFormat::RGBA);
#else
    gu2::Image<uint8_t> image(512, 512);
#endif

    createFromImage(commandPool, queue, image);
    createTextureImageView();
    createTextureSampler();
}

void Texture::createTextureImageView()
{
    // Destroy potential previous image view
    if (_imageView != nullptr)
        vkDestroyImageView(_settings.device, _imageView, nullptr);

    _imageView = gu2::createImageView(_settings.device, _image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT,
        _imageMipLevels);
}

void Texture::createTextureSampler()
{
    // Destroy potential previous sampler
    if (_sampler != nullptr)
        vkDestroySampler(_settings.device, _sampler, nullptr);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = _physicalDeviceProperties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = (float)_imageMipLevels;

    if (vkCreateSampler(_settings.device, &samplerInfo, nullptr, &_sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler!");
    }
}

VkImageView Texture::getImageView() const
{
    return _imageView;
}

VkSampler Texture::getSampler() const
{
    return _sampler;
}
