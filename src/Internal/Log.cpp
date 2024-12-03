#include <Adore/Internal/Log.hpp>

#include <iostream>
#include <fstream>

static std::mutex mutex;

void log_message(std::vector<std::string> const& tags, std::string const& message)
{
    std::lock_guard<std::mutex> lock(mutex);
#ifdef DEBUG
    std::ostream out(std::clog.rdbuf());
#else
    std::ofstream out("log.txt", std::ios::out | std::ios::app);
    // std::ostream out(outfile);
#endif
    for (auto const& tag : tags)
        out << '[' << tag << "] ";

    out << "\n" << message << "\n\n" << std::flush;
}