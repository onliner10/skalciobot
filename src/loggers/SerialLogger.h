#pragma once
#include <Arduino.h>  // Add Arduino.h for Serial
#include "../Logger.h"
#include "MessageFormatter.h"

class SerialLogger : public Logger {
public:
    SerialLogger(Logger* next = nullptr) : Logger(next) {}

    void error(const String& message, LogContext context) override {
        Serial.println(MessageFormatter::format(message, context, "ERROR"));
        if (next) next->error(message, context);
    }

    void warning(const String& message, LogContext context) override {
        Serial.println(MessageFormatter::format(message, context, "WARN"));
        if (next) next->warning(message, context);
    }

    void info(const String& message, LogContext context) override {
        Serial.println(MessageFormatter::format(message, context, "INFO"));
        if (next) next->info(message, context);
    }

    void debug(const String& message, LogContext context) override {
        Serial.println(MessageFormatter::format(message, context, "DEBUG"));
        if (next) next->debug(message, context);
    }
};
