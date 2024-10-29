#include <sstream>
#include <iomanip>

#include "Logger.hpp"

bool Logger::s_is_enabled = true;
LogLevel Logger::m_level = LogLevel::VERBOSE;

std::string Logger::logLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::FATAL:
        return "FATAL ";
    case LogLevel::ERROR:
        return "ERROR ";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::INFO:
        return "INFO  ";
    case LogLevel::DEBUG:
        return "DEBUG  ";
    case LogLevel::VERBOSE:
        return "VERBOSE  ";
    }

    return "UNKNOWN";
}

const char* Logger::line(int length) {
    static const char* l = "--------------------------------------------------------------------------------------------------";
    return l + 100 - length - 1;
}

void Logger::disable() {
	s_is_enabled = false;
}

void Logger::setLogLevel(LogLevel level) {
    m_level = level;
}

std::string Logger::getTimestamp() {
    // get a precise timestamp as a string
    const auto now = std::chrono::system_clock::now();
    const auto nowAsTimeT = std::chrono::system_clock::to_time_t(now);
    const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    std::stringstream nowSs;
    nowSs
        << std::put_time(std::localtime(&nowAsTimeT), "%T")
        << '.' << std::setfill('0') << std::setw(3) << nowMs.count();
    return nowSs.str();
}
