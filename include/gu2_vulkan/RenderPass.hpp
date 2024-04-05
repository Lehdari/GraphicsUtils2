//
// Project: GraphicsUtils2
// File: RenderPass.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "AttachmentHandle.hpp"

#include <unordered_map>
#include <vector>


namespace gu2 {


struct RenderPassSettings {
    VkDevice    device;
};


class RenderPass {
public:
    explicit RenderPass(RenderPassSettings settings) noexcept;
    RenderPass(const RenderPass&) = delete;
    RenderPass(RenderPass&&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;
    RenderPass& operator=(RenderPass&&) = delete;
    ~RenderPass();


    // Interface for setting attachments
    // Interface for defining dependent resources
    // Abstract interface for rendering

    void setOutputAttachment(const AttachmentHandle& attachment, uint32_t swapChainImageId=0);

    void build();
    virtual void render() = 0;

    friend class Renderer; // TODO change to RenderGraph

protected:
    RenderPassSettings  _settings;
    VkCommandBuffer     _commandBuffer; // active command buffer
    uint64_t            _currentFrame;

private:
    void createRenderPass(
        const std::vector<VkAttachmentReference>& colorAttachmentReferences,
        const VkAttachmentReference* depthAttachmentReference,
        const std::vector<VkAttachmentDescription>& attachmentDescriptions
    );

    void destroyFramebuffers();

    // Called from RenderGraph
    void render(VkCommandBuffer commandBuffer, uint32_t swapChainImageId, uint64_t currentFrame);

    using AttachmentStorage = std::unordered_map<uint32_t, std::vector<const AttachmentHandle*>>;

    AttachmentStorage           _outputAttachments;
    VkExtent2D                  _outputExtent;
    uint32_t                    _nSwapChainImages;
    VkRenderPass                _renderPass;
    std::vector<VkFramebuffer>  _framebuffers; // Typically contains just one, unless output is to swap chain
};


} // namespace gu2
