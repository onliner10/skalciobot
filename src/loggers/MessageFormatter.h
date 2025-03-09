#pragma once
#include <Arduino.h>
#include "../Logger.h"

class MessageFormatter {
public:
    static String format(const String& message, LogContext context, const char* level) {
        String timestamp = String(millis() / 1000.0, 1);
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "[%s][%s] %s: %s", 
            timestamp.c_str(),
            contextToString(context).c_str(),
            level,
            message.c_str());
        return String(buffer);
    }

private:
    static String contextToString(LogContext context) {
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
