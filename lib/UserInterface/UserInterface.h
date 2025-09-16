#ifndef __USER_INTERFACE_H
#define __USER_INTERFACE_H

#include <stdarg.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "PageBmp.h"

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
        PAGE_MENU_MEASURE,
        PAGE_MENU_POWER,
        PAGE_MENU_CALIB,
        PAGE_MEASURE,
        PAGE_FLASH,
        PAGE_POWER,
        PAGE_CALIB_0,
        PAGE_CALIB_1,
        PAGE_CALIB_2
    };

    static UserInterface *inst;
    UserInterface(uint8_t button_pin, uint8_t battery_pin);
    ~UserInterface();
    UserInterface(UserInterface &other) = delete;
    UserInterface &operator=(UserInterface &other) = delete;
    bool begin();
    ButtonInput getButtonInput();
    void setPage(Page page, ...);
    void isrCallback();

private:
    Adafruit_SSD1306 *display;
    volatile ButtonInput button_in;
    volatile uint8_t button_pin;
    volatile uint8_t battery_pin;
    bool menu_opened;
};

#endif