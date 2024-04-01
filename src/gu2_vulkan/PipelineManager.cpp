//
// Project: GraphicsUtils2
// File: PipelineManager.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "PipelineManager.hpp"
#include "Shader.hpp"


using namespace gu2;


void PipelineManager::setDefaultPipelineSettings(const PipelineSettings& defaultPipelineSettings)
{
    _defaultPipelineSettings = defaultPipelineSettings;
}

Pipeline* PipelineManager::getPipeline(
    const Shader* vertexShader,
    const Shader* fragmentShader,
    const std::vector<DescriptorSetLayoutHandle>& descriptorSetLayouts,
    const VkPipelineVertexInputStateCreateInfo& vertexInputInfo
) {
    auto key = std::make_pair(vertexShader, fragmentShader);
    auto pipelineIter = _pipelines.find(key);
    if (pipelineIter == _pipelines.end()) { // no such pipeline, make a new one
        PipelineSettings newSettings = _defaultPipelineSettings;
        newSettings.vertexInputInfo = vertexInputInfo;
        newSettings.vertShaderModule = vertexShader->getShaderModule();
        newSettings.fragShaderModule = fragmentShader->getShaderModule();
        for (const auto& descriptorSetLayout : descriptorSetLayouts)
            newSettings.descriptorSetLayouts.emplace_back(descriptorSetLayout);

        auto [newPipelineIter, _] = _pipelines.emplace(key, newSettings);

        return &newPipelineIter->second;
    }

    return &pipelineIter->second;
}
