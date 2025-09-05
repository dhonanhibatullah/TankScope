#include "app.h"

App::App() : app_st(App::SELECT)
{
    this->sonar_ser = new SoftwareSerial(
        SONAR_RX_PIN,
        SONAR_TX_PIN);
    this->sonar = new MaxSonarEZ(
        this->sonar_ser,
        SONAR_EN_PIN,
        SONAR_FILTER_LEN);
    this->ui = new UserInterface(
        BUTTON_PIN,
        BATTERY_LEVEL_PIN);
}

App::~App()
{
    delete this->sonar_ser;
    delete this->sonar;
    delete this->ui;
}

void App::setup()
{
    uint8_t trial = 0;

    Serial.begin(9600);
    pinMode(LASER_PIN, OUTPUT);
    pinMode(BATTERY_CHG_PIN, INPUT);
    digitalWrite(LASER_PIN, LOW);
    delay(100);

    // this->logInfo("sonar", "Initiating sonar...");
    // this->sonar_ser->begin(9600);
    // delay(100);
    // while (1)
    // {
    //     if (this->sonar->connection())
    //         break;
    //     trial += 1;
    //     delay(150);

    //     if (trial == 3)
    //     {
    //         this->logError("sonar", "Failed to initiate sonar, resetting...");
    //         this->restart();
    //     }
    // }
    // this->logInfo("sonar", "Sonar initiated successfully");

    this->logInfo("accel", "Initiating accel...");
    this->adxl.powerOn();
    this->adxl.setRangeSetting(2);
    this->applyCalibAccel();
    this->logInfo("accel", "Accel initiated");

    this->logInfo("ui", "Initiating interface...");
    if (!this->ui->begin())
    {
        this->logError("ui", "Failed to initiate interface, resetting...");
        this->restart();
    }
    this->logInfo("ui", "Interface initiated");
}

void App::loop()
{
    this->btn_in = this->ui->getButtonInput();
    switch (this->app_st)
    {
    case App::MEASURE:
        this->onMeasure();
        break;

    case App::POWER:
        this->onPower();
        break;

    case App::CALIB:
        this->onCalib();
        break;

    case App::SELECT:
        this->onSelect();
        break;
    }
}

void App::logInfo(const char *tag, String msg)
{
#ifdef ENABLE_LOG_INFO
    unsigned long ts = millis();
    unsigned int loglen = 32 + strlen(tag) + msg.length();
    char log[loglen];

    sprintf(log, "%7lu.%03lu [INFO ][%s] %s", ts / 1000, ts % 1000, tag, msg.c_str());
    Serial.println(log);
#endif
}

void App::logWarn(const char *tag, String msg)
{
#ifdef ENABLE_LOG_WARN
    unsigned long ts = millis();
    unsigned int loglen = 32 + strlen(tag) + msg.length();
    char log[loglen];

    sprintf(log, "%7lu.%03lu [WARN ][%s] %s", ts / 1000, ts % 1000, tag, msg.c_str());
    Serial.println(log);
#endif
}

void App::logError(const char *tag, String msg)
{
#ifdef ENABLE_LOG_ERROR
    unsigned long ts = millis();
    unsigned int loglen = 32 + strlen(tag) + msg.length();
    char log[loglen];

    sprintf(log, "%7lu.%03lu [ERROR][%s] %s", ts / 1000, ts % 1000, tag, msg.c_str());
    Serial.println(log);
#endif
}

void App::logDebug(const char *tag, String msg)
{
#ifdef ENABLE_LOG_DEBUG
    unsigned long ts = millis();
    unsigned int loglen = 32 + strlen(tag) + msg.length();
    char log[loglen];

    sprintf(log, "%7lu.%03lu [DEBUG][%s] %s", ts / 1000, ts % 1000, tag, msg.c_str());
    Serial.println(log);
#endif
}

void App::restart()
{
    wdt_enable(WDTO_15MS);
    while (1)
    {
    }
}

void App::applyCalibAccel()
{
    if (EEPROM.read(0) != 0xDD)
    {
        this->logWarn("accel", "Accel is not calibrated yet");
        return;
    }

    int y_off, z_off;
    EEPROM.get(2, y_off);
    EEPROM.get(6, z_off);
    this->adxl.setAxisOffset(0, y_off, z_off);

    this->logInfo("accel", "Accel calibration data applied");
}

void App::calibrateAccel(int y_off, int z_off)
{
    EEPROM.update(0, 0xDD);
    EEPROM.put(2, y_off);
    EEPROM.put(6, z_off);
    this->adxl.setAxisOffset(0, y_off, z_off);
}

void App::updateAccel(int &y, int &z)
{
    static int acc_y = 0,
               acc_z = 0;

    int tmp_x, tmp_y, tmp_z;
    this->adxl.readAccel(&tmp_x, &tmp_y, &tmp_z);
    acc_y = ((80 * acc_y) + (20 * tmp_y)) / 100;
    acc_z = ((80 * acc_z) + (20 * tmp_z)) / 100;

    y = acc_y;
    z = acc_z;
}

void App::onMeasure()
{
    static bool captured = false;
    static bool released = false;
    static int dist, y, z;

    if (!captured)
    {
        dist = random(0, 5000);
        this->updateAccel(y, z);
        this->ui->setPage(UserInterface::PAGE_MEASURE, dist, y, z);
    }

    if (!released)
    {
        if (this->btn_in == UserInterface::BUTTON_INPUT_LONG)
            released = true;
        return;
    }

    if (this->btn_in == UserInterface::BUTTON_INPUT_SHORT)
    {
        captured = !captured;
        if (captured)
        {
            this->ui->setPage(UserInterface::PAGE_FLASH);
            delay(100);
            this->ui->setPage(UserInterface::PAGE_MEASURE, dist, y, z);
        }
    }
    else if (this->btn_in == UserInterface::BUTTON_INPUT_PRESSED_LONG)
    {
        released = false;
        this->app_st = App::SELECT;
    }
}

void App::onPower()
{
    // static uint8_t chg_st = LOW;
    static int adc_val = 0;
    static bool released = false;

    // chg_st = digitalRead(BATTERY_CHG_PIN);
    adc_val = analogRead(BATTERY_LEVEL_PIN);
    this->ui->setPage(UserInterface::PAGE_POWER, adc_val);

    if (!released)
    {
        if (this->btn_in == UserInterface::BUTTON_INPUT_LONG)
            released = true;
        return;
    }

    if (this->btn_in == UserInterface::BUTTON_INPUT_PRESSED_LONG)
    {
        released = false;
        this->app_st = App::SELECT;
    }
}

void App::onCalib()
{
    static bool init = true;
    static bool released = false;
    static uint8_t calib_st = 0;
    int y, z;

    if (init)
    {
        init = false;
        this->adxl.setAxisOffset(0, 0, 0);
        this->ui->setPage(UserInterface::PAGE_CALIB_0);
    }
    if (!released)
    {
        if (this->btn_in == UserInterface::BUTTON_INPUT_LONG)
            released = true;
        return;
    }

    switch (calib_st)
    {
    case 1:
    {
        this->updateAccel(y, z);
        this->ui->setPage(UserInterface::PAGE_CALIB_1, y, z);
        break;
    }

    case 2:
        this->ui->setPage(UserInterface::PAGE_CALIB_2);
        break;
    }

    if (this->btn_in == UserInterface::BUTTON_INPUT_SHORT)
    {
        if (calib_st == 1)
            this->calibrateAccel(y, z);

        calib_st = ((calib_st == 2) ? 2 : (calib_st + 1));
    }
    else if (this->btn_in == UserInterface::BUTTON_INPUT_PRESSED_LONG)
    {
        calib_st = 0;
        init = true;
        released = false;
        this->app_st = App::SELECT;
    }
}

void App::onSelect()
{
    static const UserInterface::Page select_pages[] = {
        UserInterface::PAGE_MENU_MEASURE,
        UserInterface::PAGE_MENU_POWER,
        UserInterface::PAGE_MENU_CALIB};
    static bool init = true;
    static bool released = true;
    static uint8_t select_page_idx = 0;

    if (init)
    {
        init = false;
        this->ui->setPage(select_pages[select_page_idx]);
    }
    if (!released)
    {
        if (this->btn_in == UserInterface::BUTTON_INPUT_LONG)
            released = true;
        return;
    }

    if (this->btn_in == UserInterface::BUTTON_INPUT_SHORT)
    {
        select_page_idx = (select_page_idx + 1) % 3;
        this->ui->setPage(select_pages[select_page_idx]);
    }
    else if (this->btn_in == UserInterface::BUTTON_INPUT_PRESSED_LONG)
    {
        init = true;
        released = false;

        switch (select_page_idx)
        {
        case 0:
            this->app_st = App::MEASURE;
            break;

        case 1:
            this->app_st = App::POWER;
            break;

        case 2:
            this->app_st = App::CALIB;
            break;
        }
    }
}