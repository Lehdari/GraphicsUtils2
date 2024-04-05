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
#include "Scene.hpp"
#include "Mesh.hpp"


using namespace gu2;


CompositePass::CompositePass(RenderPassSettings settings) noexcept :
    RenderPass  (std::move(settings)),
    _scene      (nullptr)
{
}

void CompositePass::setScene(const Scene& scene) noexcept
{
    _scene = &scene;
}

void CompositePass::render()
{
    if (_scene == nullptr)
        throw std::runtime_error("No scene to render set");

    for (size_t nodeId=0; nodeId<_scene->nodes.size(); ++nodeId) {
        const auto& node = _scene->nodes[nodeId];
        node.mesh->bind(_commandBuffer);
        node.mesh->draw(_commandBuffer, _currentFrame, nodeId);
    }
}
