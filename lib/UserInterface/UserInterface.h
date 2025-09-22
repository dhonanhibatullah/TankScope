#ifndef __USER_INTERFACE_H
#define __USER_INTERFACE_H

#include <stdarg.h>
#include <math.h>
#include <Arduino.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "LogoBmp.h"

#define CALIB_

class UserInterface
{
public:
    enum ButtonInput : uint8_t
    {
        BUTTON_INPUT_NONE,
        BUTTON_INPUT_PRESSED,
        BUTTON_INPUT_PRESSED_LONG,
        BUTTON_INPUT_SHORT,
        BUTTON_INPUT_LONG
    };

    enum Page : uint8_t
    {
        PAGE_SPLASH_SCREEN,
        PAGE_MENU_DISTANCE,
        PAGE_MENU_ANGLE,
        PAGE_MENU_POWER,
        PAGE_DISTANCE,
        PAGE_ANGLE,
        PAGE_FLASH,
        PAGE_POWER
    };

    static UserInterface *inst;
    UserInterface(uint8_t button_pin, uint8_t battery_pin);
    ~UserInterface();
    UserInterface(UserInterface &other) = delete;
    UserInterface &operator=(UserInterface &other) = delete;
    bool begin();
    ButtonInput getButtonInput();
    void setPage(Page page, ...);
    void sleep();
    void isrCallback();

private:
    Adafruit_SSD1306 *display;
    volatile ButtonInput button_in;
    volatile uint8_t sleeping;
    volatile uint8_t button_pin;
    volatile uint8_t battery_pin;
    bool menu_opened;
    int calcCircleCoor(int q);
    float calcAngleDeg(int q, float n_max, float p_max);
};

#endif