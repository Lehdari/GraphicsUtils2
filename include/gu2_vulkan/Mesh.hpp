//
// Project: GraphicsUtils2
// File: Mesh.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "Descriptor.hpp"
#include "VertexAttributesDescription.hpp"
#include "VulkanSettings.hpp"
#include "gu2_util/MathTypes.hpp"
#include "gu2_util/Typedef.hpp"

#include <array>


namespace gu2 {


// TODO relocate
struct UniformBufferObject {
    alignas(16) Mat4f   model;
    alignas(16) Mat4f   view;
    alignas(16) Mat4f   projection;
};


class DescriptorManager;
class GLTFLoader;
class Material;
class Pipeline;
class Texture;
class Renderer;
class Scene;


class Mesh {
public:
    Mesh(VkPhysicalDevice physicalDevice, VkDevice device);
    Mesh(const Mesh&) = delete; // TODO
    Mesh(Mesh&&) = default;
    Mesh& operator=(const Mesh&) = delete; // TODO
    Mesh& operator=(Mesh&&) = default;
    ~Mesh();

    // Add vertex attribute from single attribute array
    template <typename T_Attribute>
    void addVertexAttribute(uint32_t location, const T_Attribute* data, size_t nElements, uint32_t stride=0);
    // Add vertex attribute from combined(interleaved) vertex data array (offset w.r.t. the beginning of T_Vertex
    // struct)
    template <typename T_Vertex, typename T_Attribute>
    void addVertexAttribute(uint32_t location, std::size_t offset, const T_Vertex* data, size_t nElements);
    // Set mesh indices, note that all provided attribute and index arrays must contain at least nIndices elements.
    template <typename T_Index> // T_Index must be either uint16_t or uint32_t
    void setIndices(const T_Index* data, uint32_t nIndices, uint32_t stride=0);

    // Upload the mesh to GPU, all provided vertex attribute and index arrays must remain valid until the execution
    // of upload is finished.
    void upload(VkCommandPool commandPool, VkQueue queue);

    void createDescriptorSets(DescriptorManager* descriptorManager, int framesInFlight);

    const DescriptorSetLayoutHandle& getDescriptorSetLayout() const;

    void setMaterial(const Material* material);
    const Material& getMaterial() const;

    void setPipeline(const Pipeline* pipeline);
    const Pipeline& getPipeline() const;

    const VertexAttributesDescription& getVertexAttributesDescription() const;

    void bind(VkCommandBuffer commandBuffer) const;
    void draw(
        VkCommandBuffer commandBuffer,
        uint32_t currentFrame,
        uint32_t uniformId
    ) const;

    // TODO subject to relocation
    static void createUniformBuffers(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        int framesInFlight,
        uint32_t nUniforms);
    static void updateUniformBuffer(const Scene& scene, const Renderer& renderer);
    static void destroyUniformBuffers(VkDevice device);

private:
    // Struct containing vertex data input buffer metadata
    struct VertexBufferInfo {
        uint32_t        location;       // Attribute location, not used for index buffer
        const void*     data;           // Pointer to the input data array
        size_t          elementSize;    // Size of an element in bytes
        size_t          nElements;      // Number of elements to be uploaded
        enum {
            ATTRIBUTE,
            INDEX
        }               type;           // Type enum enables VertexBufferInfo usage for both attribute and index inputs
    };

    VkPhysicalDevice                    _physicalDevice;
    VkPhysicalDeviceProperties          _physicalDeviceProperties;
    VkDevice                            _device;

    VertexAttributesDescription         _attributesDescription;
    std::vector<VertexBufferInfo>       _vertexBufferInfos;
    uint32_t                            _nIndices;
    VkIndexType                         _indexType;
    const Material*                     _material;
    const Pipeline*                     _pipeline;

    std::vector<VkBuffer>               _vertexAttributeBuffers;
    std::vector<VkDeviceMemory>         _vertexBufferMemories;
    VkBuffer                            _indexBuffer;
    VkDeviceMemory                      _indexBufferMemory;
    std::vector<DescriptorSetHandle>    _descriptorSets;

    // TODO store actual uniform buffers elsewhere (use dynamic uniforms)
    static std::vector<VkBuffer>        _uniformBuffers;
    static std::vector<VkDeviceMemory>  _uniformBuffersMemory;
    static std::vector<void*>           _uniformBuffersMapped;
};


template <typename T_Attribute>
void Mesh::addVertexAttribute(uint32_t location, const T_Attribute* data, size_t nElements, uint32_t stride)
{
    // TODO in case that the stride indicates a non-continuous buffer an internal, tightly packed copy needs to be made
    if (stride != 0 && stride != sizeof(T_Attribute))
        throw std::runtime_error("Currently only tightly packed vertex attribute input buffers are supported!");

    _attributesDescription.addAttribute<T_Attribute, T_Attribute>(location, location, 0, stride);

    // Rewrite existing handle with the specified location in case one was found
    for (auto& bufferHandle : _vertexBufferInfos) {
        if (bufferHandle.location == location) {
            bufferHandle.data = data;
            bufferHandle.elementSize = sizeof(T_Attribute);
            bufferHandle.nElements = nElements;
            bufferHandle.type = VertexBufferInfo::ATTRIBUTE;
            return;
        }
    }

    _vertexBufferInfos.emplace_back(location, data, sizeof(T_Attribute), nElements, VertexBufferInfo::ATTRIBUTE);
}

template <typename T_Vertex, typename T_Attribute>
void Mesh::addVertexAttribute(uint32_t location, std::size_t offset, const T_Vertex* data, size_t nElements)
{
    _attributesDescription.addAttribute<T_Vertex, T_Attribute>(0, location, offset);

    // Rewrite existing handle with the specified location in case one was found
    for (auto& bufferHandle : _vertexBufferInfos) {
        if (bufferHandle.location == location) {
            bufferHandle.data = data;
            bufferHandle.elementSize = sizeof(T_Vertex);
            bufferHandle.nElements = nElements;
            bufferHandle.type = VertexBufferInfo::ATTRIBUTE;
            return;
        }
    }

    _vertexBufferInfos.emplace_back(location, data, sizeof(T_Vertex), nElements, VertexBufferInfo::ATTRIBUTE);
}

template <typename T_Index>
void Mesh::setIndices(const T_Index* data, uint32_t nIndices, uint32_t stride)
{
    // TODO in case that the stride indicates a non-continuous buffer an internal, tightly packed copy needs to be made
    if (stride != 0 && stride != sizeof(T_Index))
        throw std::runtime_error("Currently only tightly packed index input buffers are supported!");

    _nIndices = nIndices;
    _vertexBufferInfos.emplace_back(0, data, sizeof(T_Index), _nIndices, VertexBufferInfo::INDEX);

    if constexpr (std::is_same_v<T_Index, uint16_t>)
        _indexType = VK_INDEX_TYPE_UINT16;
    else if constexpr (std::is_same_v<T_Index, uint32_t>)
        _indexType = VK_INDEX_TYPE_UINT32;
}


} // namespace gu2
