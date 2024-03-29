//
// Project: GraphicsUtils2
// File: GLTFLoader.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once

#include "MathTypes.hpp"
#include "Typedef.hpp"


namespace gu2 {


class GLTFLoader {
public:
    struct Scene {
        std::vector<int64_t>   nodes;
    };

    struct Node {
        Mat4d                   matrix      {Mat4d::Identity()};
        int64_t                 mesh        {-1};
        std::vector<int64_t>    children;
    };

    struct Mesh {
        struct Primitive {
            enum class Mode : uint8_t {
                POINTS          = 0,
                LINES           = 1,
                LINE_LOOP       = 2,
                LINE_STRIP      = 3,
                TRIANGLES       = 4,
                TRIANGLE_STRIP  = 5,
                TRIANGLE_FAN    = 6
            };

            struct Attribute {
                std::string name;
                int64_t     accessorId;
            };

            int64_t                 id          {-1}; // id in the flattened primitive vector
            Mode                    mode        {Mode::TRIANGLES};
            int64_t                 indices     {-1};
            std::vector<Attribute>  attributes;
            int64_t                 material    {-1};
        };

        std::vector<Primitive>  primitives;
    };

    struct Buffer {
        std::string uri;
        Path        filename;
        size_t      byteLength  {0};

        char*       buffer      {nullptr}; // pointer to a memory mapped file
        size_t      bufferSize  {0}; // size of the memory mapped file (typically the same as byteLength)

        ~Buffer();
    };

    struct BufferView {
        int64_t     buffer      {-1};
        uint64_t    byteLength  {0};
        uint64_t    byteOffset  {0};
        uint64_t    byteStride  {0};
    };

    struct Accessor {
        enum class ComponentType : int64_t {
            BYTE            = 5120,
            UNSIGNED_BYTE   = 5121,
            SHORT           = 5122,
            UNSIGNED_SHORT  = 5123,
            UNSIGNED_INT    = 5125,
            FLOAT           = 5126
        };

        int64_t         bufferView      {-1};
        uint64_t        byteOffset      {0};
        ComponentType   componentType   {ComponentType::BYTE};
        uint64_t        count           {0};
        std::string     type;
    };

    struct Material {
        struct {
            struct {
                int64_t index       {-1};
                int64_t texCoord    {0};
            }   baseColorTexture;

            struct {
                int64_t index       {-1};
                int64_t texCoord    {0};
            }   metallicRoughnessTexture;

            Vec4d   baseColorFactor {1.0, 1.0, 1.0, 1.0};
            float   metallicFactor  {1.0};
            float   roughnessFactor {1.0};
        }   pbrMetallicRoughness;

        struct {
            int64_t index       {-1};
            int64_t texCoord    {0};
            double  scale       {1.0};
        }   normalTexture;
    };

    struct Texture {
        int64_t source  {-1};
    };

    struct Image {
        std::string uri;
        Path        filename;
    };

    void readFromFile(const Path& filename);

    const std::vector<Scene>& getScenes() const noexcept;
    const std::vector<Node>& getNodes() const noexcept;
    const std::vector<Mesh>& getMeshes() const noexcept;
    const std::vector<Buffer>& getBuffers() const noexcept;
    const std::vector<BufferView>& getBufferViews() const noexcept;
    const std::vector<Accessor>& getAccessors() const noexcept;
    const std::vector<Material>& getMaterials() const noexcept;
    const std::vector<Texture>& getTextures() const noexcept;
    const std::vector<Image>& getImages() const noexcept;

private:
    Json                    _gltfJson;
    std::vector<Scene>      _scenes;
    std::vector<Node>       _nodes;
    std::vector<Mesh>       _meshes;
    std::vector<Buffer>     _buffers;
    std::vector<BufferView> _bufferViews;
    std::vector<Accessor>   _accessors;
    std::vector<Material>   _materials;
    std::vector<Texture>    _textures;
    std::vector<Image>      _images;
};


} // namespace gu2
