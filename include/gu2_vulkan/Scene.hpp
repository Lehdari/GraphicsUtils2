//
// Project: GraphicsUtils2
// File: Scene.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "gu2_util/MathTypes.hpp"
#include "gu2_util/GLTFLoader.hpp"

#include <vector>


namespace gu2 {


class Mesh;


struct Scene {
    struct Node {
        Mat4f   transformation;
        Mesh*   mesh;
    };

    std::vector<Node>   nodes;

    void createFromGLFT(const GLTFLoader& gltfLoader, std::vector<Mesh>& meshes);

private:
    void createNodes(
        Mat4d transformation,
        const GLTFLoader::Node& gltfNode,
        const std::vector<GLTFLoader::Node>& gltfNodes,
        const std::vector<GLTFLoader::Mesh>& gltfMeshes,
        std::vector<Mesh>& meshes);
};


} // namespace gu2
