#include <Adore/Buffer.hpp>

#include <Adore/Internal/Vulkan/Buffer.hpp>

namespace Adore
{
    std::shared_ptr<Buffer> Buffer::create(std::shared_ptr<Renderer>& renderer,
                    Usage const& usage, void* pdata, uint64_t const& size)
    {
        switch (renderer->window()->context()->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanBuffer>(renderer, usage, pdata, size);
        }
    }
}