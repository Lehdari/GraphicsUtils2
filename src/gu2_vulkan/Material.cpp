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
#include "Texture.hpp"
#include "VulkanSettings.hpp"

#include "gu2_util/GLTFLoader.hpp"

#include <stdexcept>


using namespace gu2;


std::unordered_map<int32_t, VkDescriptorSetLayout>  Material::_descriptorSetLayouts;
VkDescriptorPool                                    Material::_descriptorPool        {nullptr};


void Material::createMaterialsFromGLTF(
    const GLTFLoader& gltfLoader,
    std::vector<Material>* materials,
    std::vector<Texture>* textures,
    const VulkanSettings& vulkanSettings,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue
) {
    auto& gltfMaterials = gltfLoader.getMaterials();
    materials->clear();
    materials->reserve(gltfMaterials.size());

    auto& gltfTextures = gltfLoader.getTextures();
    auto nTextures = gltfTextures.size();
    textures->clear();
    for (const auto& t : gltfTextures)
        textures->emplace_back(physicalDevice, device);

    auto& gltfImages = gltfLoader.getImages();

    #pragma omp parallel for
    for (decltype(nTextures) i=0; i<nTextures; ++i) {
        const auto& t = gltfTextures[i];
        if (t.source < 0)
            throw std::runtime_error("Texture source not defined");

        const auto& imageFilename = gltfImages.at(t.source).filename;

        auto& texture = textures->at(i);
        texture.loadFromFile(commandPool, queue, imageFilename);

        printf("Added texture %u / %lu from %s\n", i, nTextures, GU2_PATH_TO_STRING(imageFilename));
        fflush(stdout);
    }

    for (const auto& m : gltfMaterials) {
        materials->emplace_back(device);
        auto& material = materials->back();

        // Add textures
        if (m.pbrMetallicRoughness.baseColorTexture.index >= 0)
            material.addTexture(textures->at(m.pbrMetallicRoughness.baseColorTexture.index));
        if (m.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
            material.addTexture(textures->at(m.pbrMetallicRoughness.metallicRoughnessTexture.index));
        if (m.normalTexture.index >= 0)
            material.addTexture(textures->at(m.normalTexture.index));

        material.createDescriptorSets(device, vulkanSettings.framesInFlight);
    }
}

Material::Material(VkDevice device) :
    _device (device)
{
}

void Material::addTexture(const Texture& texture)
{
    _textures.emplace_back(&texture);
}

void Material::createDescriptorSets(VkDevice device, int framesInFlight)
{
    auto& textureLayout = getTextureDescriptorSetLayout(device, static_cast<int32_t>(_textures.size()));

    // Allocate descriptor pools
    if (_descriptorPool == nullptr)
        createDescriptorPool(device, framesInFlight, 512); // TODO do something about the n. of maximum sets

    // Allocate texture descriptor sets
    std::vector<VkDescriptorSetLayout> textureLayouts(framesInFlight, textureLayout);
    VkDescriptorSetAllocateInfo textureAllocInfo{};
    textureAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    textureAllocInfo.descriptorPool = _descriptorPool;
    textureAllocInfo.descriptorSetCount = static_cast<uint32_t>(framesInFlight);
    textureAllocInfo.pSetLayouts = textureLayouts.data();
    _descriptorSets.resize(framesInFlight);
    if (vkAllocateDescriptorSets(device, &textureAllocInfo, _descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < framesInFlight; i++) {
        auto nTextures = _textures.size();
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(_textures.size());
        std::vector<VkDescriptorImageInfo> imageInfos(nTextures);
        for (decltype(nTextures) j=0; j<nTextures; ++j) {
            const auto& texture = _textures[j];
            descriptorWrites.emplace_back();
            auto& descriptorWrite = descriptorWrites.back();

            imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[j].imageView = texture->getImageView();
            imageInfos[j].sampler = texture->getSampler();

            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = _descriptorSets[i];
            descriptorWrite.dstBinding = j;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &imageInfos[j];
        }
        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
    }
}

VkDescriptorSetLayout Material::getDescriptorSetLayout() const
{
    return getTextureDescriptorSetLayout(_device, static_cast<int32_t>(_textures.size()));
}

void Material::bind(
    VkCommandBuffer commandBuffer,
    const VkPipelineLayout& pipelineLayout,
    uint32_t currentFrame
) const
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
        &_descriptorSets[currentFrame], 0, nullptr);
}

void Material::createDescriptorPool(VkDevice device, int framesInFlight, uint32_t maxSets)
{
    VkDescriptorPoolSize poolSize;
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = static_cast<uint32_t>(maxSets * framesInFlight);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(maxSets * framesInFlight);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void Material::destroyDescriptorResources(VkDevice device)
{
    if (_descriptorPool != nullptr)
        vkDestroyDescriptorPool(device, _descriptorPool, nullptr);
    for (const auto& [nTextures, layout] : _descriptorSetLayouts)
        vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

const VkDescriptorSetLayout& Material::getTextureDescriptorSetLayout(VkDevice device, int32_t nTextures)
{
    if (!_descriptorSetLayouts.contains(nTextures)) {
        auto& layout = _descriptorSetLayouts[nTextures];

        // TODO Investigate using an array of textures instead of discrete bindings, see
        // https://kylehalladay.com/blog/tutorial/vulkan/2018/01/28/Textue-Arrays-Vulkan.html

        std::vector<VkDescriptorSetLayoutBinding> bindings(nTextures);
        for (int32_t i=0; i<nTextures; ++i) {
            bindings[i].binding = static_cast<uint32_t>(i);
            bindings[i].descriptorCount = 1;
            bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindings[i].pImmutableSamplers = nullptr;
            bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        printf("Created DescriptorSetLayout for %d textures\n", nTextures);
    }

    return _descriptorSetLayouts.at(nTextures);
}
