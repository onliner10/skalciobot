#include "TimerManager.h"

esp_timer_handle_t TimerManager::timer = nullptr;
TimerCallback* TimerManager::callback = nullptr;

void IRAM_ATTR TimerManager::timerCallback(void* arg) {
    if (callback) {
        callback->onTimer();
    }
}

bool TimerManager::begin(TimerCallback* cb, uint32_t intervalMicros) {
    callback = cb;
    
    esp_timer_create_args_t args = {};
    args.callback = timerCallback;
    args.name = "pid_timer";
    
    if (esp_timer_create(&args, &timer) != ESP_OK) {
        return false;
    }
    
    return esp_timer_start_periodic(timer, intervalMicros) == ESP_OK;
}

void TimerManager::cleanup() {
    if (timer != nullptr) {
        esp_timer_stop(timer);
        esp_timer_delete(timer);
        timer = nullptr;
        callback = nullptr;
    }
}
