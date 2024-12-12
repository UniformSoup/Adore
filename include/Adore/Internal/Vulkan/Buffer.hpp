#pragma once

#include <Adore/Buffer.hpp>
#include <Adore/Internal/FramesInFlight.hpp>

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
    VulkanIndexBuffer(std::shared_ptr<Adore::Renderer>& renderer,
                     void* pdata, uint64_t const& size);
    ~VulkanIndexBuffer();
};

class VulkanVertexBuffer : public VulkanBuffer, public Adore::VertexBuffer
{
public:
    VulkanVertexBuffer(std::shared_ptr<Adore::Renderer>& renderer,
                     void* pdata, uint64_t const& size);
    ~VulkanVertexBuffer();
};

class VulkanUniformBuffer : public Adore::UniformBuffer
{
    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_memories;
    std::vector<void*> m_maps;
    std::vector<bool> m_changed;
public:
    VulkanUniformBuffer(std::shared_ptr<Adore::Renderer>& renderer,
                        void* pdata, uint64_t const& size);

    void set(void const * pdata) override;
    VkBuffer buffer(size_t const& index);
    void update(size_t const& index);

    ~VulkanUniformBuffer();
};

class VulkanSampler : public Adore::Sampler
{
    VkImage m_image;
    VkDeviceMemory m_memory;
    VkImageView m_view;
    VkSampler m_sampler;
public:
    VulkanSampler(std::shared_ptr<Adore::Renderer>& renderer, const char* path,
                  Adore::Filter const& filter, Adore::Wrap const& wrap);
    ~VulkanSampler();
    VkImageView const& view() const { return m_view; }
    VkSampler const& sampler() const { return m_sampler; }
};