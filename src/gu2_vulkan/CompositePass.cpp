//
// Project: GraphicsUtils2
// File: CompositePass.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "CompositePass.hpp"


using namespace gu2;


std::vector<Vec2f> CompositePass::_quadPositions {
    {-1.0f, -1.0f},
    {1.0f, -1.0f},
    {-1.0f, 1.0f},
    {1.0f, 1.0f}
};

std::vector<Vec2f> CompositePass::_quadTexCoords {
    {0.0f, 0.0f},
    {1.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 1.0f}
};

std::vector<uint32_t> CompositePass::_quadIndices {
    0, 1, 3, 0, 3, 2
};


CompositePass::CompositePass(RenderPassSettings settings, VkPhysicalDevice physicalDevice) noexcept :
    RenderPass      (std::move(settings)),
    _quad           (physicalDevice, _settings.device),
    _vertexShader   (_settings.device),
    _fragmentShader (_settings.device),
    _material       (_settings.device)
{
    _quad.addVertexAttribute(0, _quadPositions.data(), _quadPositions.size());
    _quad.setIndices(_quadIndices.data(), _quadIndices.size());

    _vertexShader.loadFromFile(gu2::Path(GU2_SHADER_DIR) / "vertex/pbr_lighting.glsl");
    _fragmentShader.loadFromFile(gu2::Path(GU2_SHADER_DIR) / "fragment/pbr_lighting.glsl");
    _material.load
}

void CompositePass::render()
{
    _quad.draw(_commandBuffer, _currentFrame);
}
