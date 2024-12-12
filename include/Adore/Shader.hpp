#pragma once

#include <memory>
#include <string>

#include <Adore/Window.hpp>
#include <Adore/Export.hpp>

namespace Adore
{
    class UniformBuffer;
    class Sampler;

    enum class ShaderType  { VERTEX, FRAGMENT };
    enum class ResourceType { BUFFER, SAMPLER };
    
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

    struct ADORE_EXPORT ResourceLayout
    {
        uint32_t    binding;
        uint32_t    count;
        ShaderType  stage;
        ResourceType type;
    };

    struct ADORE_EXPORT LayoutDescriptor
    {
        std::vector<AttributeLayout>    attributes;
        std::vector<BindingLayout>      bindings;
        std::vector<ResourceLayout>     resources;
    };

    template <typename T>
    struct Binding
    {
        uint32_t binding;
        std::shared_ptr<T> resource;
    };

    class ADORE_EXPORT Shader
    {
    protected:
        std::shared_ptr<Window> m_win;
        std::vector<Binding<UniformBuffer>> m_uniforms;
        std::vector<Binding<Sampler>> m_samplers;
        LayoutDescriptor m_descriptor;
    public:

        static std::shared_ptr<Shader> create(std::shared_ptr<Window>& win,
                std::vector<ShaderModule> const& modules,
                Adore::LayoutDescriptor const& descriptor);
        Shader(std::shared_ptr<Window>& win, LayoutDescriptor const& descriptor)
                    : m_win(win), m_descriptor(descriptor) {};
        std::shared_ptr<Window> window() { return m_win; }
        LayoutDescriptor const& descriptor() const { return m_descriptor; }
        virtual void attach(std::shared_ptr<UniformBuffer>& buffer, uint32_t const& binding) = 0;
        virtual void attach(std::shared_ptr<Sampler>& buffer, uint32_t const& binding) = 0;
        // impl if I can be bothered / ever need it.
        // virtual void detach(uint32_t const& binding) = 0;
        std::vector<Binding<UniformBuffer>> const& uniforms() const { return m_uniforms; }
        std::vector<Binding<Sampler>> const& samplers() const { return m_samplers; }
        virtual ~Shader() = default;
    };
}