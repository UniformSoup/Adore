#include <Adore/Internal/Log.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

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

std::string vec_to_string(std::vector<char const*> const& vec, std::string const& delim, std::string const& end)
{
    std::stringstream ss;
    for (unsigned int i = 0; i < vec.size(); i++)
        ss << vec[i] << ((i == vec.size() - 1) ? end : delim);
    return ss.str();
}