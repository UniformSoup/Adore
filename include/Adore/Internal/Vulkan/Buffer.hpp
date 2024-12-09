#pragma once

#include <Adore/Buffer.hpp>

#include <vulkan/vulkan.h>

class VulkanBuffer
{
protected:
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
    VulkanBuffer() {};
public:
    virtual ~VulkanBuffer() {};
    VkBuffer const& buffer() const { return m_buffer; }
};

class VulkanIndexBuffer : public VulkanBuffer, public Adore::IndexBuffer
{
public:
    VulkanIndexBuffer(std::shared_ptr<Adore::Renderer>& win,
                     void* pdata, uint64_t const& size);
    ~VulkanIndexBuffer();
};

class VulkanVertexBuffer : public VulkanBuffer, public Adore::VertexBuffer
{
public:
    VulkanVertexBuffer(std::shared_ptr<Adore::Renderer>& win,
                     void* pdata, uint64_t const& size);
    ~VulkanVertexBuffer();
};
