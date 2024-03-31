//
// Project: GraphicsUtils2
// File: DescriptorManager.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "Descriptor.hpp"


namespace gu2 {


class DescriptorManager {
public:
    DescriptorManager(VkDevice device);
    DescriptorManager(const DescriptorManager&) = delete;
    DescriptorManager(DescriptorManager&&) = delete; // TODO
    DescriptorManager& operator=(const DescriptorManager&) = delete;
    DescriptorManager& operator=(DescriptorManager&&) = delete; // TODO
    ~DescriptorManager();

    DescriptorSetLayoutHandle getDescriptorSetLayout(const DescriptorSetLayoutInfo& layoutInfo);
    void allocateDescriptorSets(
        std::vector<DescriptorSetHandle>* descriptorSets,
        const DescriptorSetLayoutHandle& layout,
        uint32_t nSets=1);

    inline VkDevice getDevice() const noexcept { return _device; }

    friend class DescriptorSetLayoutHandle;
    friend class DescriptorSetHandle;

private:
    struct DescriptorSetLayout {
        DescriptorSetLayoutInfo info;
        VkDescriptorSetLayout   layout;
    };

    VkDevice                                _device;
    std::vector<DescriptorSetLayout>        _descriptorSetLayouts;
    std::vector<DescriptorSetLayoutHandle*> _descriptorSetLayoutHandles;
    VkDescriptorPool                        _descriptorPool;

    size_t registerDescriptorSetLayoutHandle(DescriptorSetLayoutHandle* handle);
    void updateDescriptorSetLayoutHandlePointer(size_t handleId, DescriptorSetLayoutHandle* handle);
    void destroyDescriptorSetLayoutHandle(size_t handleId);
    void updateDescriptorSetLayoutHandles();
    void createDescriptorPool(uint32_t maxSets);
};


} // namespace gu2
