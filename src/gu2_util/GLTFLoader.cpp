//
// Project: GraphicsUtils2
// File: GLTFLoader.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "GLTFLoader.hpp"
#include "MathUtils.hpp"

#include <fstream>
#include <iostream> // TODO temp

#ifdef __unix__
#include <fcntl.h>
#include <sys/mman.h>
#else
#error "Memory mapping only supported on unix systems"
#endif // __unix__


using namespace gu2;


GLTFLoader::Buffer::~Buffer()
{
    if (buffer != nullptr) {
        munmap(buffer, bufferSize);
    }
}

void GLTFLoader::readFromFile(const Path& filename)
{
    std::ifstream gltfJsonFile(filename);
    _gltfJson = Json::parse(gltfJsonFile);

    // Parse scenes
    if (_gltfJson.contains("scenes")) {
        const auto& scenes = _gltfJson["scenes"];
        _scenes.clear();
        _scenes.reserve(scenes.size());
        for (const auto& scene: scenes) {
            _scenes.emplace_back();
            auto& s = _scenes.back();
            if (scene.contains("nodes")) {
                for (const auto& nodeId: scene["nodes"])
                    s.nodes.push_back(nodeId);
            }
        }
        printf("Parsed %lu scenes.\n", _scenes.size());
    }

    // Parse nodes
    if (_gltfJson.contains("nodes")) {
        const auto& nodes = _gltfJson["nodes"];
        _nodes.clear();
        _nodes.reserve(nodes.size());
        for (const auto& node: nodes) {
            _nodes.emplace_back();
            auto& n = _nodes.back();

            if (node.contains("matrix")) { // explicit 4x4 transformation matrix takes precedence
                n.matrix = node["matrix"].template get<Mat4d>();
            }
            else { // technically the node shouldn't contain both but we're supporting it anyway
                if (node.contains("rotation")) {
                    Quatd r = node["rotation"].template get<Quatd>();
                    n.matrix.block<3, 3>(0, 0) = r.toRotationMatrix();
                }
                if (node.contains("scale")) {
                    for (int i = 0; i < 3; ++i)
                        n.matrix(i, i) *= node["scale"][i].get<double>();
                }
                if (node.contains("translation")) {
                    n.matrix.block<3, 1>(0, 3) = node["translation"].template get<Vec3d>();
                }
            }

            if (node.contains("mesh"))
                n.mesh = node["mesh"];

            if (node.contains("children")) {
                for (const auto& nodeId: node["children"])
                    n.children.push_back(nodeId);
            }
        }
        printf("Parsed %lu nodes.\n", _nodes.size());
    }

    // Parse meshes
    if (_gltfJson.contains("meshes")) {
        const auto& meshes = _gltfJson["meshes"];
        int64_t primitiveId = 0;
        _meshes.clear();
        _meshes.reserve(meshes.size());
        for (const auto& mesh : meshes) {
            if (!mesh.contains("primitives"))
                throw std::runtime_error("Invalid GLTF file: mesh object does not contain the required \"primitives\" array.");

            _meshes.emplace_back();
            auto& m = _meshes.back();

            for (const auto& primitive : mesh["primitives"]) {
                if (!primitive.contains("attributes"))
                    throw std::runtime_error("Invalid GLTF file: primitive object does not contain the required \"attributes\" array.");

                m.primitives.emplace_back();
                auto& p = m.primitives.back();
                p.id = primitiveId++;

                for (const auto& [name, accessorId] : primitive["attributes"].items()) {
                    p.attributes.emplace_back(name, accessorId);
                }

                if (primitive.contains("indices"))
                    p.indices = primitive["indices"];

                if (primitive.contains("mode"))
                    p.mode = static_cast<Mesh::Primitive::Mode>(primitive["mode"].get<uint8_t>());
            }
        }
        printf("Parsed %lu meshes.\n", _meshes.size());
    }

    // Parse buffers
    if (_gltfJson.contains("buffers")) {
        const auto& buffers = _gltfJson["buffers"];
        _buffers.clear();
        _buffers.reserve(buffers.size());
        for (const auto& buffer : buffers) {
            if (!buffer.contains("byteLength"))
                throw std::runtime_error("Invalid GLTF file: buffer object does not contain the required \"byteLength\" property.");

            _buffers.emplace_back();
            auto& b = _buffers.back();

            b.byteLength = buffer["byteLength"];

            if (buffer.contains("uri")) {
                b.filename = filename.parent_path() / buffer["uri"];

                // memory map the file (TODO wrap into an utility function that provides fallback on non-unix systems)
                int file = open(GU2_PATH_TO_STRING(b.filename), O_RDONLY, 0);
                if (file < 0)
                    throw std::runtime_error("Unable to open file " + b.filename.string());

                b.bufferSize = lseek(file, (size_t)0, SEEK_END);
                if (b.bufferSize < 0)
                    throw std::runtime_error("Unable to determine file size");

                b.buffer = reinterpret_cast<char*>(mmap(nullptr, b.bufferSize, PROT_READ, MAP_SHARED, file, 0));
            }
        }
        printf("Parsed %lu buffers.\n", _buffers.size());
    }

    // Parse buffer views
    if (_gltfJson.contains("bufferViews")) {
        const auto& bufferViews = _gltfJson["bufferViews"];
        _bufferViews.clear();
        _bufferViews.reserve(bufferViews.size());
        for (const auto& bufferView : bufferViews) {
            if (!bufferView.contains("buffer"))
                throw std::runtime_error("Invalid GLTF file: bufferView object does not contain the required \"buffer\" property.");
            if (!bufferView.contains("byteLength"))
                throw std::runtime_error("Invalid GLTF file: bufferView object does not contain the required \"byteLength\" property.");

            _bufferViews.emplace_back();
            auto& b = _bufferViews.back();

            b.buffer = bufferView["buffer"];
            b.byteLength = bufferView["byteLength"];

            if (bufferView.contains("byteOffset"))
                b.byteOffset = bufferView["byteOffset"];

            if (bufferView.contains("byteStride"))
                b.byteStride = bufferView["byteStride"];
        }
        printf("Parsed %lu bufferViews.\n", _bufferViews.size());
    }

    // Parse accessors
    if (_gltfJson.contains("accessors")) {
        const auto& accessors = _gltfJson["accessors"];
        _accessors.clear();
        _accessors.reserve(accessors.size());
        for (const auto& accessor : accessors) {
            if (!accessor.contains("componentType"))
                throw std::runtime_error("Invalid GLTF file: accessor object does not contain the required \"componentType\" property.");
            if (!accessor.contains("count"))
                throw std::runtime_error("Invalid GLTF file: accessor object does not contain the required \"count\" property.");
            if (!accessor.contains("type"))
                throw std::runtime_error("Invalid GLTF file: accessor object does not contain the required \"type\" property.");

            _accessors.emplace_back();
            auto& a = _accessors.back();

            a.componentType = static_cast<Accessor::ComponentType>(accessor["componentType"]);
            a.count = accessor["count"];
            a.type = accessor["type"];

            if (accessor.contains("bufferView"))
                a.bufferView = accessor["bufferView"];

            if (accessor.contains("byteOffset"))
                a.byteOffset = accessor["byteOffset"];
        }
        printf("Parsed %lu accessors.\n", _accessors.size());
    }
}

const std::vector<GLTFLoader::Scene>& GLTFLoader::getScenes() const noexcept
{
    return _scenes;
}

const std::vector<GLTFLoader::Node>& GLTFLoader::getNodes() const noexcept
{
    return _nodes;
}

const std::vector<GLTFLoader::Mesh>& GLTFLoader::getMeshes() const noexcept
{
    return _meshes;
}

const std::vector<GLTFLoader::Buffer>& GLTFLoader::getBuffers() const noexcept
{
    return _buffers;
}

const std::vector<GLTFLoader::BufferView>& GLTFLoader::getBufferViews() const noexcept
{
    return _bufferViews;
}

const std::vector<GLTFLoader::Accessor>& GLTFLoader::getAccessors() const noexcept
{
    return _accessors;
}
