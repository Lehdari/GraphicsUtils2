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


using namespace gu2;


Shader::Shader(VkDevice device) :
    _device         (device),
    _shaderModule   (nullptr)
{
}

Shader::Shader(Shader&& other) :
    _device             (other._device),
    _macroDefinitions   (std::move(other._macroDefinitions)),
    _filename           (std::move(other._filename)),
    _spirv              (std::move(other._spirv)),
    _shaderModule       (other._shaderModule)
{
    other._shaderModule = nullptr;

    if (!_spirv.empty())
        parseSpirvReflection();
}

Shader& Shader::operator=(Shader&& other)
{
    if (this == &other)
        return *this;

    _device = other._device;
    _macroDefinitions = std::move(other._macroDefinitions);
    _filename = std::move(other._filename);
    _spirv = std::move(other._spirv);
    _shaderModule = other._shaderModule;

    other._shaderModule = nullptr;

    if (!_spirv.empty())
        parseSpirvReflection();

    return *this;
}

Shader::~Shader()
{
    if (_shaderModule != nullptr)
        vkDestroyShaderModule(_device, _shaderModule, nullptr);

    spvReflectDestroyShaderModule(&_reflectionModule);
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
    parseSpirvReflection();
    _shaderModule = createShaderModule(_device, _spirv);
}

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

int64_t Shader::getInputVariableLayoutLocation(const std::string& inputVariableName) const noexcept
{
    for (const auto& inputVariable : _inputVariables) {
        if (inputVariable->name == inputVariableName)
            return inputVariable->location;
    }
    return -1;
}

VkShaderModule Shader::getShaderModule() const noexcept
{
    return _shaderModule;
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

void Shader::parseSpirvReflection()
{
    // Generate reflection data for a shader
    SpvReflectResult result = spvReflectCreateShaderModule(_spirv.size() * sizeof(uint32_t), _spirv.data(),
        &_reflectionModule);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // Enumerate and extract shader's input variables
    uint32_t nVars = 0;
    result = spvReflectEnumerateInputVariables(&_reflectionModule, &nVars, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    _inputVariables.resize(nVars);
    result = spvReflectEnumerateInputVariables(&_reflectionModule, &nVars, _inputVariables.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    // Enumerate and extract shader's descriptor bindings
    uint32_t nBindings = 0;
    result = spvReflectEnumerateDescriptorBindings(&_reflectionModule, &nBindings, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    _descriptorBindings.resize(nBindings);
    result = spvReflectEnumerateDescriptorBindings(&_reflectionModule, &nBindings, _descriptorBindings.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
}
