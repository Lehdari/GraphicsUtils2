//
// Project: GraphicsUtils2
// File: CompositePass.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "RenderPass.hpp"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "Material.hpp"
#include "gu2_util/MathTypes.hpp"


namespace gu2 {


class DescriptorManager;
class PipelineManager;


class CompositePass : public RenderPass {
public:
    CompositePass(
        RenderPassSettings settings,
        VkPhysicalDevice physicalDevice,
        DescriptorManager* descriptorManager,
        PipelineManager* pipelineManager,
        int framesInFlight
    ) noexcept;
    ~CompositePass();

    void createSampler();
    void createQuad(VkCommandPool commandPool, VkQueue queue);
    void createMaterial();
    void updateDescriptorSets();

    const DescriptorSetLayoutHandle& getDescriptorSetLayout() const noexcept;
    const DescriptorSetLayoutInfo& getDescriptorSetLayoutInfo() const noexcept;

    void buildDerived() final;
    void render() final;

private:
    static std::vector<Vec2f>           _quadPositions;
    static std::vector<uint32_t>        _quadIndices;

    DescriptorManager*                  _descriptorManager;
    PipelineManager*                    _pipelineManager;
    int                                 _framesInFlight;

    Mesh                                _quad;
    Shader                              _vertexShader;
    Shader                              _fragmentShader;
    Material                            _material;
    VkSampler                           _sampler;
    std::vector<DescriptorSetHandle>    _descriptorSets;
    bool                                _quadSetupComplete;
    bool                                _materialSetupComplete;
};


} // namespace gu2
