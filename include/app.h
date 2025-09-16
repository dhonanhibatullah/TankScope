#ifndef __APP_H
#define __APP_H

#include <Arduino.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <SparkFun_ADXL345.h>
#include "MaxSonarEZ.h"
#include "UserInterface.h"
#include "config.h"

class App
{
public:
    enum AppState : uint8_t
    {
        SELECT,
        MEASURE,
        POWER,
        CALIB
    };

    App();
    ~App();
    App(App &other) = delete;
    App &operator=(App &other) = delete;
    void setup();
    void loop();

private:
    MaxSonarEZ *sonar;
    ADXL345 adxl;
    UserInterface *ui;
    AppState app_st;
    UserInterface::ButtonInput btn_in;
    void logInfo(const char *tag, String msg);
    void logWarn(const char *tag, String msg);
    void logError(const char *tag, String msg);
    void logDebug(const char *tag, String msg);
    void restart();

    void applyCalibAccel();
    void calibrateAccel(int y_off, int z_off);
    void updateAccel(int &y, int &z);

    void onSelect();
    void onMeasure();
    void onPower();
    void onCalib();
};

#endif