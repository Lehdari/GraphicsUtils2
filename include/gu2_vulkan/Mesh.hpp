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


#include "VertexAttributesDescription.hpp"
#include "VulkanSettings.hpp"
#include "gu2_util/MathTypes.hpp"
#include "gu2_util/Typedef.hpp"

#include <array>


namespace gu2 {


class Pipeline;
class Texture;
class GLTFLoader;


class Mesh {
public:
    static void createMeshesFromGLTF(
        const GLTFLoader& gltfLoader,
        std::vector<Mesh>* meshes,
        const VulkanSettings& vulkanSettings,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkCommandPool commandPool,
        VkQueue queue);

    Mesh(
        const VulkanSettings& vulkanSettings,
        VkPhysicalDevice physicalDevice,
        VkDevice device);
    Mesh(const Mesh&) = delete; // TODO
    Mesh(Mesh&&) = default;
    Mesh& operator=(const Mesh&) = delete; // TODO
    Mesh& operator=(Mesh&&) = default;
    ~Mesh();

    // Add vertex attribute from single attribute array
    template <typename T_Attribute>
    void addVertexAttribute(uint32_t location, const T_Attribute* data); // TODO add stride
    // Add vertex attribute from combined(interleaved) vertex data array (offset w.r.t. the beginning of T_Vertex
    // struct)
    template <typename T_Vertex, typename T_Attribute>
    void addVertexAttribute(uint32_t location, std::size_t offset, const T_Vertex* data);
    // Set mesh indices, note that all provided attribute and index arrays must contain at least nIndices elements.
    template <typename T_Index> // T_Index must be either uint16_t or uint32_t
    void setIndices(const T_Index* data, uint32_t nIndices); // TODO add stride

    // Upload the mesh to GPU, all provided vertex attribute and index arrays must remain valid until the execution
    // of upload is finished.
    void upload(VkCommandPool commandPool, VkQueue queue);

    const VertexAttributesDescription& getVertexAttributesDescription() const;

    void bind(VkCommandBuffer commandBuffer);
    void draw(
        VkCommandBuffer commandBuffer,
        const Pipeline& pipeline,
        uint32_t currentFrame,
        uint32_t uniformId
    ) const;

private:
    // Struct containing vertex data input buffer metadata
    struct VertexBufferInfo {
        uint32_t        location;       // Attribute location, not used for index buffer
        const void*     data;           // Pointer to the input data array
        size_t          elementSize;    // Size of an element in bytes
        enum {
            ATTRIBUTE,
            INDEX
        }               type;           // Type enum enables VertexBufferInfo usage for both attribute and index inputs
    };

    // TODO Subject to relocation
    VkPhysicalDevice                _physicalDevice;
    VkPhysicalDeviceProperties      _physicalDeviceProperties;
    VkDevice                        _device;

    VertexAttributesDescription     _attributesDescription;
    std::vector<VertexBufferInfo>   _vertexBufferInfos;
    uint32_t                        _nIndices;

    std::vector<VkBuffer>           _vertexAttributeBuffers;
    std::vector<VkDeviceMemory>     _vertexBufferMemories;
    VkBuffer                        _indexBuffer;
    VkDeviceMemory                  _indexBufferMemory;
};


template <typename T_Attribute>
void Mesh::addVertexAttribute(uint32_t location, const T_Attribute* data)
{
    _attributesDescription.addAttribute<T_Attribute, T_Attribute>(location, location, 0);

    // Rewrite existing handle with the specified location in case one was found
    for (auto& bufferHandle : _vertexBufferInfos) {
        if (bufferHandle.location == location) {
            bufferHandle.data = data;
            bufferHandle.elementSize = sizeof(T_Attribute);
            bufferHandle.type = VertexBufferInfo::ATTRIBUTE;
            return;
        }
    }

    _vertexBufferInfos.emplace_back(location, data, sizeof(T_Attribute), VertexBufferInfo::ATTRIBUTE);
}

template <typename T_Vertex, typename T_Attribute>
void Mesh::addVertexAttribute(uint32_t location, std::size_t offset, const T_Vertex* data)
{
    _attributesDescription.addAttribute<T_Vertex, T_Attribute>(0, location, offset);

    // Rewrite existing handle with the specified location in case one was found
    for (auto& bufferHandle : _vertexBufferInfos) {
        if (bufferHandle.location == location) {
            bufferHandle.data = data;
            bufferHandle.elementSize = sizeof(T_Vertex);
            bufferHandle.type = VertexBufferInfo::ATTRIBUTE;
            return;
        }
    }

    _vertexBufferInfos.emplace_back(location, data, sizeof(T_Vertex), VertexBufferInfo::ATTRIBUTE);
}

template <typename T_Index>
void Mesh::setIndices(const T_Index* data, uint32_t nIndices)
{
    _vertexBufferInfos.emplace_back(0, data, sizeof(T_Index), VertexBufferInfo::INDEX);
    _nIndices = nIndices;
}


} // namespace gu2
