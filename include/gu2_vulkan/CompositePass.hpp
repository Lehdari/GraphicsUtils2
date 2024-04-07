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


class CompositePass : public RenderPass {
public:
    CompositePass(RenderPassSettings settings, VkPhysicalDevice physicalDevice) noexcept;

    void render() final;

private:
    static std::vector<Vec2f>       _quadPositions;
    static std::vector<Vec2f>       _quadTexCoords;
    static std::vector<uint32_t>    _quadIndices;

    Mesh                            _quad;
    Shader                          _vertexShader;
    Shader                          _fragmentShader;
    Material                        _material;
};


} // namespace gu2
