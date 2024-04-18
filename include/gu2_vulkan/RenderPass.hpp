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


class Texture;


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

    void setOutputExtent(VkExtent2D extent);
    void setInputAttachment(
        uint32_t attachmentId,
        const Texture& texture
    );
    void setOutputAttachment(
        uint32_t attachmentId,
        const Texture& texture
    );
    void setOutputAttachment( // For swap chain images
        uint32_t attachmentId,
        VkImage image,
        VkFormat format,
        uint32_t swapChainImageId=0
    );

    inline VkRenderPass getRenderPass() const noexcept { return _renderPass; }
    inline uint32_t getOutputColorAttachmentsCount() const noexcept { return _nColorAttachments; }

    void build();
    virtual void buildDerived() {};
    virtual void render() = 0;

    friend class Renderer; // TODO change to RenderGraph

protected:
    enum class AttachmentType {
        COLOR,
        DEPTH,
        UNKNOWN
    };

    using OutputAttachmentStorage = std::unordered_map<uint32_t, std::vector<AttachmentHandle>>;
    using InputAttachmentStorage = std::unordered_map<uint32_t, AttachmentHandle>;

    static AttachmentType getAttachmentType(const AttachmentHandle& attachment);

    RenderPassSettings          _settings;
    bool                        _addLayoutTransitionDependency;
    VkCommandBuffer             _commandBuffer; // active command buffer
    uint64_t                    _currentFrame;
    VkRenderPass                _renderPass;
    InputAttachmentStorage      _inputAttachments;
    OutputAttachmentStorage     _outputAttachments;
    VkExtent2D                  _outputExtent;
    uint32_t                    _nSwapChainImages;
    std::vector<VkFramebuffer>  _framebuffers; // Typically contains just one, unless output is to swap chain

private:
    void createRenderPass(
        const std::vector<VkAttachmentReference>& colorAttachmentReferences,
        const VkAttachmentReference* depthAttachmentReference,
        const std::vector<VkAttachmentDescription>& attachmentDescriptions
    );

    void destroyFramebuffers();

    // Called from RenderGraph
    void render(VkCommandBuffer commandBuffer, uint64_t currentFrame, uint32_t swapChainImageId=0);

    std::vector<VkClearValue>   _clearValues;
    uint32_t                    _nColorAttachments;
};


} // namespace gu2
