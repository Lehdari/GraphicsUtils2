//
// Project: GraphicsUtils2
// File: VertexAttributesDescription.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "gu2_util/MathTypes.hpp"
#include "Util.hpp"

#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstdint>
#include <vector>


namespace gu2 {


class VertexAttributesDescription {
public:
    template <typename T_VertexOrAttribute>
    void addBinding(uint32_t binding);

    template <typename T_VertexOrAttribute, typename T_Attribute>
    void addAttribute(uint32_t binding, uint32_t location, uint32_t offset=0);

    VkPipelineVertexInputStateCreateInfo getPipelineVertexInputStateCreateInfo() const;

private:
    std::vector<VkVertexInputBindingDescription>    _bindingDecriptions;
    std::vector<VkVertexInputAttributeDescription>  _attributeDecriptions;
};


template <typename T_VertexOrAttribute>
void VertexAttributesDescription::addBinding(uint32_t binding)
{
    for (auto& bindingDecription : _bindingDecriptions) {
        // Try to find existing bindingDecription with the specified binding and update it in case one was
        // found
        if (bindingDecription.binding == binding) {
            bindingDecription.stride = sizeof(T_VertexOrAttribute);
            bindingDecription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            return;
        }
    }

    // No existing bindingDecription with the binding found, create a new one
    _bindingDecriptions.emplace_back(binding, sizeof(T_VertexOrAttribute), VK_VERTEX_INPUT_RATE_VERTEX);

    // Keep the vector sorted for easy comparison with other VertexAttributesDescription objects
    std::sort(_bindingDecriptions.begin(), _bindingDecriptions.end(),
        [](const VkVertexInputBindingDescription& a, const VkVertexInputBindingDescription& b){
            return a.binding < b.binding;
        });
}

template <typename T_Vertex, typename T_Attribute>
void VertexAttributesDescription::addAttribute(uint32_t binding, uint32_t location, uint32_t offset)
{
    addBinding<T_Vertex>(binding);

    for (auto& attributeDescription: _attributeDecriptions) {
        // Try to find existing attributeDescription with the specified binding and location and update it
        // in case one was found
        if (attributeDescription.binding == binding && attributeDescription.location == location) {
            attributeDescription.format = AttributeFormat<T_Attribute>;
            attributeDescription.offset = offset;
            return;
        }
    }

    // No existing attributeDescription with the binding found, create a new one
    _attributeDecriptions.emplace_back(location, binding, AttributeFormat<T_Attribute>, offset);

    // Keep the vector sorted for easy comparison with other VertexAttributesDescription objects
    std::sort(_attributeDecriptions.begin(), _attributeDecriptions.end(),
        [](const VkVertexInputAttributeDescription& a, const VkVertexInputAttributeDescription& b) {
            return a.binding < b.binding || (a.binding == b.binding && a.location < b.location);
        });
}


} // namespace gu2
