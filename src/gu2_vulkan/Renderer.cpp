//
// Project: GraphicsUtils2
// File: Renderer.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Renderer.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "Util.hpp"
#include "VulkanSettings.hpp"
#include "QueryWrapper.hpp"


using namespace gu2;


Renderer::Renderer(RendererSettings settings) :
    _settings           (settings),
    _framebufferResized (false),
    _currentFrame       (0),
    _geometryPass       ({_settings.device}),
    _compositePass      ({_settings.device}, _settings.physicalDevice, _settings.descriptorManager,
                         _settings.pipelineManager, _settings.vulkanSettings->framesInFlight)
{
    // Store the device properties in local struct
    vkGetPhysicalDeviceProperties(_settings.physicalDevice, &_physicalDeviceProperties);

    createCommandPool();
    createCommandBuffers();
    createSwapChain();
    createSyncObjects();
}

Renderer::~Renderer()
{
    cleanupSwapChain();
    for (auto& imageAvailableSemaphore : _imageAvailableSemaphores)
        vkDestroySemaphore(_settings.device, imageAvailableSemaphore, nullptr);
    for (auto& renderFinishedSemaphore : _renderFinishedSemaphores)
        vkDestroySemaphore(_settings.device, renderFinishedSemaphore, nullptr);
    for (auto& inFlightFence : _inFlightFences)
        vkDestroyFence(_settings.device, inFlightFence, nullptr);
    vkDestroyCommandPool(_settings.device, _commandPool, nullptr);
}

void Renderer::createCommandPool()
{
    auto queueFamilyIndices = findQueueFamilies(_settings.physicalDevice, _settings.surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(_settings.device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void Renderer::createCommandBuffers()
{
    _commandBuffers.resize(_settings.vulkanSettings->framesInFlight);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = _commandBuffers.size();

    if (vkAllocateCommandBuffers(_settings.device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void Renderer::createDepthResources()
{
    auto depthFormat = findDepthFormat(_settings.physicalDevice);
    gu2::createImage(_settings.physicalDevice, _settings.device,
        _swapChainExtent.width, _swapChainExtent.height, 1, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _depthImage, _depthImageMemory);
    _depthAttachment = AttachmentHandle{
        {
            .format = depthFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        },
        {},
        gu2::createImageView(_settings.device, _depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1),
        _swapChainExtent
    };

    gu2::transitionImageLayout(_settings.device, _commandPool, _settings.graphicsQueue, _depthImage,
        depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void Renderer::createGBufferResources()
{
    // Base color
    auto baseColorFormat = findSupportedFormat(_settings.physicalDevice,
        {VK_FORMAT_R32G32B32A32_SFLOAT}, // TODO probably way too overkill
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
    );
    gu2::createImage(_settings.physicalDevice, _settings.device, _swapChainExtent.width, _swapChainExtent.height, 1,
        baseColorFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _baseColorImage, _baseColorImageMemory);
    _baseColorAttachment = AttachmentHandle{
        .description = {
            .format = baseColorFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },
        .reference = {},
        .imageView = gu2::createImageView(_settings.device, _baseColorImage, baseColorFormat,
            VK_IMAGE_ASPECT_COLOR_BIT, 1),
        .imageExtent = _swapChainExtent
    };

    // Normal
    auto normalFormat = findSupportedFormat(_settings.physicalDevice,
        {VK_FORMAT_R32G32B32A32_SFLOAT}, // TODO probably way too overkill
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT
    );
    gu2::createImage(_settings.physicalDevice, _settings.device, _swapChainExtent.width, _swapChainExtent.height, 1,
        normalFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _normalImage, _normalImageMemory);
    _normalAttachment = AttachmentHandle{
        .description = {
            .format = normalFormat,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        },
        .reference = {},
        .imageView = gu2::createImageView(_settings.device, _normalImage, normalFormat,
            VK_IMAGE_ASPECT_COLOR_BIT, 1),
        .imageExtent = _swapChainExtent
    };
}

void Renderer::createSwapChain()
{
    VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(_settings.physicalDevice, _settings.surface);

    auto surfaceFormat = selectSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = selectSwapPresentMode(swapChainSupport.presentModes);
    auto extent = selectSwapExtent(_settings.window, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _settings.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = gu2::findQueueFamilies(_settings.physicalDevice, _settings.surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_settings.device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;

    createDepthResources();

    // Construct objects required to be replicated for each swap chain image
    auto swapChainImages = gu2::vkGetSwapchainImagesKHR(_settings.device, _swapChain);
    _swapChainObjects.resize(swapChainImages.size());
    for (size_t i=0; i<swapChainImages.size(); ++i) {
        auto& imageData = _swapChainObjects[i];

        // Image, image view and attachment
        imageData.image = swapChainImages[i];
        imageData.colorAttachment = AttachmentHandle{
            {
                .format = _swapChainImageFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            },
            {},
            gu2::createImageView(_settings.device, _swapChainObjects[i].image, _swapChainImageFormat,
                VK_IMAGE_ASPECT_COLOR_BIT, 1),
            _swapChainExtent
        };
    }

    // Create new GBuffer
    createGBufferResources();

    // Rebuild render passes
    createGeometryPass();
    createCompositePass();
}

void Renderer::cleanupSwapChain()
{
    vkDestroyImageView(_settings.device, _normalAttachment.imageView, nullptr);
    vkDestroyImage(_settings.device, _normalImage, nullptr);
    vkFreeMemory(_settings.device, _normalImageMemory, nullptr);
    vkDestroyImageView(_settings.device, _baseColorAttachment.imageView, nullptr);
    vkDestroyImage(_settings.device, _baseColorImage, nullptr);
    vkFreeMemory(_settings.device, _baseColorImageMemory, nullptr);

    vkDestroyImageView(_settings.device, _depthAttachment.imageView, nullptr);
    vkDestroyImage(_settings.device, _depthImage, nullptr);
    vkFreeMemory(_settings.device, _depthImageMemory, nullptr);

    for (auto& imageData : _swapChainObjects) {
        vkDestroyImageView(_settings.device, imageData.colorAttachment.imageView, nullptr);
    }

    vkDestroySwapchainKHR(_settings.device, _swapChain, nullptr);
}

void Renderer::recreateSwapChain()
{
    vkDeviceWaitIdle(_settings.device);

    cleanupSwapChain();
    createSwapChain();

    _framebufferResized = false;
}

void Renderer::createGeometryPass()
{
    _geometryPass.setOutputAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _baseColorAttachment);
    _geometryPass.setOutputAttachment(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _normalAttachment);
    _geometryPass.setOutputAttachment(2, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, _depthAttachment);
    _geometryPass.build();
}

void Renderer::createCompositePass()
{
    _compositePass.setInputAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _baseColorAttachment);
    _compositePass.setInputAttachment(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _normalAttachment);
    for (size_t i=0; i<_swapChainObjects.size(); ++i)
        _compositePass.setOutputAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            _swapChainObjects[i].colorAttachment, i);
    _compositePass.createQuad(_commandPool, _settings.graphicsQueue);
    _compositePass.build();
}

void Renderer::createSyncObjects()
{
    _imageAvailableSemaphores.resize(_settings.vulkanSettings->framesInFlight);
    _renderFinishedSemaphores.resize(_settings.vulkanSettings->framesInFlight);
    _inFlightFences.resize(_settings.vulkanSettings->framesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i=0; i<_settings.vulkanSettings->framesInFlight; ++i) {
        if (vkCreateSemaphore(_settings.device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(_settings.device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(_settings.device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization structures!");
        }
    }
}

bool Renderer::render(const Scene& scene, VkQueue presentQueue)
{
    uint32_t swapChainImageId = 0;
    vkWaitForFences(_settings.device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    // Check for swap chain obsolescence
    auto nextImageStatus = vkAcquireNextImageKHR(_settings.device, _swapChain, UINT64_MAX,
        _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &swapChainImageId);
    if (nextImageStatus == VK_ERROR_OUT_OF_DATE_KHR || nextImageStatus == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
        return false;
    }
    else if (nextImageStatus != VK_SUCCESS)
        throw std::runtime_error("Failed to acquire swap chain image!");

    // Reset sync objects
    vkResetFences(_settings.device, 1, &_inFlightFences[_currentFrame]);
    auto commandBuffer = _commandBuffers[_currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    // Render passes
    _geometryPass.setScene(scene);
    transitionGBufferImageToAttachment(_baseColorImage, commandBuffer);
    transitionGBufferImageToAttachment(_normalImage, commandBuffer);
    dynamic_cast<RenderPass*>(&_geometryPass)->render(commandBuffer, _currentFrame, 0);
    transitionGBufferImageToRead(_baseColorImage, commandBuffer);
    transitionGBufferImageToRead(_normalImage, commandBuffer);
    dynamic_cast<RenderPass*>(&_compositePass)->render(commandBuffer, _currentFrame, swapChainImageId);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]}; // is this needed?
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]}; // is this needed?
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    auto submitResult = vkQueueSubmit(_settings.graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]);
    if (submitResult != VK_SUCCESS) {
        printf("submitResult: %d\n", submitResult);
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &_swapChain;
    presentInfo.pImageIndices = &swapChainImageId;
    presentInfo.pResults = nullptr; // Optional

    auto queuePresentStatus = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (queuePresentStatus == VK_ERROR_OUT_OF_DATE_KHR || queuePresentStatus == VK_SUBOPTIMAL_KHR ||
        _framebufferResized) {
        recreateSwapChain();
    }
    else if (queuePresentStatus != VK_SUCCESS)
        throw std::runtime_error("Failed to present swap chain image!");

    _currentFrame = (_currentFrame+1) % _settings.vulkanSettings->framesInFlight;

    return true; // rendering succeeded
}

void Renderer::framebufferResized()
{
    _framebufferResized = true;
}

void Renderer::transitionGBufferImageToAttachment(VkImage image, VkCommandBuffer commandBuffer)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void Renderer::transitionGBufferImageToRead(VkImage image, VkCommandBuffer commandBuffer)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}
