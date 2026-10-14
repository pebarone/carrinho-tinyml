#include "../firmware/carrinho/policy.h"

constexpr float CONFIDENCE_MIN = 0.80f;
static_assert(decideAction("livre", 0.80f, true, true, CONFIDENCE_MIN) == Action::FORWARD,
              "free class at the configured threshold must move forward");
static_assert(decideAction("livre", 0.79f, true, true, CONFIDENCE_MIN) == Action::STOP,
              "low confidence must stop");
static_assert(decideAction("aproximando", 0.90f, true, true, CONFIDENCE_MIN) == Action::SLOW,
              "approaching class must slow down");
static_assert(decideAction("bloqueado", 0.99f, true, true, CONFIDENCE_MIN) == Action::STOP,
              "blocked class must stop");
static_assert(decideAction("livre", 0.99f, false, true, CONFIDENCE_MIN) == Action::STOP,
              "invalid sensor data must stop");
static_assert(decideAction("livre", 0.99f, true, false, CONFIDENCE_MIN) == Action::STOP,
              "an unarmed vehicle must stop");
static_assert(decideAction(nullptr, 0.99f, true, true, CONFIDENCE_MIN) == Action::STOP,
              "a missing model label must stop");
static_assert(decideAction("livre", -0.1f, true, true, CONFIDENCE_MIN) == Action::STOP,
              "negative model scores must stop");
static_assert(decideAction("livre", 1.1f, true, true, CONFIDENCE_MIN) == Action::STOP,
              "model scores above one must stop");
static_assert(decideAction("livre", __builtin_nanf(""), true, true, CONFIDENCE_MIN) == Action::STOP,
              "NaN model scores must stop");
