#include "SoundSensor.h"

/* No-arg constructor */
SoundSensor::SoundSensor() {}

/**
 * Constructor
 * Creates a SoundSensor object and sets its analog pin.
 * @gpio: analog pin
 */
SoundSensor::SoundSensor(uint8_t analogPin) {
  _analogPin = analogPin;
}

/**
 * Sets the analog pin used to read the analog values from this sensor.
 * @param analogPin: Arduino ADC pin connected to A0
 */
void SoundSensor::setAnalogPin(uint8_t analogPin) {
  _analogPin = analogPin;
}

/**
 * Reads the analog noise value on the ADC pin connected to the A0 output.
 * @return: analog noise value
 */
uint16_t SoundSensor::readAnalogNoise() {
  return analogRead(_analogPin);
}
