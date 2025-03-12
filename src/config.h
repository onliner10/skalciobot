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
#define SENSOR_READ_INTERVAL 33    // Time between measurement groups
#define SENSOR_READ_TIMEOUT 15     // Max time to wait for echo (15ms = ~250cm)
#define MAX_SENSOR_DISTANCE 1300     
#define MIN_FRONT_STEERING 0.3f

// Speed control
#define MIN_SPEED_PERCENT 60      
#define MAX_SPEED_PERCENT 50     // Back to full speed

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
#define MOTOR_UPDATE_INTERVAL 5    // Increased update frequency (was 50)
#define MOTOR_PWM_RESOLUTION 10     // Increased from 8 to 10 bits for finer control

// Motor PWM control
#define MOTOR_PWM_MIN_UPDATE_INTERVAL 10     // Reduced delay between PWM updates
#define MOTOR_PWM_MIN_CHANGE 1              // Reduced to allow finer adjustments

// Steering PID configuration
#define STEERING_PID_KP 1.0f    // Full opposing motor speeds at maximum error
#define STEERING_PID_KI 0.05f    // Start without integral term
#define STEERING_PID_KD 0.2f    // Start without derivative term
#define STEERING_PID_INTERVAL 5   // Reduced from 10ms for faster updates
#define STEERING_INTEGRAL_LIMIT 1.0f // Limit for integral term

// Motor calibration
#define MOTOR_CALIBRATION_TIME 2000    // Time to run calibration (ms)
#define MOTOR_CALIBRATION_SPEED 50     // Speed percent to use for calibration
#define DEFAULT_LEFT_MOTOR_SCALE 0.79f  // Default scaling factor
#define DEFAULT_RIGHT_MOTOR_SCALE 1.0f // Default scaling factor
