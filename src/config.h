#pragma once

#include <Arduino.h>

// Motor pins
#define LEFT_MOTOR_IN1 GPIO_NUM_18
#define LEFT_MOTOR_IN2 GPIO_NUM_16
#define RIGHT_MOTOR_IN1 GPIO_NUM_17
#define RIGHT_MOTOR_IN2 GPIO_NUM_4

// Motor control pins
#define MOTOR_SLEEP GPIO_NUM_5
#define MOTOR_FLT GPIO_NUM_19

// Encoder pins
#define ENCODER_LEFT GPIO_NUM_2
#define ENCODER_RIGHT GPIO_NUM_15

// Distance thresholds (in millimeters)
#define FRONT_OBSTACLE_THRESHOLD 200
#define CLEARANCE_TOLERANCE 100    // 10cm tolerance for straight movement
#define DISTANCE_ROUNDING 50       // Round distances to nearest 5cm

// Movement timing (in milliseconds)
#define BACKUP_DURATION 1000  // Time to spend backing up

// HC-SR04 pins
#define LEFT_ECHO_PIN GPIO_NUM_13
#define LEFT_TRIG_PIN GPIO_NUM_14
#define FRONT_ECHO_PIN GPIO_NUM_25
#define FRONT_TRIG_PIN GPIO_NUM_27
#define RIGHT_ECHO_PIN GPIO_NUM_34
#define RIGHT_TRIG_PIN GPIO_NUM_35

// Add this for DistanceSensors
#define NUM_SENSORS 3

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

// Navigation constants
#define MIN_TURN_DISTANCE 150    // 15cm - will cause full turn
#define TURN_EXPONENT 0.005     // Decay rate for exponential function
#define FRONT_MULTIPLIER_MAX 3.0 // Maximum turn multiplier from front sensor
#define MAX_TURN_DISTANCE 500    // Only used for front sensor
#define BACKUP_MIN_TIME 500      // Minimum backup time in ms
#define BACKUP_MAX_TIME 1500     // Maximum backup time in ms
#define TURN_MIN_TIME 300        // Minimum turn time after backup
#define TURN_MAX_TIME 800        // Maximum turn time after backup
#define STUCK_RPM_THRESHOLD 5    // RPM threshold to consider motor stuck

// Motor PID Configuration
#define MOTOR_PID_KP 0.1      // Start with low value
#define MOTOR_PID_KI 0.01     // Kp/10
#define MOTOR_PID_KD 0.001    // Kp/100
#define PID_SAMPLE_TIME 20    // 20ms (50Hz)
#define ENCODER_PPR 12        // Pulses per revolution
#define MIN_RPM 30           // Minimum RPM for reliable operation
#define MAX_RPM 200          // Maximum RPM expected
#define RPM_CALC_INTERVAL 100 // How often to calculate RPM (ms)

// Define FILTERED_CONTEXTS
#define FILTERED_CONTEXTS 0

// Define LED_BUILTIN if not defined
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif
