#pragma once
#include <Adore/Window.hpp>
#include <Adore/Shader.hpp>

#include "Export.hpp"

namespace Adore
{
    class Buffer;
    class ADORE_EXPORT Renderer
    {
    public:
        static std::shared_ptr<Renderer> create(std::shared_ptr<Window>& win);
        Renderer(std::shared_ptr<Window>& win) : m_win(win) {};
        virtual ~Renderer() = default;
        virtual void begin(std::shared_ptr<Shader>& shader) = 0;
        virtual void bind(std::shared_ptr<Buffer>& buffer, uint32_t const& binding) = 0;
        virtual void draw(uint32_t const& count) = 0;
        virtual void end() = 0;
        std::shared_ptr<Window> window() { return m_win; };

    protected:
        std::shared_ptr<Window> m_win;
    };
}