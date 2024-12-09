#include <Adore/Buffer.hpp>

#include <Adore/Internal/Vulkan/Buffer.hpp>

namespace Adore
{
    std::shared_ptr<VertexBuffer> VertexBuffer::create(std::shared_ptr<Renderer>& renderer,
                                           void* pdata, uint64_t const& size)
    {
        switch (renderer->window()->context()->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanVertexBuffer>(renderer, pdata, size);
        }
    }

    std::shared_ptr<IndexBuffer> IndexBuffer::create(std::shared_ptr<Renderer>& renderer,
                                                void* pdata, uint64_t const& size)
    {
        switch (renderer->window()->context()->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanIndexBuffer>(renderer, pdata, size);
        }
    }
}