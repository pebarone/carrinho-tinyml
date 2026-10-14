#pragma once

#include <Arduino.h>
#include "config.h"
#include "policy.h"

namespace motors {

class MotorDriver {
 public:
  bool begin() {
    const int pins[] = {ENA_PIN, ENB_PIN, IN1_PIN, IN2_PIN, IN3_PIN, IN4_PIN};
    for (int pin : pins) {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }

#if !COLLECTION_MODE
    leftReady_ = ledcAttach(ENA_PIN, 1000, 8);
    rightReady_ = ledcAttach(ENB_PIN, 1000, 8);
    pwmReady_ = leftReady_ && rightReady_;
#endif
    stop();
    return pwmReady_;
  }

  void stop() {
    if (leftReady_) ledcWrite(ENA_PIN, 0);
    if (rightReady_) ledcWrite(ENB_PIN, 0);
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    digitalWrite(IN3_PIN, LOW);
    digitalWrite(IN4_PIN, LOW);
  }

  void drive(Action action) {
    if (action == Action::STOP || !pwmReady_) {
      stop();
      return;
    }
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);
    digitalWrite(IN3_PIN, HIGH);
    digitalWrite(IN4_PIN, LOW);
    const int duty = action == Action::SLOW ? SLOW_PWM : CRUISE_PWM;
    ledcWrite(ENA_PIN, duty);
    ledcWrite(ENB_PIN, duty);
  }

  bool ready() const { return pwmReady_; }

 private:
  bool leftReady_ = false;
  bool rightReady_ = false;
  bool pwmReady_ = false;
};

}  // namespace motors
