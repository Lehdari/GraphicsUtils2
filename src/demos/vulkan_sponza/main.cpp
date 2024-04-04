//
// Project: GraphicsUtils2
// File: main.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include <gu2_os/App.hpp>
#include <gu2_os/Window.hpp>
#include <gu2_util/GLTFLoader.hpp>
#include <gu2_util/MathTypes.hpp>
#include <gu2_util/Image.hpp>
#include <gu2_util/Typedef.hpp>
#include <gu2_vulkan/backend.hpp>
#include <gu2_vulkan/DescriptorManager.hpp>
#include <gu2_vulkan/Material.hpp>
#include <gu2_vulkan/Mesh.hpp>
#include <gu2_vulkan/Pipeline.hpp>
#include <gu2_vulkan/PipelineManager.hpp>
#include <gu2_vulkan/Renderer.hpp>
#include <gu2_vulkan/Texture.hpp>
#include <gu2_vulkan/QueryWrapper.hpp>
#include <gu2_vulkan/Scene.hpp>
#include <gu2_vulkan/Shader.hpp>
#include <gu2_vulkan/Util.hpp>
#include <gu2_vulkan/VulkanSettings.hpp>

#include <vulkan/vulkan.h>

#include <shaderc/shaderc.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <limits>
#include <optional>
#include <set>
#include <iostream>
#include <memory>


bool checkValidationLayerSupport(const std::vector<const char*>& validationLayers)
{
    std::vector<VkLayerProperties> availableLayers = gu2::vkEnumerateInstanceLayerProperties();

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) != 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {

    printf("Validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator
) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


int64_t findShaderWithInputs(
    std::vector<gu2::Shader>* shaders,
    const std::vector<std::string>& requiredInputs,
    const std::vector<std::string>& requiredDescriptorBindings
) {
    for (int64_t i=0; i<shaders->size(); ++i) {
        const auto& shader = shaders->at(i);
        const auto& shaderInputVariables = shader.getInputVariables();
        if (requiredInputs.size() != shaderInputVariables.size())
            continue;
        const auto& shaderDescriptorBindings = shader.getDescriptorBindings();
        if (requiredDescriptorBindings.size() != shaderDescriptorBindings.size())
            continue;

        int requiredInputsFound = 0;
        for (const auto& requiredInput : requiredInputs) {
            for (const auto& shaderInputVariable : shaderInputVariables) {
                if (shaderInputVariable->name == requiredInput) {
                    ++requiredInputsFound;
                    break;
                }
            }
        }

        int requiredDescriptorBindingsFound = 0;
        for (const auto& requiredDescriptorBinding : requiredDescriptorBindings) {
            for (const auto& shaderDescriptorBinding : shaderDescriptorBindings) {
                if (shaderDescriptorBinding->name == requiredDescriptorBinding) {
                    ++requiredDescriptorBindingsFound;
                    break;
                }
            }
        }

        if (requiredInputsFound == shaderInputVariables.size() &&
            requiredDescriptorBindingsFound == shaderDescriptorBindings.size())
            return i;
    }

    return -1;
}

struct MaterialBuildInfo {
    int64_t                                 vertexShaderId              {-1};
    int64_t                                 fragmentShaderId            {-1};
    int64_t                                 baseColorTextureId          {-1};
    int64_t                                 metallicRoughnessTextureId  {-1};
    int64_t                                 normalTextureId             {-1};
    VkPipelineVertexInputStateCreateInfo    vertexInputInfo             {};
};
bool operator==(const MaterialBuildInfo& info1, const MaterialBuildInfo& info2) {
    return info1.vertexShaderId == info2.vertexShaderId &&
        info1.fragmentShaderId == info2.fragmentShaderId &&
        info1.baseColorTextureId == info2.baseColorTextureId &&
        info1.metallicRoughnessTextureId == info2.metallicRoughnessTextureId &&
        info1.normalTextureId == info2.normalTextureId;
}

void createFromGLTF(
    const gu2::GLTFLoader& gltfLoader,
    std::vector<gu2::Mesh>* meshes,
    std::vector<gu2::Material>* materials,
    std::vector<gu2::Shader>* shaders,
    std::vector<gu2::Texture>* textures,
    const gu2::VulkanSettings& vulkanSettings,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue,
    gu2::PipelineManager* pipelineManager,
    gu2::DescriptorManager* descriptorManager
) {
    auto& gltfTextures = gltfLoader.getTextures();
    auto nTextures = gltfTextures.size();
    textures->clear();
    for (const auto& t : gltfTextures)
        textures->emplace_back(physicalDevice, device);

    auto& gltfImages = gltfLoader.getImages();

    #pragma omp parallel for
    for (decltype(nTextures) i=0; i<nTextures; ++i) {
        const auto& t = gltfTextures[i];
        if (t.source < 0)
            throw std::runtime_error("Texture source not defined");

        const auto& imageFilename = gltfImages.at(t.source).filename;

        auto& texture = textures->at(i);
        texture.loadFromFile(commandPool, queue, imageFilename);

        printf("Added texture %u / %lu from %s\n", i, nTextures, GU2_PATH_TO_STRING(imageFilename));
        fflush(stdout);
    }

    auto& gltfMeshes = gltfLoader.getMeshes();
    auto& gltfMaterials = gltfLoader.getMaterials();

    // Count the number of primitives (one gu2::Mesh corresponds to a single GLTF mesh primitive, not mesh)
    size_t nPrimitives = 0;
    for (const auto& m : gltfMeshes)
        nPrimitives += m.primitives.size();

    std::vector<MaterialBuildInfo> materialBuildInfos;

    // Build meshes, shaders and materials
    meshes->clear();
    meshes->reserve(nPrimitives);
    std::vector<int64_t> meshMaterialIds;
    meshMaterialIds.reserve(nPrimitives);
    for (const auto& m : gltfMeshes) {
        for (const auto& p : m.primitives) {
            meshes->emplace_back(physicalDevice, device);
            auto& mesh = meshes->back();

            if (p.indices < 0) {
                fprintf(stderr, "WARNING: Unindexed meshes not currently supported, skipping...\n");
                continue;
            }

            printf("==============\n");
            // List required vertex attributes
            std::vector<std::string> requiredVertexAttributes, requiredFragmentInputs;
            for (const auto& attribute : p.attributes) {
                if (attribute.name == "POSITION") {
                    requiredVertexAttributes.emplace_back("inPosition");
                }
                else if (attribute.name == "NORMAL") {
                    requiredVertexAttributes.emplace_back("inNormal");
                    requiredFragmentInputs.emplace_back("fragNormal");
                }
                else if (attribute.name == "TANGENT") {
                    requiredVertexAttributes.emplace_back("inTangent");
                    requiredFragmentInputs.emplace_back("fragTangent");
                }
                else if (attribute.name.substr(0, 9) == "TEXCOORD_") {
                    requiredVertexAttributes.emplace_back("inTexCoord" + attribute.name.substr(9, 1));
                    requiredFragmentInputs.emplace_back("fragTexCoord" + attribute.name.substr(9, 1));
                }
            }

            // Try to find a vertex shader that matches the required input vertex attributes
            MaterialBuildInfo materialBuildInfo;
            materialBuildInfo.vertexShaderId = findShaderWithInputs(shaders, requiredVertexAttributes, {"ubo"});
            if (materialBuildInfo.vertexShaderId < 0) { // no suitable shader found, make a new one
                materialBuildInfo.vertexShaderId = shaders->size();
                shaders->emplace_back(device);
                auto& shader = shaders->back();
                if (requiredVertexAttributes.size() < 5)
                    shader.addMacroDefinition("DISABLE_IN_TEX_COORD_1", "true");
                shader.loadFromFile(gu2::Path(GU2_SHADER_DIR) / "vertex/pbr.glsl");
            }

            printf("requiredVertexAttributes.size(): %lu vertexShaderId: %ld\n",
                requiredVertexAttributes.size(), materialBuildInfo.vertexShaderId);

            // List required descriptor bindings
            auto& gltfMaterial = gltfMaterials.at(p.material);
            std::vector<std::string> requiredDescriptorBindings;

            if (gltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                requiredDescriptorBindings.emplace_back("baseColorTexture");
                materialBuildInfo.baseColorTextureId = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
            }
            if (gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
                requiredDescriptorBindings.emplace_back("metallicRoughnessTexture");
                materialBuildInfo.metallicRoughnessTextureId = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
            }
            if (gltfMaterial.normalTexture.index >= 0) {
                requiredDescriptorBindings.emplace_back("normalTexture");
                materialBuildInfo.normalTextureId = gltfMaterial.normalTexture.index;
            }

            // Try to find a fragment shader that matches the required input vertex attributes
            materialBuildInfo.fragmentShaderId = findShaderWithInputs(shaders, requiredFragmentInputs,
                requiredDescriptorBindings);
            if (materialBuildInfo.fragmentShaderId < 0) { // no suitable shader found, make a new one
                materialBuildInfo.fragmentShaderId = shaders->size();
                shaders->emplace_back(device);
                auto& shader = shaders->back();

                if (requiredVertexAttributes.size() < 5)
                    shader.addMacroDefinition("DISABLE_IN_TEX_COORD_1", "true");
                if (std::find(requiredDescriptorBindings.begin(), requiredDescriptorBindings.end(),
                    "baseColorTexture") == requiredDescriptorBindings.end())
                    shader.addMacroDefinition("DISABLE_BASE_COLOR", "true");
                if (std::find(requiredDescriptorBindings.begin(), requiredDescriptorBindings.end(),
                    "metallicRoughnessTexture") == requiredDescriptorBindings.end())
                    shader.addMacroDefinition("DISABLE_METALLIC_ROUGHNESS", "true");
                if (std::find(requiredDescriptorBindings.begin(), requiredDescriptorBindings.end(),
                    "normalTexture") == requiredDescriptorBindings.end())
                    shader.addMacroDefinition("DISABLE_USE_NORMAL_TEXTURE", "true");

                shader.loadFromFile(gu2::Path(GU2_SHADER_DIR) / "fragment/pbr.glsl");
            }

            printf("requiredDescriptorBindings.size(): %lu fragmentShaderId: %ld\n",
                requiredDescriptorBindings.size(), materialBuildInfo.fragmentShaderId);

            // Add vertex attribute data
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

                if (attribute.name == "POSITION") {
                    if (accessor.componentType == gu2::GLTFLoader::Accessor::ComponentType::FLOAT &&
                        accessor.type == "VEC3") {
                        printf("Adding POSITION, count: %lu stride: %lu\n", accessor.count, bufferView.byteStride);
                        mesh.addVertexAttribute(shaders->at(materialBuildInfo.vertexShaderId)
                            .getInputVariableLayoutLocation("inPosition"),
                            reinterpret_cast<gu2::Vec3f*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                    }
                    else
                        throw std::runtime_error("Unsupported attribute format for \"POSITION\"");
                }
                else if (attribute.name == "NORMAL") {
                    if (accessor.componentType == gu2::GLTFLoader::Accessor::ComponentType::FLOAT &&
                        accessor.type == "VEC3") {
                        printf("Adding NORMAL, count: %lu stride: %lu\n", accessor.count, bufferView.byteStride);
                        mesh.addVertexAttribute(shaders->at(materialBuildInfo.vertexShaderId)
                            .getInputVariableLayoutLocation("inNormal"),
                            reinterpret_cast<gu2::Vec3f*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                    }
                    else
                        throw std::runtime_error("Unsupported attribute format for \"NORMAL\"");
                }
                else if (attribute.name == "TANGENT") {
                    if (accessor.componentType == gu2::GLTFLoader::Accessor::ComponentType::FLOAT &&
                        accessor.type == "VEC4") {
                        printf("Adding TANGENT, count: %lu stride: %lu\n", accessor.count, bufferView.byteStride);
                        mesh.addVertexAttribute(shaders->at(materialBuildInfo.vertexShaderId)
                            .getInputVariableLayoutLocation("inTangent"),
                            reinterpret_cast<gu2::Vec4f*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                    }
                    else
                        throw std::runtime_error("Unsupported attribute format for \"TANGENT\"");
                }
                else if (attribute.name.substr(0, 9) == "TEXCOORD_") {
                    int texCoordId = std::stoi(attribute.name.substr(9, 1));
                    if (accessor.componentType == gu2::GLTFLoader::Accessor::ComponentType::FLOAT &&
                        accessor.type == "VEC2") {
                        printf("Adding %s, count: %lu stride: %lu\n", attribute.name.c_str(), accessor.count,
                            bufferView.byteStride);
                        mesh.addVertexAttribute(shaders->at(materialBuildInfo.vertexShaderId)
                            .getInputVariableLayoutLocation("inTexCoord" + attribute.name.substr(9, 1)),
                            reinterpret_cast<gu2::Vec2f*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
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
                    case gu2::GLTFLoader::Accessor::ComponentType::UNSIGNED_SHORT:
                        mesh.setIndices(
                            reinterpret_cast<uint16_t*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                        break;
                    case gu2::GLTFLoader::Accessor::ComponentType::UNSIGNED_INT:
                        mesh.setIndices(
                            reinterpret_cast<uint32_t*>(buffer.buffer + bufferView.byteOffset + accessor.byteOffset),
                            accessor.count, bufferView.byteStride);
                        break;
                    default:
                        throw std::runtime_error("Invalid accessor for indices, component type not \"UNSIGNED_SHORT\" or \"UNSIGNED_INT\"");
                }
            }

            mesh.upload(commandPool, queue);

            // Try to find pre-existing material with identical info, create new info if one is not found
            int64_t meshMaterialId = -1;
            for (int64_t i=0; i<materialBuildInfos.size(); ++i) {
                if (materialBuildInfos[i] == materialBuildInfo)
                    meshMaterialId = i;
            }
            if (meshMaterialId < 0) {
                meshMaterialId = (int64_t)materialBuildInfos.size();
                materialBuildInfo.vertexInputInfo = mesh.getVertexAttributesDescription()
                    .getPipelineVertexInputStateCreateInfo(); // TODO doesn't feel very elegant of a solution
                materialBuildInfos.push_back(std::move(materialBuildInfo));
            }
            meshMaterialIds.push_back(meshMaterialId);
        }
    }

    assert(meshMaterialIds.size() == meshes->size());

    materials->clear();
    materials->reserve(materialBuildInfos.size());
    for (const auto& materialBuildInfo : materialBuildInfos) {
        materials->emplace_back(device);
        auto& material = materials->back();

        // Add shaders
        material.setVertexShader(shaders->at(materialBuildInfo.vertexShaderId));
        material.setFragmentShader(shaders->at(materialBuildInfo.fragmentShaderId));

        // Build descriptor set layouts
        material.createDescriptorSetLayouts(descriptorManager);

        // Build material pipeline
        material.createPipeline(pipelineManager, materialBuildInfo.vertexInputInfo);

        // Add textures
        if (materialBuildInfo.baseColorTextureId >= 0)
            material.addUniform(2, 0, textures->at(materialBuildInfo.baseColorTextureId));
        if (materialBuildInfo.metallicRoughnessTextureId >= 0)
            material.addUniform(2, 1, textures->at(materialBuildInfo.metallicRoughnessTextureId));
        if (materialBuildInfo.normalTextureId >= 0)
            material.addUniform(2, 2, textures->at(materialBuildInfo.normalTextureId));

        material.createDescriptorSets(descriptorManager, vulkanSettings.framesInFlight);
    }

    for (size_t i=0; i<meshes->size(); ++i) {
        auto& mesh = meshes->at(i);
        mesh.setMaterial(&materials->at(meshMaterialIds[i]));
        mesh.createDescriptorSets(descriptorManager, vulkanSettings.framesInFlight);
    }
}


class VulkanWindow : public gu2::Window<VulkanWindow> {
public:
    VulkanWindow(const gu2::WindowSettings& windowSettings, const gu2::VulkanSettings& vulkanSettings) :
        Window<VulkanWindow>    (windowSettings),
        _vulkanSettings         (vulkanSettings),
        _vulkanPhysicalDevice   (VK_NULL_HANDLE)
    {
        initVulkan();
    }

    ~VulkanWindow()
    {
        // Wait for the Vulkan device to finish its tasks
        vkDeviceWaitIdle(_vulkanDevice);
        _meshes.clear();
        gu2::Mesh::destroyUniformBuffers(_vulkanDevice);
        _materials.clear();
        _shaders.clear();
        _textures.clear();
        _pipelineManager.reset();
        _descriptorManager.reset();
        _renderer.reset();
        vkDestroyDevice(_vulkanDevice, nullptr);
        if (_vulkanSettings.enableValidationLayers)
            DestroyDebugUtilsMessengerEXT(_vulkanInstance, _vulkanDebugMessenger, nullptr);
        vkDestroySurfaceKHR(_vulkanInstance, _vulkanSurface, nullptr);
        vkDestroyInstance(_vulkanInstance, nullptr);
    }

    void initVulkan()
    {
        if (_vulkanSettings.enableValidationLayers && !checkValidationLayerSupport(_vulkanSettings.validationLayers)) {
            throw std::runtime_error("Requested Vulkan validation layers not supported!");
        }
        createInstance();
        setupDebugMessenger();
        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();
        gu2::RendererSettings rendererSettings {
            &_vulkanSettings,
            _vulkanPhysicalDevice,
            _vulkanDevice,
            _vulkanSurface,
            _vulkanGraphicsQueue,
            &_window
        };
        _renderer = std::make_unique<gu2::Renderer>(rendererSettings);
        _renderer->createRenderPass();
        _renderer->createFramebuffers();
        _renderer->createSyncObjects();

        _pipelineManager = std::make_unique<gu2::PipelineManager>();
        gu2::PipelineSettings defaultPipelineSettings;
        defaultPipelineSettings.device = _vulkanDevice;
        defaultPipelineSettings.renderPass = _renderer->getRenderPass();
        defaultPipelineSettings.swapChainExtent = _renderer->getSwapChainExtent();
        _pipelineManager->setDefaultPipelineSettings(defaultPipelineSettings);

        _descriptorManager = std::make_unique<gu2::DescriptorManager>(_vulkanDevice);

        // Load sponza (TODO move elsewhere)
        gu2::GLTFLoader sponzaLoader;
        sponzaLoader.readFromFile(gu2::Path(ASSETS_DIR) / "sponza/Main.1_Sponza/NewSponza_Main_glTF_002.gltf");
        gu2::Mesh::createUniformBuffers(_vulkanPhysicalDevice, _vulkanDevice, _vulkanSettings.framesInFlight, 512); // TODO number of uniforms
        createFromGLTF(sponzaLoader, &_meshes, &_materials, &_shaders, &_textures, _vulkanSettings,
            _vulkanPhysicalDevice, _vulkanDevice, _renderer->getCommandPool(), _vulkanGraphicsQueue,
            _pipelineManager.get(), _descriptorManager.get());

        _scene.createFromGLFT(sponzaLoader, _meshes);
    }

    void createInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "GraphicsUtils2 Vulkan Triangle Demo";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "GraphicsUtils2";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Setup instance extensions
        auto extensions = gu2::getVulkanInstanceExtensions();
        #ifdef VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        #endif
        if (_vulkanSettings.enableValidationLayers) // enable debug utils if validation layers are requested
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // Setup validation layers in case they are requested
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (_vulkanSettings.enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(_vulkanSettings.validationLayers.size());
            createInfo.ppEnabledLayerNames = _vulkanSettings.validationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        auto createStatus = vkCreateInstance(&createInfo, nullptr, &_vulkanInstance);
        if (createStatus != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance!");
        }
    }

    void setupDebugMessenger()
    {
        if (!_vulkanSettings.enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(_vulkanInstance, &createInfo, nullptr, &_vulkanDebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    void createSurface()
    {
        if (!gu2::createWindowVulkanSurface(_window, _vulkanInstance, nullptr, &_vulkanSurface)) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void selectPhysicalDevice()
    {
        auto devices = gu2::vkEnumeratePhysicalDevices(_vulkanInstance);
        if (devices.empty())
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");

        // Select the most suitable device
        uint32_t maxSuitability = 0;
        for (const auto& device : devices) {
            uint32_t suitability = deviceSuitability(device);
            if (suitability > maxSuitability) {
                _vulkanPhysicalDevice = device;
                maxSuitability = suitability;
            }
        }

        if (_vulkanPhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }

        // Store the device properties in local struct
        vkGetPhysicalDeviceProperties(_vulkanPhysicalDevice, &_vulkanPhysicalDeviceProperties);
    }

    uint32_t deviceSuitability(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // Rank the device according to its properties and features
        uint32_t suitability = 1; // minimal suitability
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            suitability += 1000;

        auto familyIndices = gu2::findQueueFamilies(device, _vulkanSurface);
        if (!familyIndices.isComplete())
            return 0; // not suitable

        bool extensionsSupported = checkDeviceExtensionSupport(device);
        if (!extensionsSupported)
            return 0; // not suitable

        auto swapChainSupport =  gu2::querySwapChainSupport(device, _vulkanSurface);
        bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        if (!swapChainAdequate)
            return 0; // not suitable

        if (!deviceFeatures.samplerAnisotropy)
            return 0; // not suitable

        return suitability;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        auto availableExtensions = gu2::vkEnumerateDeviceExtensionProperties(device, nullptr);

        std::set<std::string> requiredExtensions(_vulkanSettings.deviceExtensions.begin(),
            _vulkanSettings.deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty(); // all required extensions found
    }

    void createLogicalDevice()
    {
        auto familyIndices = gu2::findQueueFamilies(_vulkanPhysicalDevice, _vulkanSurface);

        // Graphics queue creation info
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {
            familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // Device features to be used
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        // Logical device creation info
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(_vulkanSettings.deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = _vulkanSettings.deviceExtensions.data();
        if (_vulkanSettings.enableValidationLayers) { // ignored in modern vulkan implementations
            createInfo.enabledLayerCount = static_cast<uint32_t>(_vulkanSettings.validationLayers.size());
            createInfo.ppEnabledLayerNames = _vulkanSettings.validationLayers.data();
        }
        else
            createInfo.enabledLayerCount = 0;

        if (vkCreateDevice(_vulkanPhysicalDevice, &createInfo, nullptr, &_vulkanDevice) != VK_SUCCESS)
            throw std::runtime_error("failed to create logical device!");

        vkGetDeviceQueue(_vulkanDevice, familyIndices.graphicsFamily.value(), 0, &_vulkanGraphicsQueue);
        vkGetDeviceQueue(_vulkanDevice, familyIndices.presentFamily.value(), 0, &_vulkanPresentQueue);
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        _renderer->recordRenderPassCommands(commandBuffer, imageIndex);

        for (size_t nodeId=0; nodeId<_scene.nodes.size(); ++nodeId) {
            const auto& node = _scene.nodes[nodeId];
            node.mesh->bind(commandBuffer);
            node.mesh->draw(commandBuffer, _renderer->getCurrentFrame(), nodeId);
        }

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void handleEvent(const gu2::Event& event)
    {
        switch (event.type) {
            case gu2::Event::WINDOW:
                switch (event.window.action) {
                    case gu2::WindowEventAction::CLOSE: close(); return;
                    case gu2::WindowEventAction::RESIZE:
                        _renderer->framebufferResized();
                        printf("Resize to %d x %d\n", event.window.data1, event.window.data2); fflush(stdout);
                        return;
                    default: break;
                }
                break;
            case gu2::Event::KEY:
                switch (event.key.sym.scancode) {
                    case gu2::ScanCode::ESCAPE: close(); return;
                    default: break;
                }
                break;
            default: break;
        }
    }

    void render()
    {
        VkCommandBuffer commandBuffer;
        uint32_t imageIndex;
        if (!_renderer->beginRender(&commandBuffer, &imageIndex))
            return; // swap chain got resized
        recordCommandBuffer(commandBuffer, imageIndex);
        gu2::Mesh::updateUniformBuffer(_scene, *_renderer);
        _renderer->endRender(_vulkanPresentQueue, imageIndex);
    }

private:
    const gu2::VulkanSettings               _vulkanSettings;

    VkInstance                              _vulkanInstance;
    VkDebugUtilsMessengerEXT                _vulkanDebugMessenger;
    VkSurfaceKHR                            _vulkanSurface;
    VkPhysicalDevice                        _vulkanPhysicalDevice;
    VkPhysicalDeviceProperties              _vulkanPhysicalDeviceProperties;
    VkDevice                                _vulkanDevice;
    VkQueue                                 _vulkanGraphicsQueue;
    VkQueue                                 _vulkanPresentQueue;

    std::unique_ptr<gu2::Renderer>          _renderer;
    std::vector<gu2::Texture>               _textures;
    std::vector<gu2::Shader>                _shaders;
    std::vector<gu2::Material>              _materials;
    std::vector<gu2::Mesh>                  _meshes;
    std::unique_ptr<gu2::DescriptorManager> _descriptorManager;
    std::unique_ptr<gu2::PipelineManager>   _pipelineManager;
    gu2::Scene                              _scene;
};


int main(void)
{
    gu2::WindowSettings windowSettings;
    windowSettings.name = "Hello Vulkan Sponza!";
    windowSettings.w = 800;
    windowSettings.h = 600;

    gu2::VulkanSettings vulkanSettings;
    #ifdef NDEBUG
    vulkanSettings.enableValidationLayers = false;
    #else
    vulkanSettings.enableValidationLayers = true;
    #endif

    // Main loop
    try {
        VulkanWindow window(windowSettings, vulkanSettings);
        gu2::App::addWindow(&window);

        while (gu2::App::update());
    }
    catch (const std::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        return EXIT_FAILURE;
    }

    gu2::cleanupBackend();

    return EXIT_SUCCESS;
}