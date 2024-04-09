//
// Project: GraphicsUtils2
// File: GeometryPass.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "RenderPass.hpp"


namespace gu2 {


class Scene;


class GeometryPass : public RenderPass {
public:
    GeometryPass(RenderPassSettings settings) noexcept;

    void setScene(const Scene& scene) noexcept;

    void render() final;

private:
    const Scene*    _scene;
};


} // namespace gu2
