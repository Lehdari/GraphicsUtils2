//
// Project: GraphicsUtils2
// File: Mesh.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "VulkanSettings.hpp"
#include "gu2_util/MathTypes.hpp"
#include "gu2_util/Typedef.hpp"

#include <array>


namespace gu2 {


// TODO will be changed to separate attribute arrays
struct Vertex {
    Vec3f  p;  // position
    Vec3f  c;  // color
    Vec2f  t;  // texture coordinates

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, p);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, c);
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, t);

        return attributeDescriptions;
    }
};

// TODO relocate
struct UniformBufferObject {
    alignas(16) Mat4f   model;
    alignas(16) Mat4f   view;
    alignas(16) Mat4f   projection;
};


class Texture;


class Mesh {
public:
    Mesh(const VulkanSettings& vulkanSettings,
        VkPhysicalDevice physicalDevice,
        VkDevice device);
    Mesh(const Mesh&) = delete; // TODO
    Mesh(Mesh&&) = default;
    Mesh& operator=(const Mesh&) = delete; // TODO
    Mesh& operator=(Mesh&&) = default;
    ~Mesh();

    void createVertexBuffer(VkCommandPool commandPool, VkQueue queue, const std::vector<Vertex>& vertexData);
    void createIndexBuffer(VkCommandPool commandPool, VkQueue queue, const std::vector<uint16_t>& indexData);
    void createUniformBuffers();

    // TODO subject to relocation
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets(const Texture& texture);

    void bind(VkCommandBuffer commandBuffer);
    void draw(
        VkCommandBuffer commandBuffer,
        VkPipelineLayout pipelineLayout,
        uint32_t currentFrame,
        uint32_t uniformId
    ) const;

    void updateUniformBuffer(VkExtent2D swapChainExtent, uint32_t currentFrame); // TODO subject to relocation

    VkDescriptorSetLayout getDescriptorSetLayout() const;

private:
    // TODO Subject to relocation
    const VulkanSettings*           _vulkanSettings;
    VkPhysicalDevice                _physicalDevice;
    VkPhysicalDeviceProperties      _physicalDeviceProperties;
    VkDevice                        _device;

    VkBuffer                        _vertexBuffer;
    VkDeviceMemory                  _vertexBufferMemory;
    VkBuffer                        _indexBuffer;
    uint32_t                        _nIndices;
    VkDeviceMemory                  _indexBufferMemory;
    std::vector<VkBuffer>           _uniformBuffers;
    std::vector<VkDeviceMemory>     _uniformBuffersMemory;
    std::vector<void*>              _uniformBuffersMapped;

    // TODO Subject to relocation
    VkDescriptorSetLayout           _descriptorSetLayout;
    VkDescriptorPool                _descriptorPool;
    std::vector<VkDescriptorSet>    _descriptorSets;
};


} // namespace gu2
