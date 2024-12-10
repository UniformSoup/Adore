#include <Adore/Shader.hpp>
#include <Adore/Internal/Log.hpp>
#include <Adore/Internal/Vulkan/Shader.hpp>
#include <Adore/Internal/Vulkan/Window.hpp>

namespace Adore
{
    std::shared_ptr<Shader> Shader::create(std::shared_ptr<Window>& win,
                                std::vector<ShaderModule> const& modules,
                                LayoutDescriptor const& descriptor,
                                std::vector<std::shared_ptr<UniformBuffer>> const& uniformBuffers)
    {
        switch (win->context()->api)
        {
            case API::Vulkan:
                return std::make_shared<VulkanShader>(win, modules, descriptor, uniformBuffers);
            default:
                throw AdoreException("Unsupported API.");
        }
    }
}