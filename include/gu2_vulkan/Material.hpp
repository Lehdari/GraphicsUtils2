//
// Project: GraphicsUtils2
// File: Material.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "Descriptor.hpp"
#include "Shader.hpp"

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>


namespace gu2 {


class GLTFLoader;
class Pipeline;
class PipelineSettings;
class PipelineManager;
class Texture;
class VulkanSettings;


class Material {
public:
    Material(VkDevice device); // TODO introduce Material::Settings

    void setVertexShader(const Shader& shader); // TODO replace with generic addShader
    void setFragmentShader(const Shader& shader); // TODO replace with generic addShader

    void createDescriptorSetLayouts(DescriptorManager* descriptorManager);
    inline const std::vector<DescriptorSetLayoutHandle>& getDescriptorSetLayouts() const noexcept;

    void createPipeline(
        PipelineManager* pipelineManager,
        const VkPipelineVertexInputStateCreateInfo& vertexInputInfo
    );
    void createPipeline(PipelineManager* pipelineManager, const PipelineSettings& pipelineSettings);

    inline const Pipeline* getPipeline() const noexcept { return _pipeline; }

    void addUniform(uint32_t set, uint32_t binding, const Texture& texture);

    // Called after all uniforms have been added
    void createDescriptorSets(DescriptorManager* descriptorManager, int framesInFlight);

    void bind(
        VkCommandBuffer commandBuffer,
        uint32_t currentFrame
    ) const;

private:
    template <typename T_Uniform>
    struct UniformHandle {
        uint32_t            set;
        uint32_t            binding;
        const T_Uniform*    data;
    };


    VkDevice                                _device;
    const Shader*                           _vertexShader;
    const Shader*                           _fragmentShader;
    std::vector<DescriptorSetLayoutInfo>    _descriptorSetLayoutInfos;
    std::vector<DescriptorSetLayoutHandle>  _descriptorSetLayouts;
    const Pipeline*                         _pipeline;
    std::vector<UniformHandle<Texture>>     _textures;
    std::vector<DescriptorSetHandle>        _descriptorSets;
};


const std::vector<DescriptorSetLayoutHandle>& Material::getDescriptorSetLayouts() const noexcept
{
    return _descriptorSetLayouts;
}


} // namespace gu2
