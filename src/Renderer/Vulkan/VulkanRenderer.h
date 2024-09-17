#pragma once

#include <memory>
#include <stdexcept>

#include <imgui_impl_vulkan.h>

#include "../../World/Chunk.h"
#include "../Renderer.h"
#include "VulkanBuffer.h"
#include "VulkanCommon.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanPipeline.h"

namespace drive
{
enum RenderPipeline
{
    TEST,
    TERRAIN,
    FULLSCREEN,
};

class VulkanRenderer final : public Renderer
{
  public:
    VulkanRenderer(std::shared_ptr<Window> window);
    ~VulkanRenderer();

    VulkanRenderer(const VulkanRenderer&)            = delete;
    VulkanRenderer(VulkanRenderer&&)                 = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(VulkanRenderer&&)      = delete;

    void SetWindow(std::shared_ptr<Window> window) override;

    void ResetViewport() override
    {
        m_device.ResetViewport();
    }

    void SetViewport(Rect rect) override
    {
        m_device.SetViewport(rect);
    }

    void  ClearViewport() override;
    void  Resize() override;
    float GetAspect() override;
    void  Begin() override;
    void  Submit() override;
    void  Present() override;
    void  UpdateUniforms(const std::shared_ptr<Camera> camera) override;
    void  DrawTest() override;
    void  WaitForIdle() override;

    RendererType Type() const override
    {
        return RendererType::VULKAN;
    }

    void GetImGuiInfo(VulkanImGuiCreationInfo& info);

    VkCommandBuffer GetCommandBuffer() const
    {
        return m_device.GetCommandBuffer();
    }

    template<class T, VulkanBufferType B, class IT>
    requires std::contiguous_iterator<IT>
    void CreateBuffer(std::shared_ptr<VulkanBuffer<T, B>>& buffer, IT it, uint32_t length)
    {
        auto hostBuffer   = std::make_shared<VulkanBuffer<T, B>>(Host, length);
        auto deviceBuffer = std::make_shared<VulkanBuffer<T, B>>(Device, length);
        hostBuffer->Write(it, length);
        hostBuffer->CopyToDevice(m_device, deviceBuffer);
        buffer = deviceBuffer;
    }

    void BindPipeline(RenderPipeline pipe)
    {
        auto commandBuffer = m_device.GetCommandBuffer();
        auto currentFrame  = m_device.GetCurrentFrame();

        switch (pipe)
        {
            case TEST:
                m_testPipeline->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);
                break;

            case TERRAIN:
                m_terrainPipeline
                    ->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);
                break;

            case FULLSCREEN:
                m_fullscreenPipeline
                    ->Bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);
                break;

            default:
                throw std::runtime_error("unknown pipeline");
        }
    }

    template<class V, class I>
    void DrawWithBuffers(
        std::shared_ptr<VulkanBuffer<V, VulkanBufferType::VertexBuffer>> vertexBuffer,
        std::shared_ptr<VulkanBuffer<I, VulkanBufferType::IndexBuffer>>  indexBuffer
    )
    {
        auto commandBuffer = m_device.GetCommandBuffer();
        vertexBuffer->Bind(commandBuffer);
        indexBuffer->Bind(commandBuffer);
        indexBuffer->Draw(commandBuffer);
    }

    void DrawChunk(std::shared_ptr<Chunk> chunk)
    {
        DrawWithBuffers(chunk->vertexBuffer, chunk->indexBuffer);
        m_frameChunks.push_back(chunk);
    }

  private:
    VkShaderModule& CreateShaderModule(VkShaderModuleCreateInfo createInfo);
    constexpr VkPipelineShaderStageCreateInfo FillShaderStageCreateInfo(
        VkShaderModule&       module,
        VkShaderStageFlagBits stage
    );

    VulkanInstance m_instance;
    VulkanDevice   m_device;

    std::shared_ptr<VulkanDescriptorSet> m_descriptorSet;
    std::vector<VkShaderModule>          m_vkShaderModules;

    std::shared_ptr<VulkanPipeline<SimpleVertex>>  m_testPipeline;
    std::shared_ptr<VulkanPipeline<TerrainVertex>> m_terrainPipeline;
    std::shared_ptr<VulkanPipeline<EmptyVertex>>   m_fullscreenPipeline;

    std::vector<std::shared_ptr<VulkanBuffer<SimpleVertex, VertexBuffer>>> m_hostVertexBuffers;
    std::vector<std::shared_ptr<VulkanBuffer<Index, IndexBuffer>>>         m_hostIndexBuffers;
    std::vector<std::shared_ptr<VulkanBuffer<SimpleVertex, VertexBuffer>>> m_deviceVertexBuffers;
    std::vector<std::shared_ptr<VulkanBuffer<Index, IndexBuffer>>>         m_deviceIndexBuffers;

    // Hold on to chunk pointer so we don't call VulkanBuffer destructor
    // while still in use by command buffer.
    // TODO: untemplate VulkanBuffer for generic ref holding? :(
    std::vector<std::shared_ptr<Chunk>> m_frameChunks;
};
} // namespace drive
