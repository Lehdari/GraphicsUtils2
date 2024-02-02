//
// Project: GraphicsUtils2
// File: GLTFLoader.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "GLTFLoader.hpp"

#include <fstream>


using namespace gu2;


void GLTFLoader::load(const Path& filename)
{
    std::ifstream gltfJsonFile(filename);
    _gltfJson = Json::parse(gltfJsonFile);
}
