#include <Adore/Renderer.hpp>
#include <Adore/Internal/Vulkan/Renderer.hpp>

namespace Adore
{
    std::shared_ptr<Renderer> Renderer::create(std::shared_ptr<Window>& win)
    {
        switch (win->context()->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanRenderer>(win);
        }
    }
}