//
// Project: GraphicsUtils2
// File: GLTFLoader.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "Typedef.hpp"


namespace gu2 {


class GLTFLoader {
public:
    void load(const Path& filename);

private:
    Json    _gltfJson;
};


} // namespace gu2
