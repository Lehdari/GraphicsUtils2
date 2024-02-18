//
// Project: GraphicsUtils2
// File: Mesh.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Mesh.hpp"
#include "Pipeline.hpp"
#include "Texture.hpp"
#include "Util.hpp"
#include "gu2_util/Image.hpp"

#include <stdexcept>


using namespace gu2;


Mesh::Mesh(
    const VulkanSettings& vulkanSettings,
    VkPhysicalDevice physicalDevice,
    VkDevice device
) :
    _physicalDevice (physicalDevice),
    _device         (device)
{
    // Store the device properties in local struct
    vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);
}

Mesh::~Mesh()
{
    for (auto& vertexAttributeBuffer : _vertexAttributeBuffers)
        vkDestroyBuffer(_device, vertexAttributeBuffer, nullptr);
    for (auto& vertexBufferMemory : _vertexBufferMemories)
        vkFreeMemory(_device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(_device, _indexBuffer, nullptr);
    vkFreeMemory(_device, _indexBufferMemory, nullptr);
}

void Mesh::upload(VkCommandPool commandPool, VkQueue queue)
{
    for (const auto& bufferInfo : _vertexBufferInfos) {
        VkBufferUsageFlagBits bufferType = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VkBuffer* buffer = &_indexBuffer;
        VkDeviceMemory* memory = &_indexBufferMemory;
        if (bufferInfo.type == VertexBufferInfo::ATTRIBUTE) {
            _vertexAttributeBuffers.emplace_back();
            _vertexBufferMemories.emplace_back();
            bufferType = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            buffer = &_vertexAttributeBuffers.back();
            memory = &_vertexBufferMemories.back();
        }

        VkDeviceSize bufferSize = bufferInfo.elementSize * _nIndices;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(_physicalDevice, _device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, bufferInfo.data, (size_t)bufferSize);
        vkUnmapMemory(_device, stagingBufferMemory);

        createBuffer(_physicalDevice, _device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferType,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *buffer, *memory);

        copyBuffer(_device, commandPool, queue, stagingBuffer, *buffer, bufferSize);

        vkDestroyBuffer(_device, stagingBuffer, nullptr);
        vkFreeMemory(_device, stagingBufferMemory, nullptr);
    }
}

const VertexAttributesDescription& Mesh::getVertexAttributesDescription() const
{
    return _attributesDescription;
}

void Mesh::bind(VkCommandBuffer commandBuffer)
{
    std::vector<VkDeviceSize> offsets(_vertexAttributeBuffers.size(), 0); // TODO don't generate this every bind
    vkCmdBindVertexBuffers(commandBuffer, 0, _vertexAttributeBuffers.size(), _vertexAttributeBuffers.data(),
        offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT16);
}

void Mesh::draw(
    VkCommandBuffer commandBuffer,
    const Pipeline& pipeline,
    uint32_t currentFrame,
    uint32_t uniformId
) const {
    uint32_t offset = uniformId * padUniformBufferSize(_physicalDeviceProperties, sizeof(gu2::UniformBufferObject));
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._pipelineLayout, 0, 1,
        &(pipeline._descriptorSets[currentFrame]), 1, &offset);
    vkCmdDrawIndexed(commandBuffer, _nIndices, 1, 0, 0, 0);
}
