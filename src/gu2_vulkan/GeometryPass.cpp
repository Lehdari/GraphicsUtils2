//
// Project: GraphicsUtils2
// File: GeometryPass.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "GeometryPass.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"


using namespace gu2;


GeometryPass::GeometryPass(RenderPassSettings settings) noexcept:
    RenderPass  (std::move(settings)),
    _scene      (nullptr)
{
    _addLayoutTransitionDependency = false;
}

void GeometryPass::setScene(const Scene& scene) noexcept
{
    _scene = &scene;
}

void GeometryPass::render()
{
    if (_scene == nullptr)
        throw std::runtime_error("No scene to render set");

    for (size_t nodeId=0; nodeId<_scene->nodes.size(); ++nodeId) {
        const auto& node = _scene->nodes[nodeId];
        node.mesh->bind(_commandBuffer);
        node.mesh->draw(_commandBuffer, _currentFrame, nodeId);
    }
}
