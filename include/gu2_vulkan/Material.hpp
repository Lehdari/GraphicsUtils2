//
// Project: GraphicsUtils2
// File: Material.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.h>


namespace gu2 {


class GLTFLoader;
class Pipeline;
class PipelineManager;
class Shader;
class Texture;
class VulkanSettings;


class Material {
public:
    static void createMaterialsFromGLTF(
        const GLTFLoader& gltfLoader,
        std::vector<Material>* materials,
        std::vector<Texture>* textures,
        const VulkanSettings& vulkanSettings,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkCommandPool commandPool,
        VkQueue queue
    );

    Material(VkDevice device); // TODO introduce Material::Settings

    void setVertexShader(const Shader& shader);
    void setFragmentShader(const Shader& shader);

    void addTexture(const Texture& texture);
    inline const std::vector<const Texture*>& getTextures() const { return _textures; }

    void createPipeline(
        PipelineManager* pipelineManager,
        VkDescriptorSetLayout materialDescriptorSetLayout,
        VkDescriptorSetLayout meshDescriptorSetLayout,
        const VkPipelineVertexInputStateCreateInfo& vertexInputInfo);

    inline const Pipeline* getPipeline() const noexcept { return _pipeline; }

    // Called after all textures have been added
    void createDescriptorSets(VkDevice device, int framesInFlight);

    VkDescriptorSetLayout getDescriptorSetLayout() const;

    void bind(
        VkCommandBuffer commandBuffer,
        uint32_t currentFrame
    ) const;

    static void createDescriptorPool(VkDevice device, int framesInFlight, uint32_t maxSets);

    // Destroy the static _descriptorSetLayouts and _descriptorPool
    static void destroyDescriptorResources(VkDevice device);

    friend class Pipeline;
    friend class Mesh; // TODO remove

private:
    // Shared descriptor set layouts for materials with same amount of textures (key indicates the n. of textures)
    static std::unordered_map<int32_t, VkDescriptorSetLayout>   _descriptorSetLayouts;
    static VkDescriptorPool                                     _descriptorPool;

    static const VkDescriptorSetLayout& getTextureDescriptorSetLayout(VkDevice device, int32_t nTextures);

    VkDevice                        _device;
    const Shader*                   _vertexShader;
    const Shader*                   _fragmentShader;
    std::vector<const Texture*>     _textures;
    const Pipeline*                 _pipeline;
    std::vector<VkDescriptorSet>    _descriptorSets;
};


} // namespace gu2
