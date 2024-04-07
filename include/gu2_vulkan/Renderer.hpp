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
#include "CompositePass.hpp"
#include "GeometryPass.hpp"
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
    VkQueue                 graphicsQueue;
    WindowObject*           window;
};


class Renderer {
public:
    Renderer(RendererSettings settings);
    Renderer(const Renderer& pipeline) = delete;
    Renderer(Renderer&& pipeline) = delete;
    Renderer& operator=(const Renderer& pipeline) = delete;
    Renderer& operator=(Renderer&& pipeline) = delete;
    ~Renderer();

    void createCommandPool();
    void createCommandBuffers();
    void createDepthResources();
    void createGBufferResources();
    void createSwapChain();
    void cleanupSwapChain();
    void recreateSwapChain();
    void createGeometryPass();
    void createCompositePass();
    void createSyncObjects();

    bool render(const Scene& scene, VkQueue presentQueue);

    VkCommandPool getCommandPool();
    uint64_t getCurrentFrame() const;

    VkRenderPass getGeometryRenderPass() const;
    VkExtent2D getSwapChainExtent() const;

    void framebufferResized();

    friend class Mesh; // TODO remove

private:
    // Helper struct for wrapping all the stuff that needs to be replicated for each swapchain image
    struct SwapChainData {
        VkImage             image           {nullptr};
        AttachmentHandle    colorAttachment {};
    };

    RendererSettings                _settings;
    VkPhysicalDeviceProperties      _physicalDeviceProperties;

    VkImage                         _depthImage;
    VkDeviceMemory                  _depthImageMemory;
    AttachmentHandle                _depthAttachment;

    // GBuffer
    VkImage                         _baseColorImage;
    VkDeviceMemory                  _baseColorImageMemory;
    AttachmentHandle                _baseColorAttachment;

    VkSwapchainKHR                  _swapChain;
    VkFormat                        _swapChainImageFormat;
    VkExtent2D                      _swapChainExtent;
    std::vector<SwapChainData>      _swapChainObjects;
    VkCommandPool                   _commandPool;
    std::vector<VkCommandBuffer>    _commandBuffers;
    std::vector<VkSemaphore>        _imageAvailableSemaphores;
    std::vector<VkSemaphore>        _renderFinishedSemaphores;
    std::vector<VkFence>            _inFlightFences;
    bool                            _framebufferResized;
    uint64_t                        _currentFrame;

    GeometryPass                    _geometryPass;
    CompositePass                   _compositePass;
};


} // namespace gu2
