#pragma once
#include <Arduino.h>

class CircularLogBuffer {
    static const size_t BUFFER_SIZE = 50;  // Store last 50 messages
    String messages[BUFFER_SIZE];
    size_t writeIndex = 0;
    size_t readIndex = 0;
    bool hasWrapped = false;

public:
    void add(const String& message) {
        messages[writeIndex] = message;
        writeIndex = (writeIndex + 1) % BUFFER_SIZE;
        if (writeIndex == 0) hasWrapped = true;
    }

    String getAll() {
        String result;
        size_t start = hasWrapped ? writeIndex : 0;
        size_t count = hasWrapped ? BUFFER_SIZE : writeIndex;
        
        for (size_t i = 0; i < count; i++) {
            size_t idx = (start + i) % BUFFER_SIZE;
            if (messages[idx].length() > 0) {
                result += messages[idx] + "\n";
            }
        }
        return result;
    }
};
