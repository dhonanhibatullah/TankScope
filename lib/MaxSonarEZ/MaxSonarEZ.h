#ifndef __MAX_SONAR_EZ_H
#define __MAX_SONAR_EZ_H

#include <Arduino.h>
#include <SoftwareSerial.h>

#define MAXSONAREZ_DEFAULT_BAUDRATE 9600
#define MAXSONAREZ_DEFAULT_SERIALMODE SERIAL_8N1
#define MAXSONAREZ_MAX_DISTANCE 5000
#define MAXSONAREZ_READING_ERROR -1

class MaxSonarEZ
{
public:
    MaxSonarEZ(SoftwareSerial *ser, uint8_t enable_pin, uint32_t timeout = 150U);
    ~MaxSonarEZ();
    MaxSonarEZ(MaxSonarEZ &other) = delete;
    MaxSonarEZ &operator=(MaxSonarEZ &other) = delete;
    void begin();
    bool available();
    int read();
    void disable();

private:
    SoftwareSerial *ser;
    uint8_t enable_pin;
    uint32_t timeout;
    int *filter_buf;
};

#endif