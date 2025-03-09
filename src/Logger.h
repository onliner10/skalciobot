#pragma once
#include <Arduino.h>

enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3
};

enum class LogContext {
    Navigation,
    ModeSwitch,
    Wifi,
    Sensor,
    Motor,
    System,
    Boot
};

class Logger {
protected:
    Logger* next = nullptr;

public:
    Logger(Logger* nextLogger = nullptr) : next(nextLogger) {}
    virtual ~Logger() = default;

    virtual void error(const String& message, LogContext context) {
        if (next) next->error(message, context);
    }
    
    virtual void warning(const String& message, LogContext context) {
        if (next) next->warning(message, context);
    }
    
    virtual void info(const String& message, LogContext context) {
        if (next) next->info(message, context);
    }
    
    virtual void debug(const String& message, LogContext context) {
        if (next) next->debug(message, context);
    }

    virtual void update() {
        if (next) next->update();
    }

protected:
    String contextToString(LogContext context) {
        switch(context) {
            case LogContext::Navigation: return "NAV";
            case LogContext::ModeSwitch: return "MODE";
            case LogContext::Wifi: return "WIFI";
            case LogContext::Sensor: return "SENS";
            case LogContext::Motor: return "MOTR";
            case LogContext::System: return "SYS";
            case LogContext::Boot: return "BOOT";
            default: return "????";
        }
    }
};
