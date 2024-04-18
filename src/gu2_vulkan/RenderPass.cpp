//
// Project: GraphicsUtils2
// File: RenderPass.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "RenderPass.hpp"
#include "Texture.hpp"

#include <cassert>
#include <stdexcept>


using namespace gu2;


RenderPass::RenderPass(RenderPassSettings settings) noexcept :
    _settings                       (std::move(settings)),
    _addLayoutTransitionDependency  (true),
    _outputExtent                   {0,0},
    _nSwapChainImages               (0),
    _renderPass                     (VK_NULL_HANDLE),
    _nColorAttachments              (0)
{
}

RenderPass::~RenderPass()
{
    destroyFramebuffers();

    if (_renderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(_settings.device, _renderPass, nullptr);
}

void RenderPass::setOutputExtent(VkExtent2D extent)
{
    _outputExtent = extent;
}

void RenderPass::setInputAttachment(
    uint32_t attachmentId,
    const Texture& texture
) {
    if (texture.getImageView() == VK_NULL_HANDLE)
        throw std::runtime_error("Texture has no imageView");

    VkImageLayout layout;
    if ((texture.getProperties().usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) > 0)
        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    else if ((texture.getProperties().usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) > 0)
        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    else
        throw std::runtime_error("Unsupported image usage for attachment");

    _inputAttachments[attachmentId] = AttachmentHandle(
        {
            .format = texture.getProperties().format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = layout
        },
        {attachmentId, layout},
        texture.getImageView()
    );
}

void RenderPass::setOutputAttachment(
    uint32_t attachmentId,
    const Texture& texture
) {
    if (texture.getImageView() == VK_NULL_HANDLE)
        throw std::runtime_error("Texture has no imageView");

    VkImageLayout layout;
    if ((texture.getProperties().usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) > 0)
        layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    else if ((texture.getProperties().usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) > 0)
        layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    else
        throw std::runtime_error("Unsupported image usage for attachment");

    if (texture.getProperties().width != _outputExtent.width ||
        texture.getProperties().height != _outputExtent.height)
        throw std::runtime_error("Output attachment extent does not match render pass output extent");

    auto& attachments = _outputAttachments[attachmentId];
    attachments.clear();
    attachments.emplace_back(
        VkAttachmentDescription{
            .format = texture.getProperties().format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = layout
        },
        VkAttachmentReference{attachmentId, layout},
        texture.getImageView()
    );
    _nSwapChainImages = 1;
}

void RenderPass::setOutputAttachment(
    uint32_t attachmentId,
    VkImage image,
    VkFormat format,
    uint32_t swapChainImageId
) {
    AttachmentHandle attachment(
        VkAttachmentDescription{
            .format = format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        },
        VkAttachmentReference{attachmentId, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
        gu2::createImageView(_settings.device, image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1),
        _settings.device
    );

    auto& attachments = _outputAttachments[attachmentId];
    attachments.resize(std::max<size_t>(swapChainImageId+1, attachments.size()));
    attachments[swapChainImageId] = std::move(attachment);
    _nSwapChainImages = std::max(_nSwapChainImages, swapChainImageId+1);
}

void RenderPass::build()
{
    // Parse attachments for render pass creation
    std::vector<VkAttachmentReference> colorAttachmentReferences;
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    const AttachmentHandle* depthAttachment{nullptr};

    _clearValues.clear();
    for (const auto& [attachmentId, attachments] : _outputAttachments) {
        if (getAttachmentType(attachments.front()) == AttachmentType::COLOR) {
            if (attachments.size() != _nSwapChainImages)
                throw std::runtime_error("Inconsistent amount of color attachments provided");

            colorAttachmentReferences.emplace_back(attachments.front().reference);
            _clearValues.emplace_back(VkClearValue{.color{0.0f, 0.0f, 0.0f, 1.0f}});
        }
        else if (getAttachmentType(attachments.front()) == AttachmentType::DEPTH) {
            if (attachments.size() > 1)
                throw std::runtime_error("More than a single depth/stencil attachment provided");
            depthAttachment = &attachments.front();
            _clearValues.emplace_back(VkClearValue{.depthStencil{0.0f, 0}});
        }
        attachmentDescriptions.resize(std::max<size_t>(attachmentId+1, attachmentDescriptions.size()));
        attachmentDescriptions[attachmentId] = attachments.front().description;
    }

    // Create render pass in case it does not exist (if attachment
    if (_renderPass == VK_NULL_HANDLE)
        createRenderPass(colorAttachmentReferences, depthAttachment == nullptr ? nullptr : &depthAttachment->reference,
            attachmentDescriptions);

    auto nTotalAttachments = _nColorAttachments;
    if (depthAttachment != nullptr)
        ++nTotalAttachments;

    // Parse framebuffer attachments
    std::vector<std::vector<VkImageView>> framebufferAttachments(_nSwapChainImages);
    for (uint32_t swapChainImageId=0; swapChainImageId<_nSwapChainImages; ++swapChainImageId) {
        auto& swapChainImageAttachments = framebufferAttachments[swapChainImageId];
        swapChainImageAttachments.resize(nTotalAttachments);
        if (depthAttachment != nullptr)
            swapChainImageAttachments[depthAttachment->reference.attachment] = depthAttachment->imageView;

        for (uint32_t referenceId=0; referenceId<colorAttachmentReferences.size(); ++referenceId) {
            const auto& colorAttachmentReference = colorAttachmentReferences[referenceId];
            swapChainImageAttachments[referenceId] = _outputAttachments[colorAttachmentReference.attachment]
                [swapChainImageId].imageView;
        }
    }

    // Create framebuffers
    destroyFramebuffers(); // first clear the possible old framebuffers
    _framebuffers.resize(_nSwapChainImages, nullptr);
    for (size_t i=0; i<_nSwapChainImages; ++i) {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(framebufferAttachments[i].size());
        framebufferInfo.pAttachments = framebufferAttachments[i].data();
        framebufferInfo.width = _outputExtent.width;
        framebufferInfo.height = _outputExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_settings.device, &framebufferInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }

    // Run build operations on the derived class
    buildDerived();
}


RenderPass::AttachmentType RenderPass::getAttachmentType(const AttachmentHandle& attachment)
{
    switch (attachment.reference.layout) {
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return AttachmentType::COLOR;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            return AttachmentType::DEPTH;
        default:
            return AttachmentType::UNKNOWN;
    }
}

void RenderPass::createRenderPass(
    const std::vector<VkAttachmentReference>& colorAttachmentReferences,
    const VkAttachmentReference* depthAttachment,
    const std::vector<VkAttachmentDescription>& attachmentDescriptions
) {
    _nColorAttachments = static_cast<uint32_t>(colorAttachmentReferences.size());

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = _nColorAttachments;
    subpass.pColorAttachments = colorAttachmentReferences.data();
    subpass.pDepthStencilAttachment = depthAttachment;

    VkSubpassDependency dependency{};
    if (_addLayoutTransitionDependency) {
        // Add dependency to prevent image transitions before swap chain image is available
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // previous render pass (present)
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // wait for image copy to swapchain image to be ready
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // prevent any modifications to the color
        // attachment: this prevents layout transitions from happening before color attachment output stage
    }

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = _addLayoutTransitionDependency ? 1 : 0;
    renderPassInfo.pDependencies = _addLayoutTransitionDependency ? &dependency : nullptr;

    if (vkCreateRenderPass(_settings.device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void RenderPass::destroyFramebuffers()
{
    for (auto& framebuffer : _framebuffers) {
        vkDestroyFramebuffer(_settings.device, framebuffer, nullptr);
    }
    _framebuffers.clear();
}

void RenderPass::render(VkCommandBuffer commandBuffer, uint64_t currentFrame, uint32_t swapChainImageId)
{
    _commandBuffer = commandBuffer;
    _currentFrame = currentFrame;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _framebuffers[swapChainImageId];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _outputExtent;
    renderPassInfo.clearValueCount = static_cast<uint32_t>(_clearValues.size());
    renderPassInfo.pClearValues = _clearValues.data();

    vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_outputExtent.width);
    viewport.height = static_cast<float>(_outputExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _outputExtent;
    vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);

    render();

    vkCmdEndRenderPass(_commandBuffer);
}
