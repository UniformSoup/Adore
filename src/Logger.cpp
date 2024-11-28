#include "Adore/Logger.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>

namespace Adore
{
    static std::string const sevstrings[] = {"INFO", "WARN", "ERROR" };
    static std::string const sevcolors[] = {"\033[1;96m", "\033[1;93m", "\033[1;91m"};
    static std::string const reset = "\033[0m";
    static std::string const underline = "\033[4m";
    static std::string const purple = "\033[1;95m";
    static std::mutex mutex;

    void log_message(std::vector<std::string> const& tags,
                     std::string const& message)
    {
        std::lock_guard<std::mutex> lock(mutex);
#ifdef DEBUG
        std::ostream out(std::clog.rdbuf());
#else
        if (level != Severity::ERROR)   return;
        std::ofstream out("log.txt", std::ios::out | std::ios::app);
        // std::ostream out(outfile);
#endif
        for (auto const& tag : tags)
            out << '[' << tag << "] ";

        out << "\n" << message << "\n\n" << std::flush;
    }

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
        log_message
        (
            {
                sevcolors[level] + sevstrings[level] + reset
            },
            sevcolors[level] + message + reset
        );
    }
}