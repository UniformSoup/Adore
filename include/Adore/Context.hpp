#pragma once
#include <memory>
#include "Export.hpp"

#include <Adore/API.hpp>

namespace Adore
{   
    class ADORE_EXPORT Context
    {
    public:
        static std::shared_ptr<Context> create(API const& api, std::string const& appName);
        API const api;
        Context(API const& api) : api(api) {};
        virtual ~Context() = default;
    };
}