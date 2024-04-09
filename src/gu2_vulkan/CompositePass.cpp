//
// Project: GraphicsUtils2
// File: CompositePass.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "CompositePass.hpp"
#include "DescriptorManager.hpp"
#include "Pipeline.hpp"


using namespace gu2;


std::vector<Vec2f> CompositePass::_quadPositions {
    {-1.0f, -1.0f},
    {1.0f, -1.0f},
    {-1.0f, 1.0f},
    {1.0f, 1.0f}
};

std::vector<uint32_t> CompositePass::_quadIndices {
    0, 3, 1, 0, 2, 3
};


CompositePass::CompositePass(
    RenderPassSettings settings,
    VkPhysicalDevice physicalDevice,
    DescriptorManager* descriptorManager,
    PipelineManager* pipelineManager,
    int framesInFlight
) noexcept :
    RenderPass              (std::move(settings)),
    _descriptorManager      (descriptorManager),
    _pipelineManager        (pipelineManager),
    _framesInFlight         (framesInFlight),
    _quad                   (physicalDevice, _settings.device),
    _vertexShader           (_settings.device),
    _fragmentShader         (_settings.device),
    _material               (_settings.device),
    _quadSetupComplete      (false),
    _materialSetupComplete  (false)
{
    createSampler();
}

CompositePass::~CompositePass()
{
    vkDestroySampler(_settings.device, _sampler, nullptr);
}

void CompositePass::createSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

    if (vkCreateSampler(_settings.device, &samplerInfo, nullptr, &_sampler) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture sampler!");
    }
}

void CompositePass::createQuad(
    VkCommandPool commandPool,
    VkQueue queue
) {
    if (_quadSetupComplete)
        return;

    _quad.addVertexAttribute(0, _quadPositions.data(), _quadPositions.size());
    _quad.setIndices(_quadIndices.data(), _quadIndices.size());
    _quad.upload(commandPool, queue);

    _quadSetupComplete = true;
}

void CompositePass::createMaterial()
{
    if (_materialSetupComplete)
        return;

    _vertexShader.loadFromFile(gu2::Path(GU2_SHADER_DIR) / "vertex/pbr_lighting.glsl");
    _fragmentShader.loadFromFile(gu2::Path(GU2_SHADER_DIR) / "fragment/pbr_lighting.glsl");

    _material.setVertexShader(_vertexShader);
    _material.setFragmentShader(_fragmentShader);
    _material.createDescriptorSetLayouts(_descriptorManager);
    PipelineSettings pipelineSettings {
        .device = _settings.device,
        .renderPass = _renderPass,
        .colorAttachmentCount = getOutputColorAttachmentsCount(),
        .vertexInputInfo = _quad.getVertexAttributesDescription().getPipelineVertexInputStateCreateInfo()
    };
    _material.createPipeline(_pipelineManager, pipelineSettings);
    _material.createDescriptorSets(_descriptorManager, _framesInFlight);
    _quad.setMaterial(&_material);

    // Allocate descriptor sets
    _descriptorSets.clear();
    _descriptorManager->allocateDescriptorSets(&_descriptorSets, getDescriptorSetLayout(), _framesInFlight);

    _materialSetupComplete = true;
}

void CompositePass::updateDescriptorSets()
{
    // Fill out descriptor binding data
    std::vector<VkDescriptorImageInfo> imageInfos(_inputAttachments.size()*_framesInFlight);

    // Create descriptor write structs
    const auto& layoutInfo = getDescriptorSetLayoutInfo();
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    size_t imageInfoRunningId = 0;
    for (size_t i=0; i<_framesInFlight; i++) {
        for (const auto& binding : layoutInfo.bindings) {
            descriptorWrites.emplace_back();
            auto& descriptorWrite = descriptorWrites.back();

            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = _descriptorSets[i];
            descriptorWrite.dstBinding = binding.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            auto& inputAttachment = _inputAttachments.at(binding.binding);
            imageInfos[imageInfoRunningId] = VkDescriptorImageInfo{
                .sampler = _sampler,
                .imageView = inputAttachment.imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            descriptorWrite.pImageInfo = &imageInfos.at(imageInfoRunningId++);
        }
    }

    vkUpdateDescriptorSets(_settings.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
        0, nullptr);
}

const DescriptorSetLayoutHandle& CompositePass::getDescriptorSetLayout() const noexcept
{
    return _material.getDescriptorSetLayouts().at(renderPassDescriptorSetId);
}

const DescriptorSetLayoutInfo& CompositePass::getDescriptorSetLayoutInfo() const noexcept
{
    return _material.getDescriptorSetLayoutInfos().at(renderPassDescriptorSetId);
}

void CompositePass::buildDerived()
{
    createMaterial();
    updateDescriptorSets();
}

void CompositePass::render()
{
    vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        _material.getPipeline()->getPipelineLayout(), renderPassDescriptorSetId, 1, &_descriptorSets[_currentFrame],
        0, nullptr);

    _quad.bind(_commandBuffer);
    _quad.draw(_commandBuffer, _currentFrame);
}
