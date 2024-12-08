#include <Adore/Window.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>
#include <Adore/Internal/Vulkan/Context.hpp>

namespace Adore
{
    std::shared_ptr<Window> Window::create(std::shared_ptr<Context>& ctx, std::string const& title)
    {
        switch (ctx->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanWindow>(ctx, title);
            default:
                throw AdoreException("Unsupported API.");
        }
    }
}