#include "app.h"

App::App() : app_st(App::SELECT)
{
    this->sonar = new MaxSonarEZ(
        new SoftwareSerial(
            SONAR_RX_PIN,
            SONAR_TX_PIN),
        SONAR_EN_PIN,
        SONAR_FILTER_LEN);
    this->ui = new UserInterface(
        BUTTON_PIN,
        BATTERY_LEVEL_PIN);
}

App::~App()
{
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

    this->logInfo("ui", "Initiating interface...");
    if (!this->ui->begin())
    {
        this->logError("ui", "Failed to initiate interface, resetting...");
        this->restart();
    }
    this->logInfo("ui", "Interface initiated");
    this->ui->setPage(UserInterface::PAGE_SPLASH_SCREEN);

    this->logInfo("sonar", "Initiating sonar...");
    this->sonar->begin();
    delay(100);
    while (this->sonar->read() == MAXSONAREZ_READING_ERROR)
    {
        trial += 1;
        delay(150);

        if (trial == 3)
        {
            this->logError("sonar", "Failed to initiate sonar, resetting...");
            this->restart();
        }
    }
    this->logInfo("sonar", "Sonar initiated successfully");
    for (uint16_t i = 0; i < 70; ++i)
        this->sonar->read();

    this->logInfo("accel", "Initiating accel...");
    if (!this->adxl.begin())
    {
        this->logError("accel", "Failed to initiate accel, resetting...");
        this->restart();
    }
    this->adxl.setRange(ADXL345_RANGE_2_G);
    this->logInfo("accel", "Accel initiated");
}

void App::loop()
{
    this->btn_in = this->ui->getButtonInput();
    switch (this->app_st)
    {
    case App::DISTANCE:
        this->onDistance();
        break;

    case App::ANGLE:
        this->onAngle();
        break;

    case App::POWER:
        this->onPower();
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

void App::updateAccel()
{
    this->x_val = ((80 * this->x_val) + (20 * this->adxl.getX())) / 100;
    this->y_val = ((80 * this->y_val) + (20 * this->adxl.getY())) / 100;
    this->z_val = ((80 * this->z_val) + (20 * this->adxl.getZ())) / 100;
}

void App::updateDistance()
{
    if (this->sonar->available())
    {
        int tmp = this->sonar->read();
        if (tmp != MAXSONAREZ_READING_ERROR)
            this->dist = tmp;
    }
}

void App::onDistance()
{
    static bool laser_on = false;
    static bool captured = false;
    static bool released = false;

    if (!laser_on)
    {
        digitalWrite(LASER_PIN, HIGH);
        laser_on = true;
    }

    if (!captured)
    {
        this->updateDistance();
        this->updateAccel();
        this->ui->setPage(UserInterface::PAGE_DISTANCE, this->dist, this->y_val, this->z_val);
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
            this->ui->setPage(UserInterface::PAGE_DISTANCE, this->dist, this->y_val, this->z_val);
        }
    }
    else if (this->btn_in == UserInterface::BUTTON_INPUT_PRESSED_LONG)
    {
        digitalWrite(LASER_PIN, LOW);
        captured = false;
        laser_on = false;
        released = false;
        this->app_st = App::SELECT;
    }
}

void App::onAngle()
{
    static bool captured = false;
    static bool released = false;

    if (!captured)
    {
        this->updateAccel();
        this->ui->setPage(UserInterface::PAGE_ANGLE, this->x_val, this->y_val);
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
            this->ui->setPage(UserInterface::PAGE_ANGLE, this->x_val, this->y_val);
        }
    }
    else if (this->btn_in == UserInterface::BUTTON_INPUT_PRESSED_LONG)
    {
        captured = false;
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

void App::onSelect()
{
    static const UserInterface::Page select_pages[] = {
        UserInterface::PAGE_MENU_DISTANCE,
        UserInterface::PAGE_MENU_ANGLE,
        UserInterface::PAGE_MENU_POWER};
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
        select_page_idx = (select_page_idx + 1) % sizeof(select_pages);
        this->ui->setPage(select_pages[select_page_idx]);
    }
    else if (this->btn_in == UserInterface::BUTTON_INPUT_PRESSED_LONG)
    {
        init = true;
        released = false;

        switch (select_page_idx)
        {
        case 0:
            this->app_st = App::DISTANCE;
            break;

        case 1:
            this->app_st = App::ANGLE;
            break;

        case 2:
            this->app_st = App::POWER;
            break;
        }
    }
}