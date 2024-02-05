//
// Project: GraphicsUtils2
// File: Pipeline.hpp
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


// TODO relocate
struct UniformBufferObject {
    alignas(16) Mat4f   model;
    alignas(16) Mat4f   view;
    alignas(16) Mat4f   projection;
};


class Texture;
class VulkanSettings;


class Pipeline {
public:
    Pipeline(
        const VulkanSettings& vulkanSettings,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkSurfaceKHR surface,
        WindowObject* window
    );
    Pipeline(const Pipeline& pipeline) = delete;
    Pipeline(Pipeline&& pipeline) = delete;
    Pipeline& operator=(const Pipeline& pipeline) = delete;
    Pipeline& operator=(Pipeline&& pipeline) = delete;
    ~Pipeline();

    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets(const Texture& texture);

    void createImageViews();
    void createDepthResources(VkQueue graphicsQueue);
    void createSwapChain();
    void cleanupSwapChain();
    void recreateSwapChain(VkQueue graphicsQueue);
    void createRenderPass();
    void createGraphicsPipeline(
        VkShaderModule vertShaderModule,
        VkShaderModule fragShaderModule,
        VkPipelineVertexInputStateCreateInfo vertexInputInfo
    );
    void createCommandPool();
    void createFramebuffers();
    void createCommandBuffers();
    void createSyncObjects();

    void recordRenderPassCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    VkCommandPool getCommandPool();
    uint64_t getCurrentFrame() const;

    bool beginRender(VkQueue graphicsQueue, VkCommandBuffer* commandBuffer, uint32_t* imageIndex);
    void endRender(VkQueue graphicsQueue, VkQueue presentQueue, uint32_t imageIndex);
    void framebufferResized();

    // TODO subject to relocation
    void createUniformBuffers();
    void updateUniformBuffer(VkExtent2D swapChainExtent, uint32_t currentFrame);

    friend class Mesh;

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
    VkPipelineLayout                _pipelineLayout;
    VkPipeline                      _graphicsPipeline;
    VkRenderPass                    _renderPass;
    std::vector<VkFramebuffer>      _swapChainFramebuffers;
    VkCommandPool                   _commandPool;
    std::vector<VkCommandBuffer>    _commandBuffers;
    std::vector<VkSemaphore>        _imageAvailableSemaphores;
    std::vector<VkSemaphore>        _renderFinishedSemaphores;
    std::vector<VkFence>            _inFlightFences;
    bool                            _framebufferResized;
    uint64_t                        _currentFrame;

    VkDescriptorSetLayout           _descriptorSetLayout;
    VkDescriptorPool                _descriptorPool;
    std::vector<VkDescriptorSet>    _descriptorSets;

    // TODO subject to relocation (most probably)
    std::vector<VkBuffer>           _uniformBuffers;
    std::vector<VkDeviceMemory>     _uniformBuffersMemory;
    std::vector<void*>              _uniformBuffersMapped;
};


} // namespace gu2
