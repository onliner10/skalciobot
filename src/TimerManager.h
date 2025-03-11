#pragma once
#include <Arduino.h>
#include "esp_timer.h"

class TimerCallback {
public:
    virtual void onTimer() = 0;
    virtual ~TimerCallback() = default;
};

class TimerManager {
private:
    static esp_timer_handle_t timer;
    static TimerCallback* callback;
    static void IRAM_ATTR timerCallback(void* arg);

public:
    static bool begin(TimerCallback* cb, uint32_t intervalMicros);
    static void cleanup();  // Add cleanup method
};
