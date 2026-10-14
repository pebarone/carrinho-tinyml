#pragma once

#include <Arduino.h>
#include "config.h"

namespace ultrasonic {

class Sensor {
 public:
  void begin() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
  }

  float readCm() const {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    const unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);
    if (!duration) return -1.0f;
    const float cm = duration * 0.0343f / 2.0f;
    return isValid(cm) ? cm : -1.0f;
  }

  bool isValid(float cm) const { return cm >= 2.0f && cm <= 400.0f; }
};

}  // namespace ultrasonic
