#pragma once
#include "export.h"
#include <mutex>
#include <string>

#define ADORE_LOG(level, msg) Adore::log(Adore::Severity::level, msg);

namespace Adore
{
    enum Severity { INFO = 0, WARN = 1, ERROR = 2 };
    void ADORE_EXPORT log(Severity const& level, std::string const& message);

    class ADORE_EXPORT AdoreException : public std::exception
    {
    public:
        AdoreException(std::string const& message);
        
        char const * what() const noexcept override
        {
            return "An error ocurred inside Adore, see the log for more details.";
        }
    };   
}
