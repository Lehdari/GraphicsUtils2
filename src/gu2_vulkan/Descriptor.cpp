//
// Project: GraphicsUtils2
// File: Descriptor.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Descriptor.hpp"
#include "DescriptorManager.hpp"


using namespace gu2;


DescriptorSetLayoutInfo::DescriptorSetLayoutInfo(const DescriptorSetLayoutInfo& other) :
    setId       (other.setId),
    createInfo  (other.createInfo),
    bindings    (other.bindings)
{
    createInfo.pBindings = bindings.data(); // pointer to the copied vector data
}

DescriptorSetLayoutInfo& DescriptorSetLayoutInfo::operator=(const DescriptorSetLayoutInfo& other)
{
    setId = other.setId;
    createInfo = other.createInfo;
    bindings = other.bindings;
    createInfo.pBindings = bindings.data(); // pointer to the copied vector data

    return *this;
}


DescriptorSetLayoutHandle::DescriptorSetLayoutHandle(const DescriptorSetLayoutHandle& other) :
    _manager                (other._manager),
    _handleId               (_manager->registerDescriptorSetLayoutHandle(this)),
    _descriptorSetLayoutId  (other._descriptorSetLayoutId),
    _descriptorSetLayout    (other._descriptorSetLayout)
{
}

DescriptorSetLayoutHandle::DescriptorSetLayoutHandle(DescriptorSetLayoutHandle&& other) noexcept :
    _manager                (other._manager),
    _handleId               (other._handleId),
    _descriptorSetLayoutId  (other._descriptorSetLayoutId),
    _descriptorSetLayout    (other._descriptorSetLayout)
{
    // Update the the handle location
    _manager->updateDescriptorSetLayoutHandlePointer(_handleId, this);
    other._manager = nullptr;
}

DescriptorSetLayoutHandle& DescriptorSetLayoutHandle::operator=(const DescriptorSetLayoutHandle& other)
{
    if (&other == this)
        return *this;

    _manager = other._manager;
    _handleId = _manager->registerDescriptorSetLayoutHandle(this);
    _descriptorSetLayoutId = other._descriptorSetLayoutId;
    _descriptorSetLayout = other._descriptorSetLayout;

    return *this;
}

DescriptorSetLayoutHandle& DescriptorSetLayoutHandle::operator=(DescriptorSetLayoutHandle&& other) noexcept
{
    if (&other == this)
        return *this;

    _manager = other._manager;
    _handleId = other._handleId;
    _descriptorSetLayoutId = other._descriptorSetLayoutId;
    _descriptorSetLayout = other._descriptorSetLayout;

    other._manager = nullptr;

    return *this;
}

DescriptorSetLayoutHandle::~DescriptorSetLayoutHandle()
{
    if (_manager != nullptr)
        _manager->destroyDescriptorSetLayoutHandle(_handleId);
}

DescriptorSetLayoutHandle::DescriptorSetLayoutHandle(DescriptorManager* manager, size_t descriptorSetLayoutId) :
    _manager                (manager),
    _handleId               (_manager->registerDescriptorSetLayoutHandle(this)),
    _descriptorSetLayoutId  (descriptorSetLayoutId),
    _descriptorSetLayout    (&_manager->_descriptorSetLayouts.at(descriptorSetLayoutId).layout)
{
}


DescriptorSetHandle::DescriptorSetHandle(DescriptorSetHandle&& other) noexcept :
    _manager        (other._manager),
    _descriptorSet  (other._descriptorSet)
{
    other._manager = nullptr;
}

DescriptorSetHandle& DescriptorSetHandle::operator=(DescriptorSetHandle&& other) noexcept
{
    if (other._descriptorSet == _descriptorSet)
        return *this;

    _manager = other._manager;
    _descriptorSet = other._descriptorSet;
    other._manager = nullptr;

    return *this;
}

DescriptorSetHandle::~DescriptorSetHandle()
{
    if (_manager != nullptr)
        vkFreeDescriptorSets(_manager->_device, _manager->_descriptorPool, 1, &_descriptorSet);
}

DescriptorSetHandle::DescriptorSetHandle(DescriptorManager* manager, VkDescriptorSet descriptorSet) :
    _manager        (manager),
    _descriptorSet  (descriptorSet)
{
}
