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

#include <array>


using namespace gu2;


Renderer::Renderer(RendererSettings settings) :
    _settings           (settings),
    _renderPass         (nullptr),
    _framebufferResized (false),
    _currentFrame       (0)
{
    // Store the device properties in local struct
    vkGetPhysicalDeviceProperties(_settings.physicalDevice, &_physicalDeviceProperties);

    createCommandPool();
    createCommandBuffers();
    createSwapChain();
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
    vkDestroyRenderPass(_settings.device, _renderPass, nullptr);
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
            .format = gu2::findDepthFormat(_settings.physicalDevice),
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        },
        {
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        },
        gu2::createImageView(_settings.device, _depthImage, depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT, 1)
    };

    gu2::transitionImageLayout(_settings.device, _commandPool, _settings.graphicsQueue, _depthImage,
        depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
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
            {
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            },
            gu2::createImageView(_settings.device, _swapChainObjects[i].image, _swapChainImageFormat,
                VK_IMAGE_ASPECT_COLOR_BIT, 1)
        };
    }
}

void Renderer::cleanupSwapChain()
{
    vkDestroyImageView(_settings.device, _depthAttachment.imageView, nullptr);
    vkDestroyImage(_settings.device, _depthImage, nullptr);
    vkFreeMemory(_settings.device, _depthImageMemory, nullptr);

    for (auto& imageData : _swapChainObjects) {
        vkDestroyFramebuffer(_settings.device, imageData.framebuffer, nullptr);
        vkDestroyImageView(_settings.device, imageData.colorAttachment.imageView, nullptr);
    }

    vkDestroySwapchainKHR(_settings.device, _swapChain, nullptr);
}

void Renderer::recreateSwapChain()
{
    vkDeviceWaitIdle(_settings.device);

    cleanupSwapChain();
    createSwapChain();
    createFramebuffers();

    _framebufferResized = false;
}

void Renderer::createRenderPass()
{
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &_swapChainObjects[0].colorAttachment.reference;
    subpass.pDepthStencilAttachment = &_depthAttachment.reference;

    // Add dependency to prevent image transitions before swap chain image is available
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // previous render pass (present)
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // wait for image copy to swapchain image to be ready
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // prevent any modifications to the color
    // attachment: this prevents layout transitions from happening before color attachment output stage

    std::array<VkAttachmentDescription, 2> attachments = {
        _swapChainObjects[0].colorAttachment.description, _depthAttachment.description};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_settings.device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void Renderer::createFramebuffers()
{
    for (auto& imageData : _swapChainObjects) {
        std::array<VkImageView, 2> framebufferAttachments = {
            imageData.colorAttachment.imageView, _depthAttachment.imageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(framebufferAttachments.size());
        framebufferInfo.pAttachments = framebufferAttachments.data();
        framebufferInfo.width = _swapChainExtent.width;
        framebufferInfo.height = _swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_settings.device, &framebufferInfo, nullptr, &imageData.framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
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

void Renderer::recordRenderPassCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapChainObjects[imageIndex].framebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapChainExtent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {0.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_swapChainExtent.width);
    viewport.height = static_cast<float>(_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

VkCommandPool Renderer::getCommandPool()
{
    return _commandPool;
}

uint64_t Renderer::getCurrentFrame() const
{
    return _currentFrame;
}

VkRenderPass Renderer::getRenderPass() const
{
    return _renderPass;
}

VkExtent2D Renderer::getSwapChainExtent() const
{
    return _swapChainExtent;
}

bool Renderer::beginRender(VkCommandBuffer* commandBuffer, uint32_t* imageIndex)
{
    vkWaitForFences(_settings.device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    // Check for swap chain obsolescence
    auto nextImageStatus = vkAcquireNextImageKHR(_settings.device, _swapChain, UINT64_MAX,
        _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, imageIndex);
    if (nextImageStatus == VK_ERROR_OUT_OF_DATE_KHR || nextImageStatus == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
        return false;
    }
    else if (nextImageStatus != VK_SUCCESS)
        throw std::runtime_error("Failed to acquire swap chain image!");

    vkResetFences(_settings.device, 1, &_inFlightFences[_currentFrame]);

    vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);

    *commandBuffer = _commandBuffers[_currentFrame];
    return true;
}

void Renderer::endRender(VkQueue presentQueue, uint32_t imageIndex)
{
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]}; // is this needed?
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];
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

    VkSwapchainKHR swapChains[] = {_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    auto queuePresentStatus = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (queuePresentStatus == VK_ERROR_OUT_OF_DATE_KHR || queuePresentStatus == VK_SUBOPTIMAL_KHR ||
        _framebufferResized) {
        recreateSwapChain();
    }
    else if (queuePresentStatus != VK_SUCCESS)
        throw std::runtime_error("Failed to present swap chain image!");

    _currentFrame = (_currentFrame+1) % _settings.vulkanSettings->framesInFlight;
}

void Renderer::framebufferResized()
{
    _framebufferResized = true;
}
