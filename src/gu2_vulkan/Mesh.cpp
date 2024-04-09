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
#include "DescriptorManager.hpp"
#include "Material.hpp"
#include "Pipeline.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "Util.hpp"
#include "gu2_util/GLTFLoader.hpp"

#include <stdexcept>


using namespace gu2;


std::vector<VkBuffer>           Mesh::_uniformBuffers;
std::vector<VkDeviceMemory>     Mesh::_uniformBuffersMemory;
std::vector<void*>              Mesh::_uniformBuffersMapped;


Mesh::Mesh(VkPhysicalDevice physicalDevice, VkDevice device) :
    _physicalDevice (physicalDevice),
    _device         (device),
    _nIndices       (0),
    _indexType      (VK_INDEX_TYPE_UINT16),
    _material       (nullptr)
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

void Mesh::createDescriptorSets(DescriptorManager* descriptorManager, int framesInFlight)
{
    // Allocate descriptor sets
    auto& layout = getDescriptorSetLayout();
    _descriptorSets.clear();
    descriptorManager->allocateDescriptorSets(&_descriptorSets, layout, framesInFlight);

    for (size_t i = 0; i < framesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
    }
}

const DescriptorSetLayoutHandle& Mesh::getDescriptorSetLayout() const
{
    if (_material == nullptr)
        throw std::runtime_error("No material set!");
    return _material->getDescriptorSetLayouts().at(objectDescriptorSetId);
}

void Mesh::setMaterial(const Material* material)
{
    _material = material;
}

const Material& Mesh::getMaterial() const
{
    return *_material;
}

void Mesh::setPipeline(const Pipeline* pipeline)
{
    _pipeline = pipeline;
}

const Pipeline& Mesh::getPipeline() const
{
    return *_pipeline;
}

const VertexAttributesDescription& Mesh::getVertexAttributesDescription() const
{
    return _attributesDescription;
}

void Mesh::bind(VkCommandBuffer commandBuffer) const
{
    std::vector<VkDeviceSize> offsets(_vertexAttributeBuffers.size(), 0); // TODO don't generate this every bind
    vkCmdBindVertexBuffers(commandBuffer, 0, _vertexAttributeBuffers.size(), _vertexAttributeBuffers.data(),
        offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer, 0, _indexType);
}

void Mesh::draw(
    VkCommandBuffer commandBuffer,
    uint32_t currentFrame,
    uint32_t uniformId
) const {
    if (_material == nullptr) return;

    // Bind material
    if (_material != nullptr)
        _material->bind(commandBuffer, currentFrame);

    auto pipelineLayout = _material->getPipeline()->getPipelineLayout();

    uint32_t offset = uniformId * padUniformBufferSize(_physicalDeviceProperties, sizeof(gu2::UniformBufferObject));
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, objectDescriptorSetId, 1,
        &_descriptorSets[currentFrame], 1, &offset);

    vkCmdDrawIndexed(commandBuffer, _nIndices, 1, 0, 0, 0);
}

void Mesh::createUniformBuffers(
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    int framesInFlight,
    uint32_t nUniforms
) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    VkDeviceSize bufferSize = padUniformBufferSize(physicalDeviceProperties, sizeof(UniformBufferObject)) * nUniforms;

    _uniformBuffers.resize(framesInFlight);
    _uniformBuffersMemory.resize(framesInFlight);
    _uniformBuffersMapped.resize(framesInFlight);

    for (size_t i=0; i<framesInFlight; i++) {
        createBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffers[i],
            _uniformBuffersMemory[i]);

        vkMapMemory(device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
    }
}

void Mesh::updateUniformBuffer(const Scene& scene, const Renderer& renderer)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    double time = std::chrono::duration<double, std::chrono::seconds::period>(currentTime - startTime).count();

    int uniformId = 0;
    for (const auto& node : scene.nodes) {
        gu2::UniformBufferObject ubo;
        // Model matrix
//        double boxPosAngle = uniformId / (double)_nUniforms;
//        gu2::Vec3f modelPos(std::cos(boxPosAngle*2.0*M_PI + 0.25*time), 0.0f, std::sin(boxPosAngle*2.0*M_PI + 0.25*time));
//        modelPos *= 2.0f;
//        ubo.model <<
//                  (Eigen::AngleAxisf(0.25*M_PI*time, gu2::Vec3f::UnitX())
//                      * Eigen::AngleAxisf(0.5*M_PI*time, gu2::Vec3f::UnitY())
//                      * Eigen::AngleAxisf(0.33*M_PI*time, gu2::Vec3f::UnitZ())).toRotationMatrix(),
//            modelPos, gu2::Vec3f::Zero().transpose(), 1.0f;
        ubo.model = node.transformation;

        // View matrix
        double tScale = 0.1;
        gu2::Vec3f target(-20.0f*sin((tScale/5.0)*time), 2.5f-2.5f*cos(0.87354*tScale*time), 8.0f*cos((tScale/3.0)*time));
        gu2::Vec3f source(10.0f*cos((tScale/2.0)*time), 1.5f+1.0f*cos(0.34786*tScale*time), 5.8f*sin(tScale*time));
        gu2::Vec3f up(0.0f, 1.0f, 0.0f);

        gu2::Vec3f forward = (target-source).normalized();
        gu2::Vec3f right = forward.cross(up).normalized();
        gu2::Vec3f up2 = right.cross(forward).normalized();

        gu2::Mat3f viewRotation;
        viewRotation << right.transpose(), up2.transpose(), forward.transpose();
        ubo.view << viewRotation, -viewRotation*source,
            0.0f, 0.0f, 0.0f, 1.0f;

        // Perspective matrix
        float near = 0.1f;
        float far = 10.0f;
        float fov = M_PI/3.0; // 60 degrees
        float aspectRatio = renderer.getSwapChainExtent().width / (float) renderer.getSwapChainExtent().height;
        float r = tanf(fov / 2.0f);

        // Traditional projection matrix
//            ubo.projection <<
//                1.0f/(aspectRatio * r), 0.0f,       0.0f,           0.0f,
//                0.0f,                   -1.0f/r,    0.0f,           0.0f,
//                0.0f,                   0.0f,       far/(far-near), -(far*near)/(far-near),
//                0.0f,                   0.0f,       1.0f,           0.0f;

        // Infinite far-plane, inverted depth projection matrix
        ubo.projection <<
                       1.0f/(aspectRatio * r), 0.0f,       0.0f,   0.0f,
            0.0f,                   -1.0f/r,    0.0f,   0.0f,
            0.0f,                   0.0f,       0.0f,   near,
            0.0f,                   0.0f,       1.0f,   0.0f;

        memcpy(reinterpret_cast<uint8_t*>(_uniformBuffersMapped[renderer.getCurrentFrame()]) +
            padUniformBufferSize(renderer._physicalDeviceProperties, sizeof(ubo))*uniformId, &ubo, sizeof(ubo));

        ++uniformId;
    }
}

void Mesh::destroyUniformBuffers(VkDevice device)
{
    assert(_uniformBuffers.size() == _uniformBuffersMemory.size());
    for (int i=0; i<_uniformBuffers.size(); ++i) {
        vkDestroyBuffer(device, _uniformBuffers[i], nullptr);
        vkFreeMemory(device, _uniformBuffersMemory[i], nullptr);
    }
}
