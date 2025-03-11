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
#define RIGHT_ECHO_PIN GPIO_NUM_33
#define RIGHT_TRIG_PIN GPIO_NUM_32

// Basic configuration
#define NUM_SENSORS 3
#define SENSOR_READ_INTERVAL 50
#define SENSOR_READ_TIMEOUT 100
#define MAX_SENSOR_DISTANCE 2000     
#define FRONT_AMPLIFICATION 1.35f
#define FRONT_MIN_DISTANCE 150       // Absolute minimum distance before max amplification
#define FRONT_AGGRESSIVE_DISTANCE 300 // Distance where aggressive turning starts
#define FRONT_START_DISTANCE 800     // Distance where any turning starts
#define MIN_FRONT_STEERING 0.3f

// Speed control
#define MIN_SPEED_PERCENT 50      
#define MAX_SPEED_PERCENT 100     // Back to full speed

// Debug configuration
#define ENABLE_DEBUG_LOGS true    // Set to false to disable debug messages
#define LOG_LEVEL LogLevel::Info  // Enable debug logs

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

// Motor PWM control
#define MOTOR_PWM_MIN_UPDATE_INTERVAL 5     // Reduced from 20ms to 5ms
#define MOTOR_PWM_MIN_CHANGE 2              // Reduced from 5 to 2
#define MOTOR_PWM_SMOOTHING 0.1f           // Output smoothing factor (0-1)

// Steering PID configuration
#define STEERING_PID_KP 1.0f    // Full opposing motor speeds at maximum error
#define STEERING_PID_KI 0.05f    // Start without integral term
#define STEERING_PID_KD 0.2f    // Start without derivative term
#define STEERING_PID_INTERVAL 10 // PID update interval in milliseconds
#define STEERING_INTEGRAL_LIMIT 1.0f // Limit for integral term

// Motor calibration
#define MOTOR_CALIBRATION_TIME 2000    // Time to run calibration (ms)
#define MOTOR_CALIBRATION_SPEED 50     // Speed percent to use for calibration
#define DEFAULT_LEFT_MOTOR_SCALE 0.79f  // Default scaling factor
#define DEFAULT_RIGHT_MOTOR_SCALE 1.0f // Default scaling factor
