#pragma once

#include <memory>
#include <string>

#include <Adore/Window.hpp>
#include <Adore/Export.hpp>

namespace Adore
{
    class ADORE_EXPORT Shader
    {
    public:
        enum class Type
        {
            VERTEX,
            FRAGMENT
        };

        struct Module
        {
            Type type;
            std::string path;
        };

        struct InputDescriptor
        {
            enum class Format
            {
                FLOAT,
                DOUBLE,
                INT,
                UINT,

                VEC2_FLOAT,
                VEC3_FLOAT,
                VEC4_FLOAT,

                VEC2_DOUBLE,
                VEC3_DOUBLE,
                VEC4_DOUBLE, 

                VEC2_INT,
                VEC3_INT,
                VEC4_INT,

                VEC2_UINT,
                VEC3_UINT,
                VEC4_UINT
            };

            struct Attribute
            {
                uint32_t binding;
                uint32_t location;
                uint32_t offset;
                Format format;
            };

            struct Binding
            {
                uint32_t binding;
                uint32_t stride;
            };

            std::vector<Attribute> attributes;
            std::vector<Binding> bindings;
        };

    protected:
        std::shared_ptr<Window> m_win;
        InputDescriptor m_descriptor;
    public:

        static std::shared_ptr<Shader> create(std::shared_ptr<Window>& win,
                                std::vector<Module> const& modules,
                                InputDescriptor const& descriptor);
        Shader(std::shared_ptr<Window>& win, InputDescriptor const& descriptor)
                    : m_win(win), m_descriptor(descriptor) {};
        std::shared_ptr<Window> window() { return m_win; }
        InputDescriptor const& descriptor() const { return m_descriptor; }
        virtual ~Shader() = default;
    };
}