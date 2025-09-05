#include "MaxSonarEZ.h"

MaxSonarEZ::MaxSonarEZ(
    SoftwareSerial *ser,
    uint8_t enable_pin,
    uint8_t filter_buf_len,
    uint32_t timeout) : ser(ser),
                        enable_pin(enable_pin),
                        timeout(timeout),
                        filter_buf_len(filter_buf_len),
                        filter_buf_idx(0)
{
    this->filter_buf = new int[this->filter_buf_len];
    memset(this->filter_buf, 0, this->filter_buf_len);
}

MaxSonarEZ::~MaxSonarEZ()
{
    delete[] this->filter_buf;
}

bool MaxSonarEZ::begin()
{
    pinMode(this->enable_pin, OUTPUT);
    digitalWrite(this->enable_pin, HIGH);
    return (bool)(this->streamRead() != MAXSONAREZ_READING_ERROR);
}

bool MaxSonarEZ::update()
{
    int res = this->streamRead();
    if (res == MAXSONAREZ_READING_ERROR)
        return false;

    this->filter_buf[this->filter_buf_idx] = res;
    this->filter_buf_idx = (this->filter_buf_idx + 1) % this->filter_buf_len;
    return true;
}

int MaxSonarEZ::readRaw()
{
    return this->filter_buf[(this->filter_buf_idx + this->filter_buf_len - 1) % this->filter_buf_len];
}

int MaxSonarEZ::readFiltered()
{
    int dist = 0;

    for (uint8_t i = 0; i < this->filter_buf_len; ++i)
        dist += this->filter_buf[i];
    dist /= (int)this->filter_buf_len;

    return dist;
}

int MaxSonarEZ::streamRead()
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

            return ((buf[1] - ascii_offset) * 1000) +
                   ((buf[2] - ascii_offset) * 100) +
                   ((buf[3] - ascii_offset) * 10) +
                   (buf[4] - ascii_offset);
        }
    }
    return MAXSONAREZ_READING_ERROR;
}