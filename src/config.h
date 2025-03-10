#pragma once

#include <Arduino.h>

// Motor pins
#define LEFT_MOTOR_IN1 GPIO_NUM_16
#define LEFT_MOTOR_IN2 GPIO_NUM_18
#define RIGHT_MOTOR_IN1 GPIO_NUM_4
#define RIGHT_MOTOR_IN2 GPIO_NUM_17

// Motor control pins
#define MOTOR_SLEEP GPIO_NUM_5
#define MOTOR_FLT GPIO_NUM_19

// Encoder pins
#define ENCODER_LEFT GPIO_NUM_15
#define ENCODER_RIGHT GPIO_NUM_21

// HC-SR04 pins
#define LEFT_ECHO_PIN GPIO_NUM_13
#define LEFT_TRIG_PIN GPIO_NUM_14
#define FRONT_ECHO_PIN GPIO_NUM_25
#define FRONT_TRIG_PIN GPIO_NUM_27
#define RIGHT_ECHO_PIN GPIO_NUM_32
#define RIGHT_TRIG_PIN GPIO_NUM_33

// Add this for DistanceSensors
#define NUM_SENSORS 3

// Sensor timing (in milliseconds)
#define SENSOR_READ_INTERVAL 50    // Minimum time between sensor reads
#define SENSOR_READ_TIMEOUT 100    // Maximum age of sensor readings

// Speed control configuration
#define MIN_SPEED_DISTANCE 300   // Distance at which to use minimum speed (mm)
#define MAX_SPEED_DISTANCE 1000  // Distance at which to use maximum speed (mm)

// Debug configuration
#define ENABLE_DEBUG_LOGS true    // Set to false to disable debug messages
#define LOG_LEVEL LogLevel::Info  // Enable debug logs

// Navigation constants
#define MIN_TURN_DISTANCE 150    // 15cm - will cause full turn
#define TURN_EXPONENT 0.005     // Decay rate for exponential function
#define FRONT_MULTIPLIER_MAX 3.0 // Maximum turn multiplier from front sensor
#define MAX_TURN_DISTANCE 500    // Only used for front sensor
#define BACKUP_MIN_TIME 500      // Minimum backup time in ms
#define BACKUP_MAX_TIME 1500     // Maximum backup time in ms
#define TURN_MIN_TIME 300        // Minimum turn time after backup
#define TURN_MAX_TIME 800        // Maximum turn time after backup

// Define FILTERED_CONTEXTS
#define FILTERED_CONTEXTS 0

// Define LED_BUILTIN if not defined
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

// Motor control configuration
#define MOTOR_UPDATE_INTERVAL 50    // Motor control update interval (ms)
#define MOTOR_PWM_RESOLUTION 8      // PWM resolution bits
#define MOTOR_MAX_RPM 200.0f       
#define MOTOR_PULSES_PER_REV 20.0f // Encoder pulses per revolution

// PID control constants
// Left motor
#define LEFT_MOTOR_PID_KP 1.14f
#define LEFT_MOTOR_PID_KI 0.2f
#define LEFT_MOTOR_PID_KD 0.14f

// Right motor
#define RIGHT_MOTOR_PID_KP 1.39f
#define RIGHT_MOTOR_PID_KI 0.21f
#define RIGHT_MOTOR_PID_KD 0.3f
