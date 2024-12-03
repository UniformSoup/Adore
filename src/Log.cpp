#include <Adore/Internal/Log.hpp>

#include <sstream>
#include <iomanip>
#include <ctime>

namespace Adore
{
    AdoreException::AdoreException(std::string const& message)
    {
        auto t = std::time(nullptr);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&t), "%d/%m/%Y %H:%M:%S");
        log_message
        (
            {
                purple + "Adore" + reset,
                sevcolors[ERROR] + sevstrings[ERROR] + reset,
                underline + ss.str() + reset
            },
            purple + message + reset
        );
    }

    void log(Severity const& level, std::string const& message)
    {
#ifndef DEBUG
        if (level != Severity::ERROR)   return;
#endif
        log_message
        (
            {
                sevcolors[level] + sevstrings[level] + reset
            },
            sevcolors[level] + message + reset
        );
    }
}