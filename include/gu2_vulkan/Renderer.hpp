//
// Project: GraphicsUtils2
// File: Renderer.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "backend.hpp"
#include "gu2_util/MathTypes.hpp"

#include <vulkan/vulkan.h>

#include <vector>


namespace gu2 {


class Material;
class Texture;
class Scene;
class VulkanSettings;


struct RendererSettings {
    const VulkanSettings*   vulkanSettings;
    VkPhysicalDevice        physicalDevice;
    VkDevice                device;
    VkSurfaceKHR            surface;
    WindowObject*           window;
};


class Renderer {
public:
    Renderer(const RendererSettings& settings);
    Renderer(const Renderer& pipeline) = delete;
    Renderer(Renderer&& pipeline) = delete;
    Renderer& operator=(const Renderer& pipeline) = delete;
    Renderer& operator=(Renderer&& pipeline) = delete;
    ~Renderer();

    void createImageViews();
    void createDepthResources(VkQueue graphicsQueue);
    void createSwapChain();
    void cleanupSwapChain();
    void recreateSwapChain(VkQueue graphicsQueue);
    void createRenderPass();
    void createCommandPool();
    void createFramebuffers();
    void createCommandBuffers();
    void createSyncObjects();

    void recordRenderPassCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    VkCommandPool getCommandPool();
    uint64_t getCurrentFrame() const;

    VkRenderPass getRenderPass() const;
    VkExtent2D getSwapChainExtent() const;

    bool beginRender(VkQueue graphicsQueue, VkCommandBuffer* commandBuffer, uint32_t* imageIndex);
    void endRender(VkQueue graphicsQueue, VkQueue presentQueue, uint32_t imageIndex);
    void framebufferResized();

    // TODO subject to relocation
    void updateUniformBuffer(const Scene& scene);

    friend class Mesh; // TODO remove

private:
    // TODO Subject to relocation
    const VulkanSettings*           _vulkanSettings;
    VkPhysicalDevice                _physicalDevice;
    VkPhysicalDeviceProperties      _physicalDeviceProperties;
    VkDevice                        _device;
    VkSurfaceKHR                    _surface;
    WindowObject*                   _window;

    VkImage                         _depthImage;
    VkDeviceMemory                  _depthImageMemory;
    VkImageView                     _depthImageView;
    VkSwapchainKHR                  _swapChain;
    std::vector<VkImage>            _swapChainImages;
    std::vector<VkImageView>        _swapChainImageViews;
    VkFormat                        _swapChainImageFormat;
    VkExtent2D                      _swapChainExtent;
    VkRenderPass                    _renderPass;
    std::vector<VkFramebuffer>      _swapChainFramebuffers;
    VkCommandPool                   _commandPool;
    std::vector<VkCommandBuffer>    _commandBuffers;
    std::vector<VkSemaphore>        _imageAvailableSemaphores;
    std::vector<VkSemaphore>        _renderFinishedSemaphores;
    std::vector<VkFence>            _inFlightFences;
    bool                            _framebufferResized;
    uint64_t                        _currentFrame;
};


} // namespace gu2
