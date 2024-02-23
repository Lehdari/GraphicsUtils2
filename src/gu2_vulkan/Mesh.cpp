//
// Project: GraphicsUtils2
// File: Mesh.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Mesh.hpp"
#include "Pipeline.hpp"
#include "Texture.hpp"
#include "Util.hpp"
#include "gu2_util/GLTFLoader.hpp"

#include <stdexcept>


using namespace gu2;


void Mesh::createMeshesFromGLTF(
    const GLTFLoader& gltfLoader,
    std::vector<Mesh>* meshes,
    const VulkanSettings& vulkanSettings,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue
) {
    auto& loaderMeshes = gltfLoader.getMeshes();

    // Count the number of primitives (one gu2::Mesh corresponds to a single GLTF mesh primitive, not mesh)
    size_t nPrimitives = 0;
    for (const auto& m : loaderMeshes)
        nPrimitives += m.primitives.size();

    meshes->clear();
    meshes->reserve(nPrimitives);

    for (const auto& m : loaderMeshes) {
        for (const auto& p : m.primitives) {
            meshes->emplace_back(vulkanSettings, physicalDevice, device);
            auto& mesh = meshes->back();

            if (p.indices < 0) {
                fprintf(stderr, "WARNING: Unindexed meshes not currently supported, skipping...\n");
                continue;
            }

            printf("==============\n");
            // Add vertex attributes
            for (const auto& attribute : p.attributes) {
                if (attribute.accessorId < 0)
                    throw std::runtime_error("No accessor ID for attribute \"" + attribute.name + "\" defined");
                auto& accessor = gltfLoader.getAccessors().at(attribute.accessorId);
                if (accessor.bufferView < 0)
                    throw std::runtime_error("No buffer view for attribute \"" + attribute.name + "\" defined");
                auto& bufferView = gltfLoader.getBufferViews().at(accessor.bufferView);
                if (bufferView.buffer < 0)
                    throw std::runtime_error("No buffer for attribute \"" + attribute.name + "\" defined");
                auto& buffer = gltfLoader.getBuffers().at(bufferView.buffer);
                if (buffer.buffer == nullptr)
                    throw std::runtime_error("No buffer loaded");

                printf("%s %ld %ld\n", attribute.name.c_str(), attribute.accessorId, accessor.bufferView);

                if (attribute.name == "POSITION") {
                    if (accessor.componentType == GLTFLoader::Accessor::ComponentType::FLOAT &&
                        accessor.type == "VEC3") {
                        printf("Adding POSITION, count: %lu stride: %lu\n", accessor.count, bufferView.byteStride);
                        mesh.addVertexAttribute(0,
                            reinterpret_cast<Vec3f*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                    }
                    else
                        throw std::runtime_error("Unsupported attribute format for \"POSITION\"");
                }
                else if (attribute.name == "NORMAL") {
                    if (accessor.componentType == GLTFLoader::Accessor::ComponentType::FLOAT &&
                        accessor.type == "VEC3") {
                        printf("Adding NORMAL, count: %lu stride: %lu\n", accessor.count, bufferView.byteStride);
                        mesh.addVertexAttribute(1,
                            reinterpret_cast<Vec3f*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                    }
                    else
                        throw std::runtime_error("Unsupported attribute format for \"NORMAL\"");
                }
                else if (attribute.name == "TANGENT") {
                    if (accessor.componentType == GLTFLoader::Accessor::ComponentType::FLOAT &&
                        accessor.type == "VEC4") {
                        printf("Adding TANGENT, count: %lu stride: %lu\n", accessor.count, bufferView.byteStride);
                        mesh.addVertexAttribute(2,
                            reinterpret_cast<Vec4f*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                    }
                    else
                        throw std::runtime_error("Unsupported attribute format for \"TANGENT\"");
                }
                else if (attribute.name.substr(0, 9) == "TEXCOORD_") {
                    int texCoordId = std::stoi(attribute.name.substr(9, 1));
                    if (accessor.componentType == GLTFLoader::Accessor::ComponentType::FLOAT &&
                        accessor.type == "VEC2") {
                        printf("Adding %s, count: %lu stride: %lu\n", attribute.name.c_str(), accessor.count,
                            bufferView.byteStride);
                        mesh.addVertexAttribute(3+texCoordId,
                            reinterpret_cast<Vec2f*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                    }
                    else
                        throw std::runtime_error("Unsupported attribute format for \"TEXCOORD\"");
                }
            }

            // Add indices
            {
                if (p.indices < 0)
                    throw std::runtime_error("No accessor ID for indices defined");
                auto& accessor = gltfLoader.getAccessors().at(p.indices);
                if (accessor.bufferView < 0)
                    throw std::runtime_error("No buffer view for indices defined");
                auto& bufferView = gltfLoader.getBufferViews().at(accessor.bufferView);
                if (bufferView.buffer < 0)
                    throw std::runtime_error("No buffer for indices defined");
                auto& buffer = gltfLoader.getBuffers().at(bufferView.buffer);
                if (buffer.buffer == nullptr)
                    throw std::runtime_error("No buffer loaded");

                if (accessor.type != "SCALAR")
                    throw std::runtime_error("Invalid accessor for indices, type not \"SCALAR\"");

                printf("Adding indices, count: %lu stride: %lu\n", accessor.count, bufferView.byteStride);
                switch (accessor.componentType) {
                    case GLTFLoader::Accessor::ComponentType::UNSIGNED_SHORT:
                        mesh.setIndices(
                            reinterpret_cast<uint16_t*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                        break;
                    case GLTFLoader::Accessor::ComponentType::UNSIGNED_INT:
                        mesh.setIndices(
                            reinterpret_cast<uint32_t*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                        break;
                    default:
                        throw std::runtime_error("Invalid accessor for indices, component type not \"UNSIGNED_SHORT\" or \"UNSIGNED_INT\"");
                }
            }

            mesh.upload(commandPool, queue);
        }
    }
}


Mesh::Mesh(
    const VulkanSettings& vulkanSettings,
    VkPhysicalDevice physicalDevice,
    VkDevice device
) :
    _physicalDevice (physicalDevice),
    _device         (device)
{
    // Store the device properties in local struct
    vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);
}

Mesh::~Mesh()
{
    for (auto& vertexAttributeBuffer : _vertexAttributeBuffers)
        vkDestroyBuffer(_device, vertexAttributeBuffer, nullptr);
    for (auto& vertexBufferMemory : _vertexBufferMemories)
        vkFreeMemory(_device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(_device, _indexBuffer, nullptr);
    vkFreeMemory(_device, _indexBufferMemory, nullptr);
}

void Mesh::upload(VkCommandPool commandPool, VkQueue queue)
{
    // Sanity check that we have as many VertexBufferInfos as the biggest location indicates
    uint32_t maxLocation = 0;
    for (const auto& bufferInfo : _vertexBufferInfos) {
        if (bufferInfo.location > maxLocation)
            maxLocation = bufferInfo.location;
    }
    if (maxLocation+1 != _vertexBufferInfos.size()-1) // one VertexBufferInfo is for indices
        throw std::runtime_error("The number of buffer infos does not match the vertex attribute locations provided");

    _vertexAttributeBuffers.resize(maxLocation+1);
    _vertexBufferMemories.resize(maxLocation+1);

    for (const auto& bufferInfo : _vertexBufferInfos) {
        VkBufferUsageFlagBits bufferType = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VkBuffer* buffer = &_indexBuffer;
        VkDeviceMemory* memory = &_indexBufferMemory;
        if (bufferInfo.type == VertexBufferInfo::ATTRIBUTE) {
            bufferType = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            buffer = &_vertexAttributeBuffers[bufferInfo.location];
            memory = &_vertexBufferMemories[bufferInfo.location];
        }

        VkDeviceSize bufferSize = bufferInfo.elementSize * bufferInfo.nElements;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(_physicalDevice, _device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, bufferInfo.data, (size_t)bufferSize);
        vkUnmapMemory(_device, stagingBufferMemory);

        createBuffer(_physicalDevice, _device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferType,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *buffer, *memory);

        copyBuffer(_device, commandPool, queue, stagingBuffer, *buffer, bufferSize);

        vkDestroyBuffer(_device, stagingBuffer, nullptr);
        vkFreeMemory(_device, stagingBufferMemory, nullptr);
    }
}

const VertexAttributesDescription& Mesh::getVertexAttributesDescription() const
{
    return _attributesDescription;
}

void Mesh::bind(VkCommandBuffer commandBuffer)
{
    std::vector<VkDeviceSize> offsets(_vertexAttributeBuffers.size(), 0); // TODO don't generate this every bind
    vkCmdBindVertexBuffers(commandBuffer, 0, _vertexAttributeBuffers.size(), _vertexAttributeBuffers.data(),
        offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, VK_INDEX_TYPE_UINT16);
}

void Mesh::draw(
    VkCommandBuffer commandBuffer,
    const Pipeline& pipeline,
    uint32_t currentFrame,
    uint32_t uniformId
) const {
    uint32_t offset = uniformId * padUniformBufferSize(_physicalDeviceProperties, sizeof(gu2::UniformBufferObject));
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline._pipelineLayout, 0, 1,
        &(pipeline._descriptorSets[currentFrame]), 1, &offset);
    vkCmdDrawIndexed(commandBuffer, _nIndices, 1, 0, 0, 0);
}
