#pragma once

#include <Adore/Log.hpp>

#ifdef DEBUG
    static std::string const sevstrings[] = {"INFO", "WARN", "ERROR" };
    static std::string const sevcolors[] = {"\033[1;96m", "\033[1;93m", "\033[1;91m"};
    static std::string const reset = "\033[0m";
    static std::string const underline = "\033[4m";
    static std::string const purple = "\033[1;95m";
#else
    static std::string const sevstrings[] = {"INFO", "WARN", "ERROR" };
    static std::string const sevcolors[] = {"", "", ""};
    static std::string const reset = "";
    static std::string const underline = "";
    static std::string const purple = "";
#endif

void log_message(std::vector<std::string> const& tags, std::string const& message);

#define ADORE_INTERNAL_LOG(level, msg) log_message                  \
        (                                                           \
            {                                                       \
                purple + "Adore" + reset,                           \
                sevcolors[Adore::Severity::level]                   \
                    + sevstrings[Adore::Severity::level] + reset    \
            },                                                      \
            sevcolors[Adore::Severity::level] + msg + reset         \
        ); 