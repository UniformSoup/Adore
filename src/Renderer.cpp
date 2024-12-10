#include <Adore/Renderer.hpp>
#include <Adore/Internal/Vulkan/Renderer.hpp>
#include <Adore/Internal/Log.hpp>

namespace Adore
{
    std::shared_ptr<Renderer> Renderer::create(std::shared_ptr<Window>& win)
    {
        switch (win->context()->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanRenderer>(win);
            default:
                throw AdoreException("Unsupported API.");
        }
    }
}