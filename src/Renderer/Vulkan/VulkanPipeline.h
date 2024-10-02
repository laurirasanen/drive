#pragma once

#include <memory>

#include "vulkan/vulkan_core.h"

#include "../../Log.h"
#include "VulkanAttributes.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"

namespace drive
{
template<class V>
class VulkanPipeline
{
  public:
    VulkanPipeline() = delete;
    VulkanPipeline(
        const VulkanDevice&                          device,
        std::shared_ptr<VulkanDescriptorSet>         descriptorSet,
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
        bool                                         enableCulling = true,
        bool                                         enableDepth   = true
    );
    ~VulkanPipeline();

    VulkanPipeline(const VulkanPipeline&)            = delete;
    VulkanPipeline(VulkanPipeline&&)                 = default;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(VulkanPipeline&&)      = delete;

    void Bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, uint32_t frameIndex);
    void BindDescriptor(
        VkCommandBuffer     commandBuffer,
        VkPipelineBindPoint bindPoint,
        uint32_t            frameIndex
    );

    VkPipeline GetVkPipeline() const
    {
        return m_vkPipeline;
    }

  private:
    const VulkanDevice&                  m_device;
    std::shared_ptr<VulkanDescriptorSet> m_descriptorSet;

    VkPipelineLayout m_vkPipelineLayout;
    VkPipeline       m_vkPipeline;
};

template<typename V>
VulkanPipeline<V>::VulkanPipeline(
    const VulkanDevice&                          device,
    std::shared_ptr<VulkanDescriptorSet>         descriptorSet,
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages,
    bool                                         enableCulling,
    bool                                         enableDepth
) :
    m_device(device),
    m_descriptorSet(descriptorSet)
{
    LOG_DEBUG("Creating VulkanPipeline");

    const VkExtent2D swapchainExtent = m_device.GetSwapchainExtent();

    const std::vector dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicState {};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();

    const auto vertexBindingDescription    = GetVulkanBindingDescription<V>();
    const auto vertexAttributeDescriptions = GetVulkanAttributeDescriptions<V>();

    VkPipelineVertexInputStateCreateInfo vertexInputState {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(vertexAttributeDescriptions.size());
    vertexInputState.pVertexBindingDescriptions   = &vertexBindingDescription;
    vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState {};
    inputAssemblyState.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport {};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(swapchainExtent.width);
    viewport.height   = static_cast<float>(swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};
    scissor.extent = swapchainExtent;
    scissor.offset = {0, 0};

    VkPipelineViewportStateCreateInfo viewportState {};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationState {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable        = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizationState.lineWidth               = 1.0f;
    rasterizationState.cullMode        = enableCulling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    rasterizationState.frontFace       = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleState {};
    multisampleState.sType               = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment {};
    colorBlendAttachment.blendEnable    = VK_TRUE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendState {};
    colorBlendState.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable   = VK_FALSE;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments    = &colorBlendAttachment;

    auto descriptorSetLayouts = descriptorSet->GetLayouts();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
    pipelineLayoutInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts    = descriptorSetLayouts.data();

    VK_CHECK(
        vkCreatePipelineLayout(
            m_device.GetVkDevice(),
            &pipelineLayoutInfo,
            nullptr,
            &m_vkPipelineLayout
        ),
        "Failed to create pipeline layout"
    );

    VkPipelineDepthStencilStateCreateInfo depthStencilState {};
    depthStencilState.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable  = enableDepth ? VK_TRUE : VK_FALSE;
    depthStencilState.depthWriteEnable = enableDepth ? VK_TRUE : VK_FALSE;
    depthStencilState.depthCompareOp   = VK_COMPARE_OP_LESS;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.minDepthBounds        = 0.0f;
    depthStencilState.maxDepthBounds        = 1.0f;
    depthStencilState.stencilTestEnable     = VK_FALSE;
    depthStencilState.front                 = {};
    depthStencilState.back                  = {};

    VkFormat colorFormat = m_device.GetSwapchainImageFormat();
    VkFormat depthFormat = m_device.GetDepthFormat();

    VkPipelineRenderingCreateInfoKHR pipelineCreateInfo {};
    pipelineCreateInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipelineCreateInfo.pNext                = VK_NULL_HANDLE;
    pipelineCreateInfo.colorAttachmentCount = 1;
    pipelineCreateInfo.pColorAttachmentFormats = &colorFormat;
    pipelineCreateInfo.depthAttachmentFormat   = depthFormat;
    pipelineCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkGraphicsPipelineCreateInfo graphicsCreateInfo {};
    graphicsCreateInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsCreateInfo.pNext               = &pipelineCreateInfo;
    graphicsCreateInfo.renderPass          = VK_NULL_HANDLE;
    graphicsCreateInfo.pVertexInputState   = &vertexInputState;
    graphicsCreateInfo.pInputAssemblyState = &inputAssemblyState;
    graphicsCreateInfo.pViewportState      = &viewportState;
    graphicsCreateInfo.pRasterizationState = &rasterizationState;
    graphicsCreateInfo.pMultisampleState   = &multisampleState;
    graphicsCreateInfo.pDepthStencilState  = &depthStencilState;
    graphicsCreateInfo.pColorBlendState    = &colorBlendState;
    graphicsCreateInfo.pDynamicState       = &dynamicState;
    graphicsCreateInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
    graphicsCreateInfo.pStages             = shaderStages.data();
    graphicsCreateInfo.layout              = m_vkPipelineLayout;

    VK_CHECK(
        vkCreateGraphicsPipelines(
            m_device.GetVkDevice(),
            VK_NULL_HANDLE,
            1,
            &graphicsCreateInfo,
            nullptr,
            &m_vkPipeline
        ),
        "Failed to create graphics pipeline"
    );
}

template<typename V>
VulkanPipeline<V>::~VulkanPipeline()
{
    LOG_DEBUG("Destroying VulkanPipeline");

    vkDestroyPipeline(m_device.GetVkDevice(), m_vkPipeline, nullptr);
    vkDestroyPipelineLayout(m_device.GetVkDevice(), m_vkPipelineLayout, nullptr);
}

template<typename V>
void VulkanPipeline<V>::Bind(
    VkCommandBuffer     commandBuffer,
    VkPipelineBindPoint bindPoint,
    uint32_t            frameIndex
)
{
    vkCmdBindPipeline(commandBuffer, bindPoint, m_vkPipeline);
    BindDescriptor(commandBuffer, bindPoint, frameIndex);
}

template<typename V>
void VulkanPipeline<V>::BindDescriptor(
    VkCommandBuffer     commandBuffer,
    VkPipelineBindPoint bindPoint,
    uint32_t            frameIndex
)
{
    m_descriptorSet->Bind(commandBuffer, bindPoint, m_vkPipelineLayout, frameIndex);
}
} // namespace drive
