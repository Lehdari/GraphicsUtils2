//
// Project: GraphicsUtils2
// File: Shader.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Shader.hpp"
#include "gu2_util/FileUtils.hpp"

#include "spirv_reflect.h"


using namespace gu2;


Shader::Shader(VkDevice device) :
    _device         (device),
    _shaderModule   (nullptr)
{
}

Shader::~Shader()
{
    if (_shaderModule != nullptr)
        vkDestroyShaderModule(_device, _shaderModule, nullptr);
}

void Shader::addMacroDefinition(const std::string& name, const std::string& value)
{
    _macroDefinitions.emplace_back(name, value);
}

void Shader::loadFromFile(const Path& filename, ShaderType type, bool optimize)
{
    auto source = readFile(filename);

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    for (const auto& macro : _macroDefinitions) {
        options.AddMacroDefinition(macro.first, macro.second);
    }

    if (optimize) options.SetOptimizationLevel(shaderc_optimization_level_size);

    shaderc::CompilationResult result = compiler.CompileGlslToSpv(
        source.data(), source.size(), type, GU2_PATH_TO_STRING(filename), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::string error(result.GetErrorMessage());
        if (result.GetCompilationStatus() == shaderc_compilation_status_invalid_stage)
            error = "Unable to deduce the shader stage. Please use #pragma shader_stage in shader header or provide the correct type to loadFromFile";
        throw std::runtime_error(error);
    }

    _spirv = { result.cbegin(), result.cend() };
    _shaderModule = createShaderModule(_device, _spirv);
}

const SpirvByteCode& Shader::getSpirvByteCode() const noexcept
{
    return _spirv;
}

VkShaderModule Shader::getShaderModule() const noexcept
{
    return _shaderModule;
}

void Shader::reflectionTest()
{
    // Generate reflection data for a shader
    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(_spirv.size() * sizeof(uint32_t), _spirv.data(), &module);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // Enumerate and extract shader's input variables
    uint32_t nVars = 0;
    result = spvReflectEnumerateInputVariables(&module, &nVars, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectInterfaceVariable*> inputVars(nVars);
    result = spvReflectEnumerateInputVariables(&module, &nVars, inputVars.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    uint32_t nBindings = 0;
    result = spvReflectEnumerateDescriptorBindings(&module, &nBindings, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectDescriptorBinding*> descriptorBindings(nBindings);
    result = spvReflectEnumerateDescriptorBindings(&module, &nBindings, descriptorBindings.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // Output variables, descriptor bindings, descriptor sets, and push constants
    // can be enumerated and extracted using a similar mechanism.

    for (auto* inputVar : inputVars) {
        printf("Input var: %s\n", inputVar->name);
    }

    for (auto* descriptorBinding : descriptorBindings) {
        printf("Descriptor binding: %s\n", descriptorBinding->name);
    }

    // Destroy the reflection data when no longer required.
    spvReflectDestroyShaderModule(&module);
}

VkShaderModule Shader::createShaderModule(VkDevice device, const SpirvByteCode& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }

    return shaderModule;
}