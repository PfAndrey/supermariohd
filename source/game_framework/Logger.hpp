#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <chrono>
#include <stdarg.h>
#include <iostream>
#include <sstream>
#include <memory>

#define ENABLE_LOGS

#ifdef ENABLE_LOGS
#define LOG Logger::print
#else
#define LOG if (0)
#endif

enum LogLevel : uint8_t {
    FATAL     = 0,
    ERROR     = 1,
    WARNING   = 2,
    INFO      = 3,
    DEBUG     = 4,
    VERBOSE   = 5,
};

class Logger {
public:
    static void disable();
    static void setLogLevel(LogLevel level);

    template<typename ... Args>
    static void print(const std::string& component, LogLevel logLevel, const std::string& format, Args ... args) {
        if (!s_is_enabled || m_level < logLevel)
            return;

        int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
        if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
        auto size = static_cast<size_t>(size_s);
        std::unique_ptr<char[]> buf(new char[size]);
        std::snprintf(buf.get(), size, format.c_str(), args ...);
 
        auto str = std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside

        std::cout << getTimestamp() << " | " << component
                  << "\t | " << logLevelToString(logLevel) << " | " << str << std::endl;
    }
 
private:
 
    static const char* line(int length = 80);
    static std::string getTimestamp();
    static std::string logLevelToString(LogLevel level);
    static bool s_is_enabled;
    static LogLevel m_level;
};

#endif // ! LOGGER_HPP
