#pragma once
#include <stddef.h>
#include "config.h"
#include "policy.h"

constexpr Action navigationAction(const char* label, float confidence, float distance,
                                  bool valid, bool enabled) {
  if (!enabled || mustStopForDistance(distance, valid, OBSTACLE_STOP_CM) ||
      !isFiniteUnitScore(confidence)) return Action::STOP;
  const bool knownLabel = labelEquals(label, "livre") ||
                          labelEquals(label, "aproximando") || labelEquals(label, "bloqueado");
  if (!knownLabel) return Action::STOP;
  // Na faixa de aproximacao, a distancia limita a velocidade mesmo com classe ambigua.
  if (distance <= SLOWDOWN_CM) return Action::SLOW;
  // Uma previsao ambigua nao pode prender o carro longe da barreira de 20 cm.
  // Avanco cauteloso continua sujeito a cada nova leitura do sensor.
  if (confidence < CONFIDENCE_MIN) return Action::SLOW;
  return decideAction(label, confidence, valid, enabled, CONFIDENCE_MIN);
}

constexpr int navigationPwm(Action action, float distance, float confidence = 1.0f) {
  if (action == Action::STOP) return 0;
  if (action != Action::SLOW) return CRUISE_PWM;
  if (confidence < CONFIDENCE_MIN) return SLOW_PWM;
  if (!(distance > OBSTACLE_STOP_CM)) return SLOW_PWM;
  if (distance >= SLOWDOWN_CM) return APPROACH_PWM;
  return SLOW_PWM + static_cast<int>((APPROACH_PWM - SLOW_PWM) *
      (distance - OBSTACLE_STOP_CM) / (SLOWDOWN_CM - OBSTACLE_STOP_CM));
}

// Pausas evitam inverter a ponte com os motores ainda acionados.
class Avoidance {
 public:
  constexpr void start(unsigned long now) { phase_ = 1; since_ = now; }
  constexpr void cancel() { phase_ = 0; }
  constexpr bool active() const { return phase_ != 0; }
  constexpr Action tick(unsigned long now) {
    if (!active()) return Action::STOP;
    const unsigned long duration = phase_ == 2 ? 300 : (phase_ == 4 ? 450 : 150);
    if (now - since_ >= duration) {
      ++phase_;
      since_ = now;
      if (phase_ > 5) cancel();
    }
    if (phase_ == 2) return Action::REVERSE;
    if (phase_ == 4) return Action::TURN_RIGHT;
    return Action::STOP;
  }

 private:
  unsigned int phase_ = 0;
  unsigned long since_ = 0;
};
