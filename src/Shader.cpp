#include <Adore/Shader.hpp>
#include <Adore/Internal/Log.hpp>
#include <Adore/Internal/Vulkan/Shader.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>

namespace Adore
{
    std::shared_ptr<Shader> Shader::create(std::shared_ptr<Window>& win,
                                std::vector<std::pair<Type, std::string>> const& modules)
    {
        switch (win->context()->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanShader>(win, modules);
            default:
                throw AdoreException("Unsupported API.");
        }
    }
}