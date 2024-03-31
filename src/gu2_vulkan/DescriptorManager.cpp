//
// Project: GraphicsUtils2
// File: DescriptorManager.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "DescriptorManager.hpp"

#include <array>
#include <stdexcept>


using namespace gu2;


DescriptorManager::DescriptorManager(VkDevice device) :
    _device         (device),
    _descriptorPool (nullptr)
{
    createDescriptorPool(1000); // TODO determine a correct value for maxSets
}

DescriptorManager::~DescriptorManager()
{
    if (_descriptorPool != nullptr)
        vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

    for (const auto& descriptorSetLayout : _descriptorSetLayouts)
        vkDestroyDescriptorSetLayout(_device, descriptorSetLayout.layout, nullptr);
}

DescriptorSetLayoutHandle DescriptorManager::getDescriptorSetLayout(
    const DescriptorSetLayoutInfo& layoutInfo
) {
    // Try to find existing descriptor set layout matching the info
    for (size_t i=0; i<_descriptorSetLayouts.size(); ++i) {
        if (_descriptorSetLayouts[i].info.createInfo == layoutInfo.createInfo)
            return {this, i};
    }

    // No matching descriptor set layout found, create a new one
    auto* layoutData = _descriptorSetLayouts.data();
    _descriptorSetLayouts.emplace_back(layoutInfo, VkDescriptorSetLayout{});
    auto& layout = _descriptorSetLayouts.back();
    if (vkCreateDescriptorSetLayout(_device, &layout.info.createInfo, nullptr, &layout.layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
    // _descriptorSetLayouts got reallocated, update handles so that they point to the reallocated vector
    if (layoutData != _descriptorSetLayouts.data())
        updateDescriptorSetLayoutHandles();

    return {this, _descriptorSetLayouts.size()-1};
}

void DescriptorManager::allocateDescriptorSets(
    std::vector<DescriptorSetHandle>* descriptorSets,
    const DescriptorSetLayoutHandle& layout,
    uint32_t nSets
) {
    // Allocate new descriptor sets
    std::vector<VkDescriptorSetLayout> layouts(nSets, layout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = nSets;
    allocInfo.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> newDescriptorSets(nSets);
    if (vkAllocateDescriptorSets(_device, &allocInfo, newDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    // Create handles and store them to descriptorSets
    descriptorSets->reserve(descriptorSets->size() + nSets);
    for (uint32_t i=0; i<nSets; ++i) {
        DescriptorSetHandle descriptorSet(this, newDescriptorSets[i]);
        descriptorSets->push_back(std::move(descriptorSet));
    }
}

size_t DescriptorManager::registerDescriptorSetLayoutHandle(DescriptorSetLayoutHandle* handle)
{
    // Try to find free handle slot
    for (size_t i=0; i<_descriptorSetLayoutHandles.size(); ++i) {
        if (_descriptorSetLayoutHandles[i] == nullptr) { // free handle found, reuse it
            _descriptorSetLayoutHandles[i] = handle;
            return i;
        }
    }

    // No free slot found, make a new one
    _descriptorSetLayoutHandles.push_back(handle);
    return _descriptorSetLayoutHandles.size()-1;
}

void DescriptorManager::updateDescriptorSetLayoutHandlePointer(size_t handleId, DescriptorSetLayoutHandle* handle)
{
    _descriptorSetLayoutHandles.at(handleId) = handle;
}

void DescriptorManager::destroyDescriptorSetLayoutHandle(size_t handleId)
{
    _descriptorSetLayoutHandles[handleId] = nullptr;

    // TODO destroy descriptor set layout if there is no more handles to it
}

void DescriptorManager::updateDescriptorSetLayoutHandles()
{
    for (auto* handle : _descriptorSetLayoutHandles) {
        handle->_descriptorSetLayout = &_descriptorSetLayouts[handle->_descriptorSetLayoutId].layout;
    }
}

void DescriptorManager::createDescriptorPool(uint32_t maxSets)
{
    std::array<VkDescriptorPoolSize, 2> poolSize{};
    poolSize[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize[0].descriptorCount = static_cast<uint32_t>(maxSets);

    poolSize[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSize[1].descriptorCount = static_cast<uint32_t>(maxSets);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSize.data();
    poolInfo.maxSets = static_cast<uint32_t>(maxSets);

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a descriptor pool!");
    }
}
