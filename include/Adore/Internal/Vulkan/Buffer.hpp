#pragma once

#include <Adore/Buffer.hpp>

#include <vulkan/vulkan.h>

class VulkanBuffer : public Adore::Buffer
{
    VkBuffer m_buffer;
    VkDeviceMemory m_memory;
public:
    VulkanBuffer(std::shared_ptr<Adore::Renderer>& win,
                        Adore::Buffer::Usage const& usage, void* pdata,
                        uint64_t const& size);
    ~VulkanBuffer();

    VkBuffer const& buffer() const { return m_buffer; }
};
