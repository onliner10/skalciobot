#pragma once
#include "../Logger.h"

class LedLogger : public Logger {
    const uint8_t ledPin;
    bool ledState = false;
    int remainingBlinks = 0;
    unsigned long lastBlinkTime = 0;
    static const unsigned long BLINK_INTERVAL = 50;  // 50ms on/off time
    static const int ERROR_BLINKS = 3;
    static const int WARNING_BLINKS = 2;
    static const int INFO_BLINKS = 1;

public:
    LedLogger(uint8_t pin, Logger* next = nullptr) : Logger(next), ledPin(pin) {
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, LOW);
    }

    void error(const String& message, LogContext context) override {
        remainingBlinks = ERROR_BLINKS * 2;  // *2 because each blink is on+off
        if (next) next->error(message, context);
    }

    void warning(const String& message, LogContext context) override {
        remainingBlinks = WARNING_BLINKS * 2;
        if (next) next->warning(message, context);
    }

    void info(const String& message, LogContext context) override {
        if (next) next->info(message, context);
    }

    void debug(const String& message, LogContext context) override {
        if (next) next->debug(message, context);
    }

    void update() override {
        if (remainingBlinks > 0 && millis() - lastBlinkTime >= BLINK_INTERVAL) {
            ledState = !ledState;
            digitalWrite(ledPin, ledState);
            lastBlinkTime = millis();
            remainingBlinks--;
        } else if (remainingBlinks == 0) {
            digitalWrite(ledPin, LOW);
            ledState = false;
        }
        
        if (next) next->update();
    }
};
