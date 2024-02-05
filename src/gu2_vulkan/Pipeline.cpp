//
// Project: GraphicsUtils2
// File: Pipeline.cpp
//
// Copyright (c) 2024 Miika 'Lehdari' Lehtimäki
// You may use, distribute and modify this code under the terms
// of the licence specified in file LICENSE which is distributed
// with this source code package.
//

#include "Pipeline.hpp"
#include "Texture.hpp"
#include "Util.hpp"
#include "VulkanSettings.hpp"
#include "QueryWrapper.hpp"

#include <array>


using namespace gu2;


Pipeline::Pipeline(
    const VulkanSettings& vulkanSettings,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkSurfaceKHR surface,
    WindowObject* window
) :
    _vulkanSettings     (&vulkanSettings),
    _physicalDevice     (physicalDevice),
    _device             (device),
    _surface            (surface),
    _window             (window),
    _framebufferResized (false),
    _currentFrame       (0)
{
    // Store the device properties in local struct
    vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);

    createUniformBuffers();
    createDescriptorPool();
}

Pipeline::~Pipeline()
{
    cleanupSwapChain();
    for (int i=0; i<_vulkanSettings->framesInFlight; ++i) {
        vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(_device, _inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(_device, _commandPool, nullptr);
    for (int i=0; i<_vulkanSettings->framesInFlight; ++i) {
        vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
        vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);

    vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);
}

void Pipeline::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void Pipeline::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(_vulkanSettings->framesInFlight * _vulkanSettings->nBoxes);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(_vulkanSettings->framesInFlight);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(_vulkanSettings->framesInFlight);

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void Pipeline::createDescriptorSets(const Texture& texture)
{
    std::vector<VkDescriptorSetLayout> layouts(_vulkanSettings->framesInFlight, _descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(_vulkanSettings->framesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(_vulkanSettings->framesInFlight);
    if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < _vulkanSettings->framesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture.getImageView();
        imageInfo.sampler = texture.getSampler();

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = _descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(), 0, nullptr);
    }
}

void Pipeline::createImageViews() {
    _swapChainImageViews.resize(_swapChainImages.size());
    for (size_t i = 0; i<_swapChainImages.size(); i++) {
        _swapChainImageViews[i] = gu2::createImageView(_device, _swapChainImages[i],
            _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void Pipeline::createDepthResources(VkQueue graphicsQueue)
{
    auto depthFormat = findDepthFormat(_physicalDevice);
    gu2::createImage(_physicalDevice, _device,
        _swapChainExtent.width, _swapChainExtent.height, 1, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _depthImage, _depthImageMemory);
    _depthImageView = gu2::createImageView(_device, _depthImage, depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    gu2::transitionImageLayout(_device, _commandPool, graphicsQueue, _depthImage,
        depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void Pipeline::createSwapChain()
{
    VulkanSwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice, _surface);

    auto surfaceFormat = selectSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = selectSwapPresentMode(swapChainSupport.presentModes);
    auto extent = selectSwapExtent(_window, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = gu2::findQueueFamilies(_physicalDevice, _surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    _swapChainImages = gu2::vkGetSwapchainImagesKHR(_device, _swapChain);
    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;
}

void Pipeline::cleanupSwapChain()
{
    vkDestroyImageView(_device, _depthImageView, nullptr);
    vkDestroyImage(_device, _depthImage, nullptr);
    vkFreeMemory(_device, _depthImageMemory, nullptr);

    for (auto& framebuffer : _swapChainFramebuffers) {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }

    for (auto& imageView : _swapChainImageViews) {
        vkDestroyImageView(_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
}

void Pipeline::recreateSwapChain(VkQueue graphicsQueue)
{
    vkDeviceWaitIdle(_device);

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createDepthResources(graphicsQueue);
    createFramebuffers();

    _framebufferResized = false;
}

void Pipeline::createRenderPass()
{
    // Color attachment
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = gu2::findDepthFormat(_physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Add dependency to prevent image transitions before swap chain image is available
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // previous render pass (present)
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT; // wait for image copy to swapchain
    // image to be ready
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; // prevent any modifications to the color
    // attachment: this prevents layout transitions from happening before color attachment output stage

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void Pipeline::createGraphicsPipeline(
    VkShaderModule vertShaderModule,
    VkShaderModule fragShaderModule,
    VkPipelineVertexInputStateCreateInfo vertexInputInfo
) {
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Dynamic states
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_swapChainExtent.width;
    viewport.height = (float)_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Blend mode
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout!");

    // Enable depth testing
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER; // We're using inverse depth buffer (1.0f at near and 0.0f and far plane)
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }

    // Cleanup shader objects
    vkDestroyShaderModule(_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}

void Pipeline::createCommandPool()
{
    auto queueFamilyIndices = findQueueFamilies(_physicalDevice, _surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }
}

void Pipeline::createCommandBuffers()
{
    _commandBuffers.resize(_vulkanSettings->framesInFlight);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = _commandBuffers.size();

    if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void Pipeline::createFramebuffers()
{
    _swapChainFramebuffers.resize(_swapChainImageViews.size());

    for (size_t i = 0; i < _swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            _swapChainImageViews[i],
            _depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _swapChainExtent.width;
        framebufferInfo.height = _swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

void Pipeline::createSyncObjects()
{
    _imageAvailableSemaphores.resize(_vulkanSettings->framesInFlight);
    _renderFinishedSemaphores.resize(_vulkanSettings->framesInFlight);
    _inFlightFences.resize(_vulkanSettings->framesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i=0; i<_vulkanSettings->framesInFlight; ++i) {
        if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create synchronization structures!");
        }
    }
}

void Pipeline::recordRenderPassCommands(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapChainExtent;
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {0.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(_swapChainExtent.width);
    viewport.height = static_cast<float>(_swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

VkCommandPool Pipeline::getCommandPool()
{
    return _commandPool;
}

uint64_t Pipeline::getCurrentFrame() const
{
    return _currentFrame;
}

bool Pipeline::beginRender(VkQueue graphicsQueue, VkCommandBuffer* commandBuffer, uint32_t* imageIndex)
{
    vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    // Check for swap chain obsolescence
    auto nextImageStatus = vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX,
        _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, imageIndex);
    if (nextImageStatus == VK_ERROR_OUT_OF_DATE_KHR || nextImageStatus == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain(graphicsQueue);
        return false;
    }
    else if (nextImageStatus != VK_SUCCESS)
        throw std::runtime_error("Failed to acquire swap chain image!");

    vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);

    vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);

    *commandBuffer = _commandBuffers[_currentFrame];
    return true;
}

void Pipeline::endRender(VkQueue graphicsQueue, VkQueue presentQueue, uint32_t imageIndex)
{
    updateUniformBuffer(_swapChainExtent, _currentFrame); // TODO move

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]}; // is this needed?
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];
    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]}; // is this needed?
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    auto queuePresentStatus = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (queuePresentStatus == VK_ERROR_OUT_OF_DATE_KHR || queuePresentStatus == VK_SUBOPTIMAL_KHR ||
        _framebufferResized) {
        recreateSwapChain(graphicsQueue);
    }
    else if (queuePresentStatus != VK_SUCCESS)
        throw std::runtime_error("Failed to present swap chain image!");

    _currentFrame = (_currentFrame+1) % _vulkanSettings->framesInFlight;
}

void Pipeline::framebufferResized()
{
    _framebufferResized = true;
}

void Pipeline::createUniformBuffers()
{
    VkDeviceSize bufferSize = padUniformBufferSize(_physicalDeviceProperties, sizeof(UniformBufferObject)) *
        _vulkanSettings->nBoxes;

    _uniformBuffers.resize(_vulkanSettings->framesInFlight);
    _uniformBuffersMemory.resize(_vulkanSettings->framesInFlight);
    _uniformBuffersMapped.resize(_vulkanSettings->framesInFlight);

    for (size_t i = 0; i < _vulkanSettings->framesInFlight; i++) {
        createBuffer(_physicalDevice, _device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffers[i],
            _uniformBuffersMemory[i]);

        vkMapMemory(_device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
    }
}

void Pipeline::updateUniformBuffer(VkExtent2D swapChainExtent, uint32_t currentFrame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    double time = std::chrono::duration<double, std::chrono::seconds::period>(currentTime - startTime).count();

    for (int boxId=0; boxId<_vulkanSettings->nBoxes; ++boxId) {
        gu2::UniformBufferObject ubo;
        // Model matrix
        double boxPosAngle = boxId / (double)_vulkanSettings->nBoxes;
        gu2::Vec3f modelPos(std::cos(boxPosAngle*2.0*M_PI + 0.25*time), 0.0f, std::sin(boxPosAngle*2.0*M_PI + 0.25*time));
        modelPos *= 2.0f;
        ubo.model <<
                  (Eigen::AngleAxisf(0.25*M_PI*time, gu2::Vec3f::UnitX())
                      * Eigen::AngleAxisf(0.5*M_PI*time, gu2::Vec3f::UnitY())
                      * Eigen::AngleAxisf(0.33*M_PI*time, gu2::Vec3f::UnitZ())).toRotationMatrix(),
            modelPos, gu2::Vec3f::Zero().transpose(), 1.0f;

        // View matrix
        gu2::Vec3f target(0.0f, 0.0f, 0.0f);
        gu2::Vec3f source(1.0f, 3.0f, 5.0f);
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
        float aspectRatio = swapChainExtent.width / (float) swapChainExtent.height;
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

        memcpy(reinterpret_cast<uint8_t*>(_uniformBuffersMapped[currentFrame]) +
            padUniformBufferSize(_physicalDeviceProperties, sizeof(ubo))*boxId, &ubo, sizeof(ubo));
    }
}
