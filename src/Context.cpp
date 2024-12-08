#include <Adore/Context.hpp>
#include <Adore/Internal/Vulkan/Context.hpp>
#include <Adore/Log.hpp>

namespace Adore
{
    std::shared_ptr<Context> Context::create(API const& api, std::string const& appName)
    {
        switch (api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanContext>(appName);
            default:
                throw AdoreException("Unsupported API.");
        }
    }
}