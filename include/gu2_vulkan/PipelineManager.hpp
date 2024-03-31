//
// Project: GraphicsUtils2
// File: PipelineManager.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "Pipeline.hpp"
#include "Descriptor.hpp"

#include <unordered_map>


struct pair_hash { // TODO remove
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2;
    }
};


namespace gu2 {


class Shader;


class PipelineManager {
public:
    void setDefaultPipelineSettings(const gu2::PipelineSettings& defaultPipelineSettings);

    Pipeline* getPipeline(
        const Shader* vertexShader,
        const Shader* fragmentShader,
        const std::vector<DescriptorSetLayoutHandle>& descriptorSetLayouts,
        const VkPipelineVertexInputStateCreateInfo& vertexInputInfo);

private:
    using PipelineStorage = std::unordered_map<std::pair<const Shader*, const Shader*>, Pipeline, pair_hash>;
    PipelineStorage         _pipelines;
    gu2::PipelineSettings   _defaultPipelineSettings;
};


} // namespace gu2
