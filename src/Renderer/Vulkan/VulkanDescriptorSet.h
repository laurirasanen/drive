#pragma once

#include "../DataTypes.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include <vulkan/vulkan_core.h>

namespace drive
{
class VulkanDescriptorSet
{
  public:
    VulkanDescriptorSet() = delete;
    VulkanDescriptorSet(const VulkanDevice& device, uint32_t maxFrames);
    ~VulkanDescriptorSet();

    VulkanDescriptorSet(const VulkanDescriptorSet&)            = delete;
    VulkanDescriptorSet(VulkanDescriptorSet&&)                 = default;
    VulkanDescriptorSet& operator=(const VulkanDescriptorSet&) = delete;
    VulkanDescriptorSet& operator=(VulkanDescriptorSet&&)      = delete;

    void UpdateUBO(uint32_t frameIndex, const UniformBufferObject* ubo);

    void Bind(
        VkCommandBuffer     commandBuffer,
        VkPipelineBindPoint bindPoint,
        VkPipelineLayout    pipelineLayout,
        uint32_t            frameIndex
    );

    std::vector<VkDescriptorSetLayout> GetLayouts() const
    {
        return m_vkLayouts;
    }

  private:
    const VulkanDevice&                                                            m_device;
    std::vector<VkDescriptorSetLayout>                                             m_vkLayouts;
    std::vector<VkDescriptorSet>                                                   m_vkSets;
    std::vector<std::shared_ptr<VulkanBuffer<UniformBufferObject, UniformBuffer>>> m_uniformBuffers;
    std::vector<void*> m_ubosMappedMemory;
};
} // namespace drive
