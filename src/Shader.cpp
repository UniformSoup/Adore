#include <Adore/Shader.hpp>
#include <Adore/Internal/Log.hpp>
#include <Adore/Internal/Vulkan/Shader.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>

namespace Adore
{
    std::shared_ptr<Shader> Shader::create(std::shared_ptr<Window>& win,
                                std::vector<ShaderModule> const& modules,
                                LayoutDescriptor const& descriptor)
    {
        switch (win->context()->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanShader>(win, modules, descriptor);
            default:
                throw AdoreException("Unsupported API.");
        }
    }
}