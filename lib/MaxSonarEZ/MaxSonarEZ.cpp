#include "MaxSonarEZ.h"

MaxSonarEZ::MaxSonarEZ(
    SoftwareSerial *ser,
    uint8_t enable_pin,
    uint32_t timeout) : ser(ser),
                        enable_pin(enable_pin),
                        timeout(timeout)
{
}

MaxSonarEZ::~MaxSonarEZ()
{
}

void MaxSonarEZ::begin()
{
    this->ser->begin(9600);
    pinMode(this->enable_pin, OUTPUT);
    digitalWrite(this->enable_pin, HIGH);
}

bool MaxSonarEZ::available()
{
    return (bool)(this->ser->available() > 0);
}

int MaxSonarEZ::read()
{
    static const uint8_t ascii_offset = 48;
    static uint8_t buf[6];
    uint32_t start_ts = millis();

    while (millis() - start_ts < this->timeout)
    {
        if (this->ser->available() > 0)
        {
            buf[0] = this->ser->read() & 0xFF;
            if ((char)buf[0] != 'R')
                continue;

            size_t n = this->ser->readBytes(&buf[1], 5);
            if (n != 5 || buf[5] != '\r')
                continue;

            while (this->ser->available() > 0)
                this->ser->read();

            return ((buf[1] - ascii_offset) * 1000) +
                   ((buf[2] - ascii_offset) * 100) +
                   ((buf[3] - ascii_offset) * 10) +
                   (buf[4] - ascii_offset);
        }
    }
    return MAXSONAREZ_READING_ERROR;
}