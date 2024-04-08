//
// Project: GraphicsUtils2
// File: Descriptor.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include <vulkan/vulkan.h>

#include <vector>


namespace gu2 {


// Descriptor set IDs for dedicated scopes
constexpr uint32_t renderPassDescriptorSetId    {1};
constexpr uint32_t materialDescriptorSetId      {2};
constexpr uint32_t objectDescriptorSetId        {3};


struct DescriptorSetLayoutInfo {
    uint32_t                                    setId       {0};
    VkDescriptorSetLayoutCreateInfo             createInfo  {};
    std::vector<VkDescriptorSetLayoutBinding>   bindings;

    DescriptorSetLayoutInfo() = default;
    DescriptorSetLayoutInfo(const DescriptorSetLayoutInfo& other);
    DescriptorSetLayoutInfo(DescriptorSetLayoutInfo&&) = default;
    DescriptorSetLayoutInfo& operator=(const DescriptorSetLayoutInfo& other);
    DescriptorSetLayoutInfo& operator=(DescriptorSetLayoutInfo&&) = default;
};

// Comparison operator used to check for existing descriptor set layouts in the DescriptorManager
inline bool operator==(const VkDescriptorSetLayoutCreateInfo& info1, const VkDescriptorSetLayoutCreateInfo& info2)
{
    if (info1.flags != info2.flags || info1.bindingCount != info2.bindingCount)
        return false;

    for (size_t i=0; i<info1.bindingCount; ++i) {
        const auto& binding1 = info1.pBindings[i];
        const auto& binding2 = info2.pBindings[i];
        if (binding1.binding != binding2.binding ||
            binding1.descriptorType != binding2.descriptorType ||
            binding1.descriptorCount != binding2.descriptorCount ||
            binding1.stageFlags != binding2.stageFlags ||
            binding1.pImmutableSamplers != binding2.pImmutableSamplers)
            return false;
    }

    return true;
}


class DescriptorManager;


class DescriptorSetLayoutHandle {
public:
    DescriptorSetLayoutHandle(const DescriptorSetLayoutHandle& other);
    DescriptorSetLayoutHandle(DescriptorSetLayoutHandle&& other) noexcept;
    DescriptorSetLayoutHandle& operator=(const DescriptorSetLayoutHandle& other);
    DescriptorSetLayoutHandle& operator=(DescriptorSetLayoutHandle&& other) noexcept;
    ~DescriptorSetLayoutHandle();

    inline operator const VkDescriptorSetLayout&() const noexcept { return *_descriptorSetLayout; }

    friend class DescriptorManager;

private:
    DescriptorManager*              _manager;
    size_t                          _handleId; // handle location in manager storage
    size_t                          _descriptorSetLayoutId; // descriptor set location in manager storage
    const VkDescriptorSetLayout*    _descriptorSetLayout;

    DescriptorSetLayoutHandle(DescriptorManager* manager, size_t descriptorSetLayoutId);
};


class DescriptorSetHandle {
public:
    DescriptorSetHandle(const DescriptorSetHandle& other) = delete;
    DescriptorSetHandle(DescriptorSetHandle&& other) noexcept;
    DescriptorSetHandle& operator=(const DescriptorSetHandle& other) = delete;
    DescriptorSetHandle& operator=(DescriptorSetHandle&& other) noexcept;
    ~DescriptorSetHandle();

    inline operator const VkDescriptorSet&() const noexcept { return _descriptorSet; }
    inline const VkDescriptorSet* operator&() const noexcept { return &_descriptorSet; }

    friend class DescriptorManager;

private:
    DescriptorSetHandle(DescriptorManager* manager, VkDescriptorSet descriptorSet);

    DescriptorManager*  _manager;
    VkDescriptorSet     _descriptorSet;
};


} // namespace gu2
