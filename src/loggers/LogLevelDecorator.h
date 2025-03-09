#pragma once
#include "../Logger.h"

class LogLevelDecorator : public Logger {
    LogLevel minLevel;
    uint32_t filteredContexts;  // Bitfield for contexts to filter out

public:
    LogLevelDecorator(Logger* next, LogLevel level, uint32_t filtered = 0) 
        : Logger(next), minLevel(level), filteredContexts(filtered) {}

    bool shouldLog(LogLevel level, LogContext context) {
        if (level < minLevel) return false;
        return !(filteredContexts & (1 << static_cast<int>(context)));
    }

    void error(const String& message, LogContext context) override {
        if (shouldLog(LogLevel::Error, context) && next) 
            next->error(message, context);
    }

    void warning(const String& message, LogContext context) override {
        if (shouldLog(LogLevel::Warning, context) && next) 
            next->warning(message, context);
    }

    void info(const String& message, LogContext context) override {
        if (shouldLog(LogLevel::Info, context) && next) 
            next->info(message, context);
    }

    void debug(const String& message, LogContext context) override {
        if (shouldLog(LogLevel::Debug, context) && next) 
            next->debug(message, context);
    }
};
