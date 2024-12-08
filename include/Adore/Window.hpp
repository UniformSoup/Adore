#pragma once

#include <memory>

#include <Adore/API.hpp>
#include <Adore/Context.hpp>

#include "Export.hpp"

// get rid of .open and .close
// needs .is_open
// needs .poll

namespace Adore
{
    class ADORE_EXPORT Window
    {
    protected:
        std::shared_ptr<Context> m_ctx;
    public:
        static std::shared_ptr<Window> create(std::shared_ptr<Context>& ctx, std::string const& title);
        Window(std::shared_ptr<Context>& ctx) : m_ctx(ctx) {}
        virtual ~Window() = default;
        virtual void resize(int const& width, int const& height) = 0;
        virtual void close() = 0;
        virtual bool is_open() = 0;
        virtual void poll() = 0;
        virtual void framebufferSize(uint32_t& width, uint32_t& height) = 0;
        std::shared_ptr<Context> context() { return m_ctx; }; 
    };
}