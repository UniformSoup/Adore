#include "Adore/Logger.hpp"
#include <fstream>
#include <iostream>
#include <ctime>

namespace Adore
{
    char const * color(Logger::Severity const& level)
    {
        switch (level)
        {
            case Logger::Severity::INFO:    return "\033[38;5;69m";
            case Logger::Severity::WARN:    return "\033[38;5;226m";
            case Logger::Severity::ERROR:   return "\033[38;5;196m";
        }
    }

    static char const* sevstrings[] = {"INFO", "WARN", "ERROR" };
    static char const* reset = "\033[0m";

    Logger::Logger()
    {

    }

    Logger& Logger::single()
    {
        static Logger instance;
        return instance;
    }

    void Logger::log(Severity const& level, std::string const& message)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
#ifdef DEBUG
        std::ostream out(std::cout.rdbuf());
#else
        if (level != Severity::ERROR)   return;
        std::ofstream out("log.txt", std::ios::out | std::ios::app);
        // std::ostream out(outfile);
#endif
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        out << reset << '[' << color(level) << sevstrings[level] << reset
            << std::put_time(&tm, " %d/%m/%Y %H:%M:%S]\n")
            << message << reset << "\n\n" << std::flush;
    }
}