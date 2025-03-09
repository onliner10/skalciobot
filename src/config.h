#pragma once

// Motor pins
#define DRIVE_MOTOR_1 GPIO_NUM_9
#define DRIVE_MOTOR_2 GPIO_NUM_8
#define STEERING_MOTOR_1 GPIO_NUM_11
#define STEERING_MOTOR_2 GPIO_NUM_10

// Motor control pins
#define MOTOR_SLEEP GPIO_NUM_2
#define MOTOR_FLT GPIO_NUM_3    // Only fault pin we'll use

// I2C pins
#define SDA_PIN GPIO_NUM_33
#define SCL_PIN GPIO_NUM_35

// Distance thresholds (in millimeters)
#define FRONT_OBSTACLE_THRESHOLD 200
#define CLEARANCE_TOLERANCE 100    // 10cm tolerance for straight movement
#define DISTANCE_ROUNDING 50       // Round distances to nearest 5cm

// Movement timing (in milliseconds)
#define BACKUP_DURATION 1000  // Time to spend backing up

// PCF8574 pins
#define BUZZER_PIN 5  // P3 on PCF8574

// Buzzer timing
#define BEEP_DURATION 100  // Short 100ms beep

// Stuck detection configuration
#define STUCK_INIT_TIME 1000    // Wait 1 second before enabling stuck detection
#define STUCK_HISTORY_TIME 2000 // Keep 2 seconds of history for stuck detection
#define LOOP_DELAY 50          // Main loop delay in milliseconds

// Sensor configuration
#define SENSOR_INVALID_READING 8190

// Sensor timing (in milliseconds)
#define SENSOR_READ_INTERVAL 50    // Minimum time between sensor reads
#define SENSOR_READ_TIMEOUT 100    // Maximum age of sensor readings

// Speed control configuration
#define MIN_FORWARD_SPEED 50     // Minimum forward speed (%)
#define MAX_FORWARD_SPEED 100    // Maximum forward speed (%)
#define MIN_SPEED_DISTANCE 300   // Distance at which to use minimum speed (mm)
#define MAX_SPEED_DISTANCE 1000  // Distance at which to use maximum speed (mm)

// Debug configuration
#define ENABLE_DEBUG_LOGS true    // Set to false to disable debug messages
#define LOG_LEVEL LogLevel::Debug  // Enable debug logs

// Filter context examples:
// Filter nothing (show all logs):
// #define FILTERED_CONTEXTS 0
#define FILTERED_CONTEXTS (1 << static_cast<int>(LogContext::Sensor))

// Current setting - filter navigation and motor messages:
// #define FILTERED_CONTEXTS ((1 << static_cast<int>(LogContext::Navigation)) | \
//                          (1 << static_cast<int>(LogContext::Motor)))

// Filter only navigation:
// #define FILTERED_CONTEXTS (1 << static_cast<int>(LogContext::Navigation))

// Filter navigation, motor and sensor messages:
// #define FILTERED_CONTEXTS ((1 << static_cast<int>(LogContext::Navigation)) | \
//                          (1 << static_cast<int>(LogContext::Motor)) | \
//                          (1 << static_cast<int>(LogContext::Sensor)))
