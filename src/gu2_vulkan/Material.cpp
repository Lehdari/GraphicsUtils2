//
// Project: GraphicsUtils2
// File: Material.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Material.hpp"
#include "DescriptorManager.hpp"
#include "PipelineManager.hpp"
#include "Texture.hpp"
#include "VulkanSettings.hpp"

#include "gu2_util/GLTFLoader.hpp"

#include <stdexcept>


using namespace gu2;


Material::Material(VkDevice device) :
    _device         (device),
    _vertexShader   (nullptr),
    _fragmentShader (nullptr),
    _pipeline       (nullptr)
{
}

void Material::setVertexShader(const Shader& shader)
{
    _vertexShader = &shader;
}

void Material::setFragmentShader(const Shader& shader)
{
    _fragmentShader = &shader;
}

void mergeBindings(DescriptorSetLayoutInfo* dest, const DescriptorSetLayoutInfo& src)
{
    for (const auto& srcBinding : src.bindings) {
        for (auto destIter = dest->bindings.begin(); destIter != dest->bindings.end(); ++destIter) {
            auto& destBinding = *destIter;
            if (srcBinding.binding == destBinding.binding) {
                if (srcBinding.descriptorType != destBinding.descriptorType) {
                    throw std::runtime_error("Incompatible shader stages: Different descriptor types for set = "
                        + std::to_string(dest->setId) + ", binding = " + std::to_string(srcBinding.binding));
                }
                if (srcBinding.descriptorCount != destBinding.descriptorCount) {
                    throw std::runtime_error("Incompatible shader stages: Different descriptor counts for set = "
                        + std::to_string(dest->setId) + ", binding = " + std::to_string(srcBinding.binding));
                }
            }
            destIter = dest->bindings.insert(dest->bindings.end(), srcBinding);
        }
    }

    dest->createInfo.bindingCount = dest->bindings.size();
}

void Material::createDescriptorSetLayouts(DescriptorManager* descriptorManager)
{
    if (descriptorManager->getDevice() != _device)
        throw std::runtime_error("DescriptorManager with different Vulkan device provided");

    // TODO make _shaders a member and add generic interface for adding arbitrary number of shaders
    std::vector<const Shader*> _shaders{_vertexShader, _fragmentShader};

    // Merge descriptor set layouts from all shader stages
    std::vector<DescriptorSetLayoutInfo> descriptorSetLayoutInfos;
    for (auto* shader : _shaders) {
        auto shaderDescriptorSetLayouts = shader->getDescriptorSetLayouts();
        for (const auto& shaderDescriptorSetLayout : shaderDescriptorSetLayouts) {
            // Try to find existing layout with the same id, merge binding info if one is found
            bool matchingSetLayoutFound = false;
            for (auto& descriptorSetLayoutInfo : descriptorSetLayoutInfos) {
                if (descriptorSetLayoutInfo.setId == shaderDescriptorSetLayout.setId) {
                    mergeBindings(&descriptorSetLayoutInfo, shaderDescriptorSetLayout);
                    matchingSetLayoutFound = true;
                    break;
                }
            }
            if (!matchingSetLayoutFound) {
                descriptorSetLayoutInfos.emplace_back(shaderDescriptorSetLayout);
            }
        }
    }

    // Create padded list of descriptor set layout infos (preserve order and fill unused sets with dummies, as per
    // https://github.com/KhronosGroup/Vulkan-Docs/issues/1372)
    uint32_t maxLayoutSetId = 0;
    for (const auto& descriptorSetLayoutInfo : descriptorSetLayoutInfos) {
        if (descriptorSetLayoutInfo.setId > maxLayoutSetId)
            maxLayoutSetId = descriptorSetLayoutInfo.setId;
    }
    _descriptorSetLayoutInfos.clear();
    for (uint32_t i=0; i<=maxLayoutSetId; ++i) {
        _descriptorSetLayoutInfos.emplace_back();
        auto& descriptorSetLayoutInfo = _descriptorSetLayoutInfos.back();
        descriptorSetLayoutInfo.setId = i;
        descriptorSetLayoutInfo.createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.createInfo.pNext = nullptr;
        descriptorSetLayoutInfo.createInfo.flags = 0;
        descriptorSetLayoutInfo.createInfo.bindingCount = 0;
        descriptorSetLayoutInfo.createInfo.pBindings = nullptr;
    }
    for (const auto& descriptorSetLayoutInfo : descriptorSetLayoutInfos) {
        _descriptorSetLayoutInfos[descriptorSetLayoutInfo.setId] = descriptorSetLayoutInfo;
    }

    // TODO replace this hack that turns object descriptor UBO types to dynamic
    if (_descriptorSetLayoutInfos.size() > objectDescriptorSetId) {
        for (auto& binding: _descriptorSetLayoutInfos.at(objectDescriptorSetId).bindings) {
            if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        }
    }

    // Create new layouts
    _descriptorSetLayouts.reserve(_descriptorSetLayoutInfos.size());
    for (const auto& descriptorSetLayoutInfo : _descriptorSetLayoutInfos) {
        _descriptorSetLayouts.emplace_back(descriptorManager->getDescriptorSetLayout(descriptorSetLayoutInfo));
    }
}

void Material::createPipeline(
    PipelineManager* pipelineManager,
    const VkPipelineVertexInputStateCreateInfo& vertexInputInfo
) {
    _pipeline = pipelineManager->getPipeline(_vertexShader, _fragmentShader, _descriptorSetLayouts, vertexInputInfo);
}

void Material::createPipeline(
    PipelineManager* pipelineManager,
    const PipelineSettings& pipelineSettings
) {
    _pipeline = pipelineManager->getPipeline(pipelineSettings, _vertexShader, _fragmentShader, _descriptorSetLayouts);
}

void Material::addUniform(uint32_t set, uint32_t binding, const Texture& texture)
{
    // TODO add check for layout correctness (set, binding, type) (_descriptorSetLayoutInfos)
    _textures.emplace_back(set, binding, &texture);
}

void Material::createDescriptorSets(DescriptorManager* descriptorManager, int framesInFlight)
{
    if (_descriptorSetLayouts.size() <= materialDescriptorSetId)
        return;

    // Allocate descriptor sets
    _descriptorSets.clear();
    descriptorManager->allocateDescriptorSets(&_descriptorSets,
        _descriptorSetLayouts.at(materialDescriptorSetId),
        framesInFlight
    );

    // Fill out descriptor binding data
    auto nTextures = _textures.size();
    std::vector<VkDescriptorImageInfo> imageInfos(nTextures);
    for (decltype(nTextures) i=0; i<nTextures; ++i) {
        const auto& texture = *_textures[i].data;

        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfos[i].imageView = texture.getImageView();
        imageInfos[i].sampler = texture.getSampler();
    }

    // Create descriptor write structs
    std::vector<VkWriteDescriptorSet> descriptorWrites;
    for (size_t i=0; i<framesInFlight; i++) {
        const auto& layoutInfo = _descriptorSetLayoutInfos.at(materialDescriptorSetId); // 2 is the descriptor set dedicated for material properties

        for (const auto& binding : layoutInfo.bindings) {
            descriptorWrites.emplace_back();
            auto& descriptorWrite = descriptorWrites.back();

            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = _descriptorSets[i];
            descriptorWrite.dstBinding = binding.binding;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorCount = 1;

            bool descriptorDataFound = false;
            for (decltype(nTextures) k=0; k<nTextures; ++k) {
                const auto& texture = _textures[k];
                if (texture.set == layoutInfo.setId && texture.binding == binding.binding) {
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.pImageInfo = &imageInfos.at(k);
                    descriptorDataFound = true;
                    break;
                }
            }

            if (!descriptorDataFound)
                throw std::runtime_error("No descriptor data found for set = " + std::to_string(layoutInfo.setId)
                    + ", binding = " + std::to_string(binding.binding));
        }
    }

    vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(),
        0, nullptr);
}

void Material::bind(VkCommandBuffer commandBuffer, uint32_t currentFrame) const
{
    _pipeline->bind(commandBuffer);

    if (_descriptorSets.size() <= currentFrame)
        return;

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline->getPipelineLayout(),
        materialDescriptorSetId, 1, &_descriptorSets[currentFrame], 0, nullptr);
}
