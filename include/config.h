#ifndef __CONFIG_H
#define __CONFIG_H

#include <Arduino.h>

// #define ENABLE_LOG_INFO
// #define ENABLE_LOG_WARN
// #define ENABLE_LOG_ERROR
// #define ENABLE_LOG_DEBUG
#define SLEEP_TIME 30000
#define ACTIVE_SLEEP_TIME 300000

#define SONAR_TX_PIN A0
#define SONAR_RX_PIN A2
#define SONAR_EN_PIN A3
#define SONAR_FILTER_LEN 8
#define BUTTON_PIN 2
#define LASER_PIN 9
#define BATTERY_LEVEL_PIN A1
#define BATTERY_CHG_PIN 3

#endif