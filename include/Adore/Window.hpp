#pragma once

#include <memory>

#include <Adore/API.hpp>
#include <Adore/Context.hpp>

#include "Export.hpp"

namespace Adore
{
    class ADORE_EXPORT Window
    {
    public:
        static std::shared_ptr<Window> create(std::shared_ptr<Context>& ctx, std::string const& title);
        virtual ~Window() = default;
        virtual void resize(int const& width, int const& height) = 0;
        virtual void close() = 0;
        virtual void open() = 0;
    };
}