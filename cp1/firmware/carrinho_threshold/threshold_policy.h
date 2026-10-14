#pragma once

enum class ThresholdAction { STOP, SLOW, FORWARD };

constexpr float EMERGENCY_CM = 8.0f;
constexpr float STOP_DISTANCE_CM = 20.0f;
constexpr float CAUTION_DISTANCE_CM = 40.0f;

constexpr ThresholdAction decideThresholdAction(float distanceCm, bool valid) {
  if (!valid || distanceCm != distanceCm || distanceCm < 2.0f || distanceCm > 400.0f ||
      distanceCm < EMERGENCY_CM ||
      distanceCm <= STOP_DISTANCE_CM) {
    return ThresholdAction::STOP;
  }
  if (distanceCm <= CAUTION_DISTANCE_CM) return ThresholdAction::SLOW;
  return ThresholdAction::FORWARD;
}
