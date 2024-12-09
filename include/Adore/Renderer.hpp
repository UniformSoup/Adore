#pragma once
#include <Adore/Window.hpp>
#include <Adore/Shader.hpp>

#include "Export.hpp"

// Circular includes, Shader depends on Renderer (I don't like...)
// Renderer .render() should be split into:
    // .begin() <- requires shader pipeline
    // .draw()
    // .end()


namespace Adore
{    
    class ADORE_EXPORT Renderer
    {
    public:
        static std::shared_ptr<Renderer> create(std::shared_ptr<Window>& win);
        Renderer(std::shared_ptr<Window>& win) : m_win(win) {};
        virtual ~Renderer() = default;
        virtual void begin(std::shared_ptr<Adore::Shader>& shader) = 0;
        virtual void draw() = 0;
        virtual void end() = 0;
        std::shared_ptr<Window> window() { return m_win; };

    protected:
        std::shared_ptr<Window> m_win;
    };
}