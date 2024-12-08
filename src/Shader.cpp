#include <Adore/Shader.hpp>
#include <Adore/Internal/Log.hpp>
#include <Adore/Internal/Vulkan/Shader.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>

namespace Adore
{
    std::shared_ptr<Shader> Shader::create(std::shared_ptr<Renderer>& renderer,
                                std::vector<std::pair<Type, std::string>> const& modules)
    {
        switch (renderer->window()->context()->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanShader>(renderer, modules);
            default:
                throw AdoreException("Unsupported API.");
        }
    }
}