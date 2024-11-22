#pragma once

#include <stdexcept>

#include <export.h>
#include "Logger.hpp"

namespace Adore
{
    class ADORE_EXPORT AdoreException : public std::exception
    {
    public:
        AdoreException(std::string const& message)
        {
            Logger::single().log(Logger::Severity::ERROR, "Adore: " + message);
        }
        char const * what() const noexcept override
        {
            return "An error ocurred inside Adore, see the log for more details.";
        }
    };
}