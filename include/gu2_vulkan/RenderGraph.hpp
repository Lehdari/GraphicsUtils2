//
// Project: GraphicsUtils2
// File: RenderGraph.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


namespace gu2 {


class RenderPass;


class RenderGraph {
public:
    RenderGraph() = default;
    RenderGraph(const RenderGraph&) = delete;
    RenderGraph(RenderGraph&&) = delete;
    RenderGraph operator=(const RenderGraph&) = delete;
    RenderGraph operator=(RenderGraph&&) = delete;

    void addRenderPass(const RenderPass* renderPass);
    void build();
    void execute();
};


} // namespace gu2
