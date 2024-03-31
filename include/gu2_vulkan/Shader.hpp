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


#include "Descriptor.hpp"
#include "gu2_util/Typedef.hpp"

#include <shaderc/shaderc.hpp>
#include <spirv_reflect.h>
#include <vulkan/vulkan.h>


namespace gu2 {


using ShaderType = shaderc_shader_kind;
using SpirvByteCode = std::vector<uint32_t>;


class Shader {
public:
    Shader(VkDevice device = nullptr);
    Shader(const Shader&) = delete;
    Shader(Shader&&);
    Shader& operator=(const Shader&) = delete;
    Shader& operator=(Shader&&);
    ~Shader();

    void addMacroDefinition(const std::string& name, const std::string& value);

    void loadFromFile(
        const Path& filename,
        ShaderType type = shaderc_glsl_infer_from_source,
        bool optimize = false);

    inline const SpirvByteCode& getSpirvByteCode() const noexcept;
    inline const std::vector<SpvReflectInterfaceVariable*>& getInputVariables() const noexcept;
    inline const std::vector<SpvReflectDescriptorBinding*>& getDescriptorBindings() const noexcept;
    inline const std::vector<DescriptorSetLayoutInfo>& getDescriptorSetLayouts() const noexcept;

    int64_t getInputVariableLayoutLocation(const std::string& inputVariableName) const noexcept;

    VkShaderModule getShaderModule() const noexcept;

    static VkShaderModule createShaderModule(VkDevice device, const SpirvByteCode& code);

private:
    using MacroDefinitionList = std::vector<std::pair<std::string, std::string>>; // TODO tidy up (use struct instead of pair)

    VkDevice            _device;
    MacroDefinitionList _macroDefinitions;
    Path                _filename;
    SpirvByteCode       _spirv; // SPIR-V bytecode
    VkShaderModule      _shaderModule;

    // SPIR-V reflection data
    SpvReflectShaderModule                      _reflectionModule;
    std::vector<SpvReflectInterfaceVariable*>   _inputVariables;
    std::vector<SpvReflectDescriptorBinding*>   _descriptorBindings;
    std::vector<SpvReflectDescriptorSet*>       _descriptorSets;
    std::vector<DescriptorSetLayoutInfo>        _descriptorSetLayouts;

    void parseSpirvReflection();
};


const SpirvByteCode& Shader::getSpirvByteCode() const noexcept
{
    return _spirv;
}

const std::vector<SpvReflectInterfaceVariable*>& Shader::getInputVariables() const noexcept
{
    return _inputVariables;
}

const std::vector<SpvReflectDescriptorBinding*>& Shader::getDescriptorBindings() const noexcept
{
    return _descriptorBindings;
}

const std::vector<DescriptorSetLayoutInfo>& Shader::getDescriptorSetLayouts() const noexcept
{
    return _descriptorSetLayouts;
}


} // namespace gu2
