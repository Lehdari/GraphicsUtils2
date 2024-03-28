//
// Project: GraphicsUtils2
// File: Pipeline.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "backend.hpp"
#include "gu2_util/MathTypes.hpp"

#include <vulkan/vulkan.h>

#include <vector>


namespace gu2 {


class Material;
class Mesh;
class Texture;
class Scene;
class VulkanSettings;


struct PipelineSettings {
    VkDevice                                device                      {nullptr};

    // Renderer info
    VkRenderPass                            renderPass                  {nullptr};
    VkExtent2D                              swapChainExtent;

    // Shader modules
    VkShaderModule                          vertShaderModule            {nullptr};
    VkShaderModule                          fragShaderModule            {nullptr};

    // Mesh / Material configuration
    VkPipelineVertexInputStateCreateInfo    vertexInputInfo;
    VkDescriptorSetLayout                   materialDescriptorSetLayout {nullptr};
    VkDescriptorSetLayout                   meshDescriptorSetLayout     {nullptr};
};


class Pipeline {
public:
    // Creates the necessary pipelines and assigns them to meshes
    static void createPipelines(
        const PipelineSettings& pipelineDefaultSettings, // mesh / material configuration will be overwritten
        std::vector<Pipeline>* pipelines,
        std::vector<Mesh>* meshes
    );

    Pipeline(const PipelineSettings& settings = PipelineSettings());
    Pipeline(const Pipeline& pipeline) = delete;
    Pipeline(Pipeline&& pipeline) = default;
    Pipeline& operator=(const Pipeline& pipeline) = delete;
    Pipeline& operator=(Pipeline&& pipeline) = default;
    ~Pipeline();

    void createGraphicsPipeline(VkShaderModule vertShaderModule, VkShaderModule fragShaderModule);

    const PipelineSettings& getSettings() const;

    void bind(VkCommandBuffer commandBuffer) const;

    friend class Mesh;
    friend class Material;

private:
    PipelineSettings    _settings;

    VkPipelineLayout    _pipelineLayout;
    VkPipeline          _graphicsPipeline;
};


} // namespace gu2
