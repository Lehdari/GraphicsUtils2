//
// Project: GraphicsUtils2
// File: Scene.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Scene.hpp"
#include "Mesh.hpp"
#include "gu2_util/GLTFLoader.hpp"


using namespace gu2;


void Scene::createFromGLFT(const GLTFLoader& gltfLoader, std::vector<Mesh>& meshes)
{
    nodes.clear();

    const auto& gltfScenes = gltfLoader.getScenes();
    const auto& gltfNodes = gltfLoader.getNodes();

    for (const auto& gltfScene : gltfScenes) {
        for (const auto& gltfNodeId : gltfScene.nodes) {
            const auto& gltfNode = gltfNodes.at(gltfNodeId);
            createNodes(Mat4d::Identity(), gltfNode, gltfNodes, meshes);
        }
    }
}

void Scene::createNodes(
    Mat4d transformation,
    const GLTFLoader::Node& gltfNode,
    const std::vector<GLTFLoader::Node>& gltfNodes,
    std::vector<Mesh>& meshes
) {
    transformation = transformation * gltfNode.matrix;
    if (gltfNode.mesh >= 0) {
        nodes.emplace_back(transformation.cast<float>(), &meshes.at(gltfNode.mesh));
        return;
    }

    for (const auto& childNodeId : gltfNode.children) {
        createNodes(transformation, gltfNodes.at(childNodeId), gltfNodes, meshes);
    }
}
