#pragma once
#include "export.h"
#include <mutex>
#include <string>

namespace Adore
{
    class ADORE_EXPORT Logger
    {
    public:
        enum Severity { INFO = 0, WARN = 1, ERROR = 2};
        static Logger& single();
        void log(Severity const& level, std::string const& message);

    private:
        std::mutex m_mutex;
        Logger();
    };
}