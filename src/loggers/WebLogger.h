#pragma once
#include "../Logger.h"
#include "CircularLogBuffer.h"
#include "MessageFormatter.h"

class WebLogger : public Logger {
    CircularLogBuffer buffer;

public:
    WebLogger(Logger* next = nullptr) : Logger(next) {}

    void error(const String& message, LogContext context) override {
        buffer.add(MessageFormatter::format(message, context, "ERROR"));
        if (next) next->error(message, context);
    }

    void warning(const String& message, LogContext context) override {
        buffer.add(MessageFormatter::format(message, context, "WARN"));
        if (next) next->warning(message, context);
    }

    void info(const String& message, LogContext context) override {
        buffer.add(MessageFormatter::format(message, context, "INFO"));
        if (next) next->info(message, context);
    }

    void debug(const String& message, LogContext context) override {
        buffer.add(MessageFormatter::format(message, context, "DEBUG"));
        if (next) next->debug(message, context);
    }

    String getLogs() {
        return buffer.getAll();
    }
};
