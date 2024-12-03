#include <Adore/Window.hpp>
#include <Adore/Internal/Vulkan/VulkanWindow.hpp>
#include <Adore/Internal/Vulkan/VulkanContext.hpp>

namespace Adore
{
    std::shared_ptr<Window> Window::create(std::shared_ptr<Context>& ctx, std::string const& title)
    {
        switch (ctx->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanWindow>(reinterpret_cast<std::shared_ptr<VulkanContext>&>(ctx), title);
            default:
                throw AdoreException("Unsupported API.");
        }
    }
}