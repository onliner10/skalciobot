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
#define SENSOR_READ_TIMEOUT 8      // 8ms timeout for 1.3m max range
#define SENSOR_CYCLE_TIME 10      // Minimum time between sensor triggers (ms)
#define MAX_SENSOR_DISTANCE 1300   // Maximum detection range in mm
#define MIN_FRONT_STEERING 0.3f    // Minimum steering correction when obstacle in front

// Speed control
#define SPEED_THRESHOLD_MM 500     // Midpoint for speed transition sigmoid
#define SPEED_SIGMOID_SLOPE 0.12f  // Slope parameter for sigmoid function (higher = sharper transition)
// Wolfram Alpha expression: plot 40 + 60/(1 + exp(-0.01*(x-600))) for x=0 to 1300
#define MIN_SPEED_PERCENT 40       // Minimum speed when close to obstacles
#define MAX_SPEED_PERCENT 100      // Maximum speed when path is clear

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
#define STEERING_PID_INTERVAL 10   // Reduced from 10ms for faster updates
#define STEERING_INTEGRAL_LIMIT 1.0f // Limit for integral term

// Motor calibration
#define MOTOR_CALIBRATION_TIME 2000    // Time to run calibration (ms)
#define MOTOR_CALIBRATION_SPEED 50     // Speed percent to use for calibration
#define DEFAULT_LEFT_MOTOR_SCALE 0.79f  // Default scaling factor
#define DEFAULT_RIGHT_MOTOR_SCALE 1.0f // Default scaling factor

// Stuck detector configuration
#define STUCK_HISTORY_SIZE 40      // 1 second of readings at 50ms intervals
#define STUCK_MIN_STDDEV_LOW_SPEED 15.0f   // Lower threshold for high speeds
#define STUCK_MIN_STDDEV_HIGH_SPEED 45.0f  // Higher threshold for low speeds
#define STUCK_ENCODER_TIME 500     // Time in ms before considering encoder stuck
#define STUCK_UPDATE_INTERVAL 50   // Update interval in ms
#define STUCK_BACKUP_MIN_TIME 1000  // Minimum backup time
#define STUCK_BACKUP_MAX_TIME 2000  // Maximum backup time
#define STUCK_BACKUP_SPEED 60       // Increase backup speed for more reliable movement
#define STUCK_BACKUP_COOLDOWN 3000  // Wait at least 3 seconds before triggering another backup

// Auto mode configuration
#define AUTO_SWITCH_TIMEOUT 30000  // Time in ms to automatically switch to auto mode (30 seconds)
