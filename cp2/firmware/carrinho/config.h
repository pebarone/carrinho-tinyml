#pragma once
// ESP32 DevKit WROOM. Assumido: ponte L298N com jumpers ENA/ENB removidos.
#ifndef COLLECTION_MODE
#define COLLECTION_MODE 0
#endif
constexpr int TRIG_PIN = 23;
constexpr int ECHO_PIN = 22;
constexpr int ENA_PIN = 25;
constexpr int IN1_PIN = 26;
constexpr int IN2_PIN = 27;
constexpr int ENB_PIN = 33;
constexpr int IN3_PIN = 32;
constexpr int IN4_PIN = 21;
constexpr unsigned long SAMPLE_MS = 100;
constexpr size_t WINDOW_SAMPLES = 10;
constexpr float EMERGENCY_CM = 8.0f;
constexpr float OBSTACLE_STOP_CM = 20.0f;
constexpr float SLOWDOWN_CM = 40.0f;
constexpr float CONFIDENCE_MIN = 0.80f;
constexpr int SLOW_PWM = 180;
constexpr int APPROACH_PWM = 220;
constexpr int CRUISE_PWM = 255;
static_assert(SLOW_PWM > 0 && SLOW_PWM < APPROACH_PWM && APPROACH_PWM < CRUISE_PWM && CRUISE_PWM <= 255,
              "PWM lento deve ser menor que cruzeiro");
static_assert(SLOWDOWN_CM > OBSTACLE_STOP_CM, "Reducao deve anteceder parada");
constexpr unsigned long AUTO_START_MS = 5000;
static_assert(CONFIDENCE_MIN >= 0.0f && CONFIDENCE_MIN <= 1.0f,
              "CONFIDENCE_MIN deve estar entre 0 e 1");
