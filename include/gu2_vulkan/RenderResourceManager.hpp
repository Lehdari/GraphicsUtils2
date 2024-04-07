//
// Project: GraphicsUtils2
// File: RenderResourceManager.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "RenderResourceHandle.hpp"


namespace gu2 {


class RenderResourceManager {
public:
    template <typename T_Resource>
    RenderResourceHandle<T_Resource> getResourceHandle();
};


} // namespace gu2
