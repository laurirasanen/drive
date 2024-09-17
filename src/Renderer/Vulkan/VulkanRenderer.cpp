#include <imgui_impl_vulkan.h>
#include <memory>

#include "../../Log.h"
#include "../DataTypes.h"
#include "../Shader.h"
#include "VulkanRenderer.h"

namespace drive
{

#define MAX_FRAMES_IN_FLIGHT 2

VulkanRenderer::VulkanRenderer(std::shared_ptr<Window> window) :
    m_instance(window),
    m_device(m_instance, MAX_FRAMES_IN_FLIGHT)
{
    LOG_INFO("Creating VulkanRenderer");

    m_descriptorSet = std::make_shared<VulkanDescriptorSet>(m_device, MAX_FRAMES_IN_FLIGHT);

    auto moduleSimpleFrag = CreateShaderModule(LOAD_VULKAN_SPV(Simple_frag));
    auto moduleSimpleVert = CreateShaderModule(LOAD_VULKAN_SPV(Simple_vert));
    auto stageSimpleFrag =
        FillShaderStageCreateInfo(moduleSimpleFrag, VK_SHADER_STAGE_FRAGMENT_BIT);
    auto stageSimpleVert = FillShaderStageCreateInfo(moduleSimpleVert, VK_SHADER_STAGE_VERTEX_BIT);
    std::vector simpleStages {stageSimpleFrag, stageSimpleVert};

    m_testPipeline =
        std::make_shared<VulkanPipeline<SimpleVertex>>(m_device, m_descriptorSet, simpleStages);

    auto moduleTerrainFrag = CreateShaderModule(LOAD_VULKAN_SPV(Terrain_frag));
    auto moduleTerrainVert = CreateShaderModule(LOAD_VULKAN_SPV(Terrain_vert));
    auto stageTerrainFrag =
        FillShaderStageCreateInfo(moduleTerrainFrag, VK_SHADER_STAGE_FRAGMENT_BIT);
    auto stageTerrainVert =
        FillShaderStageCreateInfo(moduleTerrainVert, VK_SHADER_STAGE_VERTEX_BIT);
    std::vector terrainStages {stageTerrainFrag, stageTerrainVert};

    m_terrainPipeline =
        std::make_shared<VulkanPipeline<TerrainVertex>>(m_device, m_descriptorSet, terrainStages);

    auto moduleFullscreenFrag = CreateShaderModule(LOAD_VULKAN_SPV(Fullscreen_frag));
    auto moduleFullscreenVert = CreateShaderModule(LOAD_VULKAN_SPV(Fullscreen_vert));
    auto stageFullscreenFrag =
        FillShaderStageCreateInfo(moduleFullscreenFrag, VK_SHADER_STAGE_FRAGMENT_BIT);
    auto stageFullscreenVert =
        FillShaderStageCreateInfo(moduleFullscreenVert, VK_SHADER_STAGE_VERTEX_BIT);
    std::vector fullscreenStages {stageFullscreenFrag, stageFullscreenVert};

    m_fullscreenPipeline = std::make_shared<VulkanPipeline<EmptyVertex>>(
        m_device,
        m_descriptorSet,
        fullscreenStages,
        false,
        false
    );

    // test plane
    const std::vector<SimpleVertex> vertices = {
        {{-0.5, -0.5, 63.0}, {1.0f, 0.0f, -0.1f}},
        { {0.5, -0.5, 63.0}, {0.0f, 1.0f, -0.1f}},
        {  {0.5, 0.5, 63.0},  {0.0f, 0.0f, 0.9f}},
        { {-0.5, 0.5, 63.0},  {1.0f, 1.0f, 0.9f}},
    };
    const std::vector<Index> indices = {{0}, {1}, {2}, {2}, {3}, {0}};

    auto hostVertexBuffer =
        std::make_shared<VulkanBuffer<SimpleVertex, VertexBuffer>>(Host, vertices.size());
    auto deviceVertexBuffer =
        std::make_shared<VulkanBuffer<SimpleVertex, VertexBuffer>>(Device, vertices.size());
    hostVertexBuffer->Write(vertices.begin(), static_cast<uint32_t>(vertices.size()));
    hostVertexBuffer->CopyToDevice(m_device, deviceVertexBuffer);
    m_deviceVertexBuffers.push_back(deviceVertexBuffer);

    auto hostIndexBuffer = std::make_shared<VulkanBuffer<Index, IndexBuffer>>(Host, indices.size());
    auto deviceIndexBuffer =
        std::make_shared<VulkanBuffer<Index, IndexBuffer>>(Device, indices.size());
    hostIndexBuffer->Write(indices.begin(), static_cast<uint32_t>(indices.size()));
    hostIndexBuffer->CopyToDevice(m_device, deviceIndexBuffer);
    m_deviceIndexBuffers.push_back(deviceIndexBuffer);
}

VulkanRenderer::~VulkanRenderer()
{
    LOG_INFO("Destroying VulkanRenderer");

    vkDeviceWaitIdle(m_device.GetVkDevice());

    m_testPipeline.reset();
    m_terrainPipeline.reset();
    m_fullscreenPipeline.reset();

    m_descriptorSet.reset();

    for (auto& module : m_vkShaderModules)
    {
        vkDestroyShaderModule(m_device.GetVkDevice(), module, nullptr);
    }

    m_deviceVertexBuffers.clear();
    m_deviceIndexBuffers.clear();
    m_hostVertexBuffers.clear();
    m_hostIndexBuffers.clear();
    m_frameChunks.clear();
}

void VulkanRenderer::SetWindow(std::shared_ptr<Window> window)
{
    LOG_INFO("Setting window");
    m_instance.SetWindow(window);
    Resize();
}

void VulkanRenderer::ClearViewport()
{
    auto commandBuffer = m_device.GetCommandBuffer();
    BindPipeline(RenderPipeline::FULLSCREEN);
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

void VulkanRenderer::Resize()
{
    LOG_INFO("Resizing");
    m_device.ResizeFramebuffer();
}

float VulkanRenderer::GetAspect()
{
    return m_device.GetSwapchainAspect();
}

void VulkanRenderer::Begin()
{
    m_device.Begin();
}

void VulkanRenderer::Submit()
{
    m_device.Submit();

    // Not ideal but guarantees chunk buffers aren't freed too early.
    m_device.WaitForGraphicsIdle();
    m_frameChunks.clear();
}

void VulkanRenderer::Present()
{
    m_device.Present();
}

void VulkanRenderer::UpdateUniforms(const std::shared_ptr<Camera> camera)
{
    auto currentFrame = m_device.GetCurrentFrame();
    auto ubo          = UniformBufferObject(camera);
    m_descriptorSet->UpdateUBO(currentFrame, &ubo);
}

void VulkanRenderer::DrawTest()
{
    BindPipeline(RenderPipeline::TEST);
    DrawWithBuffers(m_deviceVertexBuffers[0], m_deviceIndexBuffers[0]);
}

void VulkanRenderer::WaitForIdle()
{
    vkDeviceWaitIdle(m_device.GetVkDevice());
}

void VulkanRenderer::GetImGuiInfo(VulkanImGuiCreationInfo& info)
{
    info.colorFormat     = m_device.GetSwapchainImageFormat();
    VkFormat depthFormat = m_device.GetDepthFormat();

    info.pipelineCreateInfo       = {};
    info.pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    info.pipelineCreateInfo.pNext = VK_NULL_HANDLE;
    info.pipelineCreateInfo.colorAttachmentCount    = 1;
    info.pipelineCreateInfo.pColorAttachmentFormats = &info.colorFormat;
    info.pipelineCreateInfo.depthAttachmentFormat   = depthFormat;
    info.pipelineCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    info.imGuiInfo                             = {};
    info.imGuiInfo.Instance                    = m_instance.GetVkInstance();
    info.imGuiInfo.PhysicalDevice              = m_device.GetVkPhysicalDevice();
    info.imGuiInfo.Device                      = m_device.GetVkDevice();
    info.imGuiInfo.QueueFamily                 = m_device.GetGraphicsQueueIndex();
    info.imGuiInfo.Queue                       = m_device.GetGraphicsQueue();
    info.imGuiInfo.PipelineCache               = VK_NULL_HANDLE; // TODO
    info.imGuiInfo.DescriptorPool              = m_device.GetImGuiDescriptorPool();
    info.imGuiInfo.UseDynamicRendering         = true;
    info.imGuiInfo.PipelineRenderingCreateInfo = info.pipelineCreateInfo;
    info.imGuiInfo.MinImageCount               = MAX_FRAMES_IN_FLIGHT;
    info.imGuiInfo.ImageCount                  = MAX_FRAMES_IN_FLIGHT;
    info.imGuiInfo.MSAASamples                 = VK_SAMPLE_COUNT_1_BIT;
    info.imGuiInfo.Allocator                   = nullptr; // TODO vma?
    info.imGuiInfo.CheckVkResultFn             = ImGuiVkCheck;
}

VkShaderModule& VulkanRenderer::CreateShaderModule(VkShaderModuleCreateInfo createInfo)
{
    VkShaderModule module;
    VK_CHECK(
        vkCreateShaderModule(m_device.GetVkDevice(), &createInfo, nullptr, &module),
        "Failed to create shader module"
    );
    return m_vkShaderModules.emplace_back(module);
}

constexpr VkPipelineShaderStageCreateInfo VulkanRenderer::FillShaderStageCreateInfo(
    VkShaderModule&       module,
    VkShaderStageFlagBits stage
)
{
    VkPipelineShaderStageCreateInfo createInfo {};
    createInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    createInfo.stage  = stage;
    createInfo.module = module;
    createInfo.pName  = "main";
    return createInfo;
}
} // namespace drive
