#pragma once

enum class Action { STOP, SLOW, FORWARD, REVERSE, TURN_RIGHT };

constexpr bool mustStopForDistance(float distance, bool valid, float stopDistance) {
  return !valid || !(distance > stopDistance);
}

constexpr bool isFiniteUnitScore(float score) {
  return score == score && score >= 0.0f && score <= 1.0f;
}

constexpr bool labelEquals(const char* actual, const char* expected) {
  if (actual == nullptr || expected == nullptr) return false;
  while (*actual != '\0' && *expected != '\0') {
    if (*actual != *expected) return false;
    ++actual;
    ++expected;
  }
  return *actual == *expected;
}

constexpr Action decideAction(const char* label, float confidence, bool valid, bool armed,
                              float minimumConfidence) {
  if (!valid || !armed || !isFiniteUnitScore(confidence) ||
      !isFiniteUnitScore(minimumConfidence) || confidence < minimumConfidence) {
    return Action::STOP;
  }
  if (labelEquals(label, "livre")) return Action::FORWARD;
  if (labelEquals(label, "aproximando")) return Action::SLOW;
  return Action::STOP;
}
