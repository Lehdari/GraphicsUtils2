//
// Project: GraphicsUtils2
// File: Shader.hpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#pragma once


#include "gu2_util/Typedef.hpp"

#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.h>


namespace gu2 {


using ShaderType = shaderc_shader_kind;
using SpirvByteCode = std::vector<uint32_t>;


class Shader {
public:
    Shader(VkDevice device = nullptr);
    Shader(const Shader&) = delete;
    Shader(Shader&&) = default;
    Shader& operator=(const Shader&) = delete;
    Shader& operator=(Shader&&) = default;
    ~Shader();
    
    void loadFromFile(
        const Path& filename,
        ShaderType type = shaderc_glsl_infer_from_source,
        bool optimize = false);

    const SpirvByteCode& getSpirvByteCode() const noexcept;
    VkShaderModule getShaderModule() const noexcept;

    static VkShaderModule createShaderModule(VkDevice device, const SpirvByteCode& code);

private:
    VkDevice        _device;
    Path            _filename;
    SpirvByteCode   _spirv; // SPIR-V bytecode
    VkShaderModule  _shaderModule;
};


} // namespace gu2
