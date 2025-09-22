#ifndef __APP_H
#define __APP_H

#include <Arduino.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
#include <Adafruit_ADXL345_U.h>
#include "MaxSonarEZ.h"
#include "UserInterface.h"
#include "config.h"

class App
{
public:
    enum AppState : uint8_t
    {
        SLEEP,
        SELECT,
        DISTANCE,
        ANGLE,
        POWER
    };

    App();
    ~App();
    App(App &other) = delete;
    App &operator=(App &other) = delete;
    void setup();
    void loop();

private:
    MaxSonarEZ *sonar;
    Adafruit_ADXL345_Unified adxl;
    UserInterface *ui;
    AppState app_st;
    UserInterface::ButtonInput btn_in;
    int16_t x_val, y_val, z_val;
    int dist;

    void logInfo(const char *tag, String msg);
    void logWarn(const char *tag, String msg);
    void logError(const char *tag, String msg);
    void logDebug(const char *tag, String msg);
    void restart();
    void sleepCount();

    void updateAccel();
    void updateDistance();

    void onSelect();
    void onDistance();
    void onAngle();
    void onPower();
};

#endif