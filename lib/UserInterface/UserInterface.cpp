#include "UserInterface.h"

UserInterface *UserInterface::inst = nullptr;

UserInterface::UserInterface(
    uint8_t button_pin,
    uint8_t battery_pin) : display(nullptr),
                           button_in(UserInterface::BUTTON_INPUT_NONE),
                           button_pin(button_pin),
                           battery_pin(battery_pin),
                           menu_opened(false)
{
}

UserInterface::~UserInterface()
{
    TIMSK1 &= ~(1 << OCIE1A);
    delete this->display;
    UserInterface::inst = nullptr;
}

bool UserInterface::begin()
{
    UserInterface::inst = this;

    pinMode(this->button_pin, INPUT);
    pinMode(this->battery_pin, INPUT);

    noInterrupts();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 39999;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS11);
    TIMSK1 |= (1 << OCIE1A);
    interrupts();

    this->display = new Adafruit_SSD1306(128, 32, &Wire, -1);
    if (!this->display->begin(SSD1306_SWITCHCAPVCC, 0x3C))
        return false;
    this->display->setTextColor(SSD1306_WHITE);
    this->display->clearDisplay();
    this->display->display();
    this->display->setRotation(3);

    return true;
}

UserInterface::ButtonInput UserInterface::getButtonInput()
{
    if (this->button_in == UserInterface::BUTTON_INPUT_PRESSED)
        return UserInterface::BUTTON_INPUT_PRESSED;

    else if (this->button_in == UserInterface::BUTTON_INPUT_PRESSED_LONG)
        return UserInterface::BUTTON_INPUT_PRESSED_LONG;

    UserInterface::ButtonInput ret = this->button_in;
    this->button_in = UserInterface::BUTTON_INPUT_NONE;
    return ret;
}

void UserInterface::setPage(UserInterface::Page page, ...)
{
    this->display->clearDisplay();

    switch (page)
    {
    case UserInterface::PAGE_SPLASH_SCREEN:
        this->display->setCursor(12, 12);
        this->display->setTextSize(2);
        this->display->print("TankScope");
        break;

    case UserInterface::PAGE_MENU_MEASURE:
        this->display->drawBitmap(0, 0, measure_menu_bmp, 123, 32, SSD1306_WHITE);
        break;

    case UserInterface::PAGE_MENU_POWER:
        this->display->drawBitmap(0, 0, power_menu_bmp, 123, 32, SSD1306_WHITE);
        break;

    case UserInterface::PAGE_MENU_CALIB:
        this->display->drawBitmap(0, 0, calib_menu_bmp, 123, 32, SSD1306_WHITE);
        break;

    case UserInterface::PAGE_MEASURE:
    {
        va_list args;
        va_start(args, page);
        int dist = va_arg(args, int);
        int y = va_arg(args, int);
        int z = va_arg(args, int);
        va_end(args);

        this->display->setCursor(0, 1);
        this->display->setTextSize(1);
        this->display->print("Jarak:");

        this->display->setCursor(0, 15);
        this->display->setTextSize(2);
        this->display->print(dist / 1000);

        char dp[7];
        sprintf(dp, ".%03d m", dist % 1000);
        this->display->setCursor(11, 23);
        this->display->setTextSize(1);
        this->display->print(dp);

        this->display->drawLine(52, 0, 52, 32, SSD1306_WHITE);

        this->display->setCursor(59, 4);
        this->display->print("y: ");
        this->display->drawRect(72, 2, 56, 13, SSD1306_WHITE);
        if (y > 0)
        {
            y = (y > 150) ? 150 : y;
            int w = (y * 28) / 150;
            this->display->fillRect(100, 2, w, 13, SSD1306_WHITE);
        }
        else if (y < 0)
        {
            y = abs(y);
            y = (y > 150) ? 150 : y;
            int w = (y * 28) / 150;
            this->display->fillRect(100 - w, 2, w, 13, SSD1306_WHITE);
        }

        this->display->setCursor(60, 20);
        this->display->print("z: ");
        this->display->drawRect(72, 17, 56, 13, SSD1306_WHITE);
        if (z > 0)
        {
            z = (z > 150) ? 150 : z;
            int w = (z * 28) / 150;
            this->display->fillRect(100, 17, w, 13, SSD1306_WHITE);
        }
        else if (z < 0)
        {
            z = abs(z);
            z = (z > 150) ? 150 : z;
            int w = (z * 28) / 150;
            this->display->fillRect(100 - w, 17, w, 13, SSD1306_WHITE);
        }
        break;
    }

    case UserInterface::PAGE_FLASH:
        this->display->fillRect(0, 0, 128, 32, SSD1306_WHITE);
        break;

    case UserInterface::PAGE_POWER:
    {
        va_list args;
        va_start(args, page);
        int level = va_arg(args, int);
        va_end(args);

        int percentage = ((level - 850) * 100) / 173;

        if (percentage < 0)
            percentage = 0;

        this->display->fillRect(4, 0, percentage / 2, 32, SSD1306_WHITE);
        this->display->drawRect(4, 0, 50, 32, SSD1306_WHITE);
        this->display->fillRect(54, 8, 5, 16, SSD1306_WHITE);

        this->display->setCursor(70, 10);
        this->display->setTextSize(2);
        this->display->print((String(percentage) + String("%")));

        break;
    }

    case UserInterface::PAGE_CALIB_0:
        this->display->setCursor(0, 4);
        this->display->setTextSize(1);
        this->display->println(F("Arahkan alat ke bawah"));
        this->display->println(F("dengan tegak lurus"));
        this->display->println(F("sempurna."));
        break;

    case UserInterface::PAGE_CALIB_1:
    {
        va_list args;
        va_start(args, page);
        int y = va_arg(args, int);
        int z = va_arg(args, int);
        va_end(args);

        this->display->setCursor(0, 0);
        this->display->setTextSize(1);
        this->display->println(F("> Tekan untuk simpan"));

        this->display->setCursor(0, 12);
        this->display->println("y: " + String(y));
        this->display->println("z: " + String(z));
        break;
    }

    case UserInterface::PAGE_CALIB_2:
        this->display->setCursor(0, 0);
        this->display->setTextSize(1);
        this->display->println(F("Kalibrasi berhasil!"));
        break;
    }

    this->display->display();
}

void UserInterface::isrCallback()
{
    volatile static uint8_t pressed_cnt = 0;
    volatile static uint8_t last_st = LOW;
    volatile static uint8_t st = LOW;

    st = digitalRead(this->button_pin);
    if (st == LOW && last_st == HIGH)
    {
        if (this->button_in == UserInterface::BUTTON_INPUT_PRESSED_LONG)
            this->button_in = UserInterface::BUTTON_INPUT_LONG;
        else if (this->button_in == UserInterface::BUTTON_INPUT_PRESSED)
            this->button_in = UserInterface::BUTTON_INPUT_SHORT;
        pressed_cnt = 0;
    }
    else if (st == HIGH)
    {
        pressed_cnt += 1;
        if (pressed_cnt > 27)
            this->button_in = UserInterface::BUTTON_INPUT_PRESSED_LONG;
        else if (pressed_cnt > 0)
            this->button_in = UserInterface::BUTTON_INPUT_PRESSED;
    }

    last_st = st;
}

ISR(TIMER1_COMPA_vect)
{
    if (UserInterface::inst != nullptr)
        UserInterface::inst->isrCallback();
}