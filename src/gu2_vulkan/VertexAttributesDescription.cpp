//
// Project: GraphicsUtils2
// File: VertexAttributesDescription.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "VertexAttributesDescription.hpp"


using namespace gu2;


VkPipelineVertexInputStateCreateInfo VertexAttributesDescription::getPipelineVertexInputStateCreateInfo() const
{
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(_bindingDecriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = _bindingDecriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(_attributeDecriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = _attributeDecriptions.data();

    return vertexInputInfo;
}
