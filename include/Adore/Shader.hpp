#pragma once

#include <memory>
#include <string>

#include <Adore/Window.hpp>
#include <Adore/Export.hpp>

namespace Adore
{
    class UniformBuffer;
    enum class ShaderType  { VERTEX, FRAGMENT };
    enum class UniformType { BUFFER, SAMPLER };
    struct ADORE_EXPORT ShaderModule { ShaderType type; std::string path; };

    enum class AttributeFormat
    {
        FLOAT, DOUBLE, INT, UINT,
        VEC2_FLOAT, VEC3_FLOAT, VEC4_FLOAT,
        VEC2_DOUBLE, VEC3_DOUBLE, VEC4_DOUBLE, 
        VEC2_INT, VEC3_INT, VEC4_INT,
        VEC2_UINT, VEC3_UINT, VEC4_UINT
    };

    struct ADORE_EXPORT AttributeLayout
    {
        uint32_t        binding;
        uint32_t        location;
        uint32_t        offset;
        AttributeFormat format;
    };

    struct ADORE_EXPORT BindingLayout
    {
        uint32_t binding;
        uint32_t stride;
    };

    struct ADORE_EXPORT UniformLayout
    {
        uint32_t    binding;
        uint32_t    count;
        ShaderType  stage;
        UniformType type;
    };

    struct ADORE_EXPORT LayoutDescriptor
    {
        std::vector<AttributeLayout>    attributes;
        std::vector<BindingLayout>      bindings;
        std::vector<UniformLayout>      uniforms;
    };

    class ADORE_EXPORT Shader
    {
    protected:
        std::shared_ptr<Window> m_win;
        std::vector<std::shared_ptr<UniformBuffer>> m_uniformBuffers;
        LayoutDescriptor m_descriptor;
    public:

        static std::shared_ptr<Shader> create(std::shared_ptr<Window>& win,
                std::vector<ShaderModule> const& modules,
                Adore::LayoutDescriptor const& descriptor,
                std::vector<std::shared_ptr<UniformBuffer>> const& uniformBuffers);
        Shader(std::shared_ptr<Window>& win, LayoutDescriptor const& descriptor,
                std::vector<std::shared_ptr<UniformBuffer>> const& uniformBuffers)
                    : m_win(win), m_uniformBuffers(uniformBuffers), m_descriptor(descriptor) {};
        std::shared_ptr<Window> window() { return m_win; }
        LayoutDescriptor const& descriptor() const { return m_descriptor; }
        std::vector<std::shared_ptr<UniformBuffer>> const& uniforms() const { return m_uniformBuffers; }
        virtual ~Shader() = default;
    };
}