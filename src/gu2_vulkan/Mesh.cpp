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
#include "Texture.hpp"
#include "Util.hpp"
#include "gu2_util/Image.hpp"

#include <stdexcept>


using namespace gu2;


Mesh::Mesh(const VulkanSettings& vulkanSettings,
    VkPhysicalDevice physicalDevice,
    VkDevice device
) :
    _vulkanSettings (&vulkanSettings),
    _physicalDevice (physicalDevice),
    _device         (device)
{
    // Store the device properties in local struct
    vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);

    createUniformBuffers();
    createDescriptorPool();
}

Mesh::~Mesh()
{
    for (int i=0; i<_vulkanSettings->framesInFlight; ++i) {
        vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
        vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);
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

void Mesh::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void Mesh::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(_vulkanSettings->framesInFlight * _vulkanSettings->nBoxes);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(_vulkanSettings->framesInFlight);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(_vulkanSettings->framesInFlight);

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void Mesh::createDescriptorSets(const Texture& texture)
{
    std::vector<VkDescriptorSetLayout> layouts(_vulkanSettings->framesInFlight, _descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(_vulkanSettings->framesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(_vulkanSettings->framesInFlight);
    if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < _vulkanSettings->framesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.getImageView();
        imageInfo.sampler = texture.getSampler();

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = _descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
    }
}

VkDescriptorSetLayout Mesh::getDescriptorSetLayout() const
{
    return _descriptorSetLayout;
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
    VkPipelineLayout pipelineLayout,
    uint32_t currentFrame,
    uint32_t uniformId
) const {
    uint32_t offset = uniformId * padUniformBufferSize(_physicalDeviceProperties, sizeof(gu2::UniformBufferObject));
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
        &_descriptorSets[currentFrame], 1, &offset);
    vkCmdDrawIndexed(commandBuffer, _nIndices, 1, 0, 0, 0);
}

void Mesh::createUniformBuffers()
{
    VkDeviceSize bufferSize = padUniformBufferSize(_physicalDeviceProperties, sizeof(UniformBufferObject)) *
        _vulkanSettings->nBoxes;

    _uniformBuffers.resize(_vulkanSettings->framesInFlight);
    _uniformBuffersMemory.resize(_vulkanSettings->framesInFlight);
    _uniformBuffersMapped.resize(_vulkanSettings->framesInFlight);

    for (size_t i = 0; i < _vulkanSettings->framesInFlight; i++) {
        createBuffer(_physicalDevice, _device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffers[i],
            _uniformBuffersMemory[i]);

        vkMapMemory(_device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
    }
}

void Mesh::updateUniformBuffer(VkExtent2D swapChainExtent, uint32_t currentFrame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    double time = std::chrono::duration<double, std::chrono::seconds::period>(currentTime - startTime).count();

    for (int boxId=0; boxId<_vulkanSettings->nBoxes; ++boxId) {
        gu2::UniformBufferObject ubo;
        // Model matrix
        double boxPosAngle = boxId / (double)_vulkanSettings->nBoxes;
        gu2::Vec3f modelPos(std::cos(boxPosAngle*2.0*M_PI + 0.25*time), 0.0f, std::sin(boxPosAngle*2.0*M_PI + 0.25*time));
        modelPos *= 2.0f;
        ubo.model <<
                  (Eigen::AngleAxisf(0.25*M_PI*time, gu2::Vec3f::UnitX())
                      * Eigen::AngleAxisf(0.5*M_PI*time, gu2::Vec3f::UnitY())
                      * Eigen::AngleAxisf(0.33*M_PI*time, gu2::Vec3f::UnitZ())).toRotationMatrix(),
            modelPos, gu2::Vec3f::Zero().transpose(), 1.0f;

        // View matrix
        gu2::Vec3f target(0.0f, 0.0f, 0.0f);
        gu2::Vec3f source(1.0f, 3.0f, 5.0f);
        gu2::Vec3f up(0.0f, 1.0f, 0.0f);

        gu2::Vec3f forward = (target-source).normalized();
        gu2::Vec3f right = forward.cross(up).normalized();
        gu2::Vec3f up2 = right.cross(forward).normalized();

        gu2::Mat3f viewRotation;
        viewRotation << right.transpose(), up2.transpose(), forward.transpose();
        ubo.view << viewRotation, -viewRotation*source,
            0.0f, 0.0f, 0.0f, 1.0f;

        // Perspective matrix
        float near = 0.1f;
        float far = 10.0f;
        float fov = M_PI/3.0; // 60 degrees
        float aspectRatio = swapChainExtent.width / (float) swapChainExtent.height;
        float r = tanf(fov / 2.0f);

        // Traditional projection matrix
//            ubo.projection <<
//                1.0f/(aspectRatio * r), 0.0f,       0.0f,           0.0f,
//                0.0f,                   -1.0f/r,    0.0f,           0.0f,
//                0.0f,                   0.0f,       far/(far-near), -(far*near)/(far-near),
//                0.0f,                   0.0f,       1.0f,           0.0f;

        // Infinite far-plane, inverted depth projection matrix
        ubo.projection <<
            1.0f/(aspectRatio * r), 0.0f,       0.0f,   0.0f,
            0.0f,                   -1.0f/r,    0.0f,   0.0f,
            0.0f,                   0.0f,       0.0f,   near,
            0.0f,                   0.0f,       1.0f,   0.0f;

        memcpy(reinterpret_cast<uint8_t*>(_uniformBuffersMapped[currentFrame]) +
            padUniformBufferSize(_physicalDeviceProperties, sizeof(ubo))*boxId, &ubo, sizeof(ubo));
    }
}
