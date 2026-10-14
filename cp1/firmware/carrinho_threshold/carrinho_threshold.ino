#include <Arduino.h>
#include "threshold_policy.h"

// ESP32 DevKit WROOM + HC-SR04 + L298N. Remove the ENA/ENB jumpers.
constexpr int TRIG_PIN = 23;
constexpr int ECHO_PIN = 22;
constexpr int ENA_PIN = 25;
constexpr int IN1_PIN = 26;
constexpr int IN2_PIN = 27;
constexpr int ENB_PIN = 33;
constexpr int IN3_PIN = 32;
constexpr int IN4_PIN = 21;
constexpr unsigned long SAMPLE_MS = 100;
constexpr int SLOW_PWM = 110;
constexpr int CRUISE_PWM = 160;

unsigned long nextSample = 0;
bool armed = false;
bool pwmReady = false;

void stopMotors() {
  if (pwmReady) {
    ledcWrite(ENA_PIN, 0);
    ledcWrite(ENB_PIN, 0);
  }
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
}

void drive(ThresholdAction action) {
  if (!armed || !pwmReady || action == ThresholdAction::STOP) {
    stopMotors();
    return;
  }
  digitalWrite(IN1_PIN, HIGH);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, HIGH);
  digitalWrite(IN4_PIN, LOW);
  const int duty = action == ThresholdAction::SLOW ? SLOW_PWM : CRUISE_PWM;
  ledcWrite(ENA_PIN, duty);
  ledcWrite(ENB_PIN, duty);
}

float readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  const unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (!duration) return -1.0f;
  const float cm = duration * 0.0343f / 2.0f;
  return cm >= 2.0f && cm <= 400.0f ? cm : -1.0f;
}

const char* actionName(ThresholdAction action) {
  switch (action) {
    case ThresholdAction::SLOW: return "slow";
    case ThresholdAction::FORWARD: return "forward";
    default: return "stop";
  }
}

void setup() {
  Serial.begin(115200);
  for (int pin : {ENA_PIN, ENB_PIN, IN1_PIN, IN2_PIN, IN3_PIN, IN4_PIN}) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  const bool leftReady = ledcAttach(ENA_PIN, 1000, 8);
  const bool rightReady = ledcAttach(ENB_PIN, 1000, 8);
  pwmReady = leftReady && rightReady;
  stopMotors();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  Serial.println("timestamp,distance_cm,valid,action");
  Serial.println("# threshold baseline: >40 cm forward; >20-40 cm slow; <=20 cm stop; <8 cm emergency");
  Serial.println("# A arms, S stops and disarms");
  nextSample = millis();
}

void loop() {
  while (Serial.available()) {
    const char command = Serial.read();
    if (command == 'S' || command == 's') {
      armed = false;
      stopMotors();
    } else if (command == 'A' || command == 'a') {
      armed = true;
    }
  }

  const unsigned long now = millis();
  if (static_cast<long>(now - nextSample) < 0) return;
  nextSample = now + SAMPLE_MS;

  const float distance = readDistance();
  const bool valid = distance >= 2.0f && distance <= 400.0f;
  const ThresholdAction requestedAction = decideThresholdAction(distance, valid);
  const ThresholdAction effectiveAction = armed && pwmReady ? requestedAction : ThresholdAction::STOP;
  drive(effectiveAction);
  Serial.printf("%lu,%.3f,%d,%s\n", now, distance, valid ? 1 : 0, actionName(effectiveAction));
}
