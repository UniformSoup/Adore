#pragma once

#include <memory>
#include <Adore/Window.hpp>

#include "Export.hpp"

namespace Adore
{
    class ADORE_EXPORT Shader
    {
    protected:
        std::shared_ptr<Window> m_win;
    public:
        enum class Type
        {
            VERTEX,
            FRAGMENT
        };
        static std::shared_ptr<Shader> create(std::shared_ptr<Window>& win,
                                std::vector<std::pair<Type, std::string>> const& modules);
        Shader(std::shared_ptr<Window>& win) : m_win(win) {};
        std::shared_ptr<Window> window() { return m_win; }
        virtual ~Shader() = default;
    };
}