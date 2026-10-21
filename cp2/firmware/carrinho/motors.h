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

  void drive(Action action, int duty = -1) {
    if (action == Action::STOP || !pwmReady_) {
      stop();
      return;
    }
    ledcWrite(ENA_PIN, 0);
    ledcWrite(ENB_PIN, 0);
    const bool leftForward = action != Action::REVERSE;
    const bool rightForward = action != Action::REVERSE && action != Action::TURN_RIGHT;
    digitalWrite(IN1_PIN, leftForward ? HIGH : LOW);
    digitalWrite(IN2_PIN, leftForward ? LOW : HIGH);
    // Compensa OUT3/OUT4, conforme correcao solicitada no teste de bancada.
    digitalWrite(IN3_PIN, rightForward ? LOW : HIGH);
    digitalWrite(IN4_PIN, rightForward ? HIGH : LOW);
    const int requestedDuty = duty < 0 ? (action == Action::SLOW ? SLOW_PWM : CRUISE_PWM) : duty;
    const int appliedDuty = constrain(requestedDuty, 0, 255);
    ledcWrite(ENA_PIN, appliedDuty);
    ledcWrite(ENB_PIN, appliedDuty);
  }

  bool ready() const { return pwmReady_; }

 private:
  bool leftReady_ = false;
  bool rightReady_ = false;
  bool pwmReady_ = false;
};

}  // namespace motors
