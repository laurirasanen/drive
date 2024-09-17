#pragma once

#include <cstring>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "../../Log.h"
#include "VmaUsage.h"
#include "VulkanAttributes.h"
#include "VulkanDevice.h"

namespace drive
{

enum VulkanBufferType
{
    VertexBuffer,
    IndexBuffer,
    UniformBuffer,
};

enum VulkanBufferLocation
{
    Host,
    Device,
};

template<class T, VulkanBufferType B>
class VulkanBuffer
{
  public:
    VulkanBuffer() = delete;
    VulkanBuffer(VulkanBufferLocation location, uint32_t count);

    ~VulkanBuffer();

    VulkanBuffer(const VulkanBuffer&)            = delete;
    VulkanBuffer(VulkanBuffer&&)                 = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(VulkanBuffer&&)      = delete;

    template<class IT>
    requires std::contiguous_iterator<IT>
    void Write(const IT it, uint32_t length)
    {
        if (m_bufferLocation != Host)
        {
            throw std::runtime_error("Tried mapping a non-host buffer");
        }

        VkDeviceSize size = length * sizeof(T);
        if (size > m_size)
        {
            throw std::runtime_error("Tried to write more data than allocated");
        }

        vmaCopyMemoryToAllocation(g_vma, &(*it), m_vmaAllocation, 0, size);
        m_elementCount = length;
    }

    void CopyToDevice(VulkanDevice& device, std::shared_ptr<VulkanBuffer> deviceBuffer);

    void Clear();

    void Bind(VkCommandBuffer commandBuffer);

    void Draw(VkCommandBuffer commandBuffer);
    void Draw(
        VkCommandBuffer commandBuffer,
        uint32_t        indexOffset,
        uint32_t        indexCount,
        int32_t         vertexOffset
    );

    void Map(void** data);
    void Unmap();

    void SetElementCount(uint32_t count)
    {
        m_elementCount = count;
    }

    uint32_t ElementCount() const
    {
        return m_elementCount;
    }

    constexpr VulkanBufferType GetType() const
    {
        return B;
    }

    VulkanBufferLocation GetDestination() const
    {
        return m_bufferLocation;
    }

    VkDeviceSize GetSize() const
    {
        return m_size;
    }

    size_t ElementSize()
    {
        return sizeof(T);
    }

    VkBuffer GetVkBuffer() const
    {
        return m_vkBuffer;
    }

    consteval const char* PrettyElementType()
    {
        if constexpr (std::is_same<T, SimpleVertex>::value)
        {
            return "Vertex";
        }
        else if constexpr (std::is_same<T, Index>::value)
        {
            return "Index";
        }

        return "Unknown";
    }

    consteval const char* PrettyBufferType()
    {
        if constexpr (B == VertexBuffer)
        {
            return "VertexBuffer";
        }
        else if constexpr (B == IndexBuffer)
        {
            return "IndexBuffer";
        }
        else if constexpr (B == UniformBuffer)
        {
            return "UniformBuffer";
        }

        return "Unknown";
    }

    const char* PrettyBufferLocation()
    {
        switch (m_bufferLocation)
        {
            case Host:
                return "Host";
            case Device:
                return "Device";
            default:
                return "Unkown";
        }
    }

  private:
    VkBuffer                   m_vkBuffer;
    VmaAllocation              m_vmaAllocation;
    const VulkanBufferLocation m_bufferLocation;
    uint32_t                   m_elementCount;
    const VkDeviceSize         m_size;
    bool                       m_isMapped = false;
};

template<class T, VulkanBufferType B>
VulkanBuffer<T, B>::VulkanBuffer(VulkanBufferLocation location, uint32_t count) :
    m_bufferLocation(location),
    m_size(count * sizeof(T))
{
    /*
    LOG_DEBUG(
        "Creating VulkanBuffer<{}, {}> {} {}",
        PrettyElementType(),
        PrettyBufferType(),
        PrettyBufferLocation(),
        GetSize()
    );
    */

    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size        = m_size;
    bufferInfo.usage       = 0;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo {};

    if constexpr (B == VertexBuffer)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    else if constexpr (B == IndexBuffer)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    else if constexpr (B == UniformBuffer)
    {
        bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        allocInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    else
    {
        static_assert(false, "Unhandled buffer type");
    }

    switch (location)
    {
        case Host:
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            allocInfo.requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            allocInfo.preferredFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

            break;
        }

        case Device:
        {
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

            allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
            allocInfo.preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            break;
        }

        default:
        {
            throw std::runtime_error("Unhandled buffer destination");
        }
    }

    vmaCreateBuffer(g_vma, &bufferInfo, &allocInfo, &m_vkBuffer, &m_vmaAllocation, nullptr);
}

template<class T, VulkanBufferType B>
VulkanBuffer<T, B>::~VulkanBuffer()
{
    /*
    LOG_DEBUG(
        "Destroying VulkanBuffer<{}, {}> {} {}",
        PrettyElementType(),
        PrettyBufferType(),
        PrettyBufferLocation(),
        GetSize()
    );
    */
    if (m_isMapped)
    {
        Unmap();
    }
    vmaDestroyBuffer(g_vma, m_vkBuffer, m_vmaAllocation);
}

template<class T, VulkanBufferType B>
void VulkanBuffer<T, B>::CopyToDevice(
    VulkanDevice&                       device,
    std::shared_ptr<VulkanBuffer<T, B>> deviceBuffer
)
{
    if (m_bufferLocation != Host)
    {
        throw std::runtime_error("Tried copying from a non-host buffer");
    }

    if (deviceBuffer->GetDestination() != Device)
    {
        throw std::runtime_error("Tried copying to a non-device buffer");
    }

    VkDeviceSize requiredSize = ElementCount() * ElementSize();
    if (deviceBuffer->GetSize() < requiredSize)
    {
        throw std::runtime_error("Tried copying to a buffer that is too small");
    }

    auto commandBuffer = device.GetTemporaryCommandBuffer();

    VkBufferCopy copyRegion {};
    copyRegion.size      = requiredSize;
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;

    vkCmdCopyBuffer(commandBuffer, m_vkBuffer, deviceBuffer->GetVkBuffer(), 1, &copyRegion);

    device.SubmitTemporaryCommandBuffer(commandBuffer);

    deviceBuffer->SetElementCount(m_elementCount);
}

template<class T, VulkanBufferType B>
void VulkanBuffer<T, B>::Clear()
{
    if (m_bufferLocation != Host)
    {
        throw std::runtime_error("Tried mapping a non-host buffer");
    }

    VmaAllocationInfo allocInfo {};
    vmaGetAllocationInfo(g_vma, m_vmaAllocation, &allocInfo);

    void* mappedData;
    vmaMapMemory(g_vma, m_vmaAllocation, &mappedData);
    std::memset(mappedData, 0, allocInfo.size);
    vmaUnmapMemory(g_vma, m_vmaAllocation);

    if (!(allocInfo.memoryType & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
    {
        vmaFlushAllocation(g_vma, m_vmaAllocation, 0, allocInfo.size);
    }
}

template<class T, VulkanBufferType B>
void VulkanBuffer<T, B>::Bind(VkCommandBuffer commandBuffer)
{
    if (m_bufferLocation != Device)
    {
        throw std::runtime_error("Tried to bind a non-device buffer");
    }

    if constexpr (B == VertexBuffer)
    {
        VkBuffer     buffers[] = {m_vkBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }
    else if constexpr (B == IndexBuffer)
    {
        constexpr auto indexType = GetVulkanIndexType<T>();
        vkCmdBindIndexBuffer(commandBuffer, m_vkBuffer, 0, indexType);
    }
    else
    {
        static_assert(false, "Unhandled buffer type");
    }
}

template<class T, VulkanBufferType B>
void VulkanBuffer<T, B>::Draw(VkCommandBuffer commandBuffer)
{
    if constexpr (B == VertexBuffer)
    {
        vkCmdDraw(commandBuffer, m_elementCount, 1, 0, 0);
    }
    else if constexpr (B == IndexBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, m_elementCount, 1, 0, 0, 0);
    }
    else
    {
        static_assert(false, "Unhandled buffer type");
    }
}

template<class T, VulkanBufferType B>
void VulkanBuffer<T, B>::Draw(
    VkCommandBuffer commandBuffer,
    uint32_t        indexOffset,
    uint32_t        indexCount,
    int32_t         vertexOffset
)
{
    if constexpr (B == IndexBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, indexOffset, vertexOffset, 0);
    }
    else
    {
        static_assert(false, "Unhandled buffer type");
    }
}

template<class T, VulkanBufferType B>
void VulkanBuffer<T, B>::Map(void** data)
{
    if (m_isMapped)
    {
        throw std::runtime_error("Tried to map a buffer that is already mapped");
    }

    vmaMapMemory(g_vma, m_vmaAllocation, data);
    m_isMapped = true;
}

template<class T, VulkanBufferType B>
void VulkanBuffer<T, B>::Unmap()
{
    if (!m_isMapped)
    {
        throw std::runtime_error("Tried to unmap buffer that isn't mapped");
    }

    vmaUnmapMemory(g_vma, m_vmaAllocation);
    m_isMapped = false;
}
} // namespace drive
