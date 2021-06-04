#ifndef SoundSensor_h
#define SoundSensor_h

#include "Arduino.h"

class SoundSensor {
  private:
    uint8_t _analogPin;

  public:
    SoundSensor();
    SoundSensor(uint8_t analogPin);
    void setAnalogPin(uint8_t analogPin);
    uint16_t readAnalogNoise();
};

#endif
