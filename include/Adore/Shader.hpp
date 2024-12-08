#pragma once

#include <memory>
#include <Adore/Renderer.hpp>

#include "Export.hpp"

namespace Adore
{
    class ADORE_EXPORT Shader
    {
    protected:
        std::shared_ptr<Renderer> m_renderer;
    public:
        enum class Type
        {
            VERTEX,
            FRAGMENT
        };
        static std::shared_ptr<Shader> create(std::shared_ptr<Renderer>& renderer,
                                std::vector<std::pair<Type, std::string>> const& modules);
        Shader(std::shared_ptr<Renderer>& renderer) : m_renderer(renderer) {};
        virtual ~Shader() = default;
    };
}