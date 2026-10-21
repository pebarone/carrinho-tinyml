#include "../firmware/carrinho/policy.h"
#include "../firmware/carrinho/navigation.h"

constexpr bool avoidanceSequence() {
  Avoidance navigation;
  if (navigation.active() || navigation.tick(0) != Action::STOP) return false;
  navigation.start(100);
  if (navigation.tick(249) != Action::STOP) return false;
  if (navigation.tick(250) != Action::REVERSE) return false;
  if (navigation.tick(549) != Action::REVERSE) return false;
  if (navigation.tick(550) != Action::STOP) return false;
  if (navigation.tick(700) != Action::TURN_RIGHT) return false;
  if (navigation.tick(1149) != Action::TURN_RIGHT) return false;
  if (navigation.tick(1150) != Action::STOP) return false;
  if (navigation.tick(1300) != Action::STOP || navigation.active()) return false;
  navigation.start(~0UL - 100);
  if (navigation.tick(49) != Action::REVERSE) return false;
  navigation.cancel();
  return !navigation.active() && navigation.tick(500) == Action::STOP;
}
static_assert(avoidanceSequence(), "maneuver must pause, reverse, pause, turn, stop; cancellation and millis wrap must work");

static_assert(SLOW_PWM < APPROACH_PWM && APPROACH_PWM < CRUISE_PWM,
              "approach speeds must actually reduce motor duty");
static_assert(navigationAction("livre", .99f, 20, true, true) == Action::STOP,
              "distance must stop even a confident free prediction");
static_assert(navigationAction("livre", .99f, 30, true, true) == Action::SLOW,
              "current distance must slow a stale free prediction");
static_assert(navigationAction("bloqueado", .5f, 30, true, true) == Action::SLOW,
              "ambiguous known classes in the approach band must not cause stop/start chatter");
static_assert(navigationAction("livre", .5f, 60, true, true) == Action::SLOW,
              "uncertainty must allow cautious progress toward the distance barrier");
static_assert(navigationAction("aproximando", .53125f, 45, true, true) == Action::SLOW &&
              navigationAction("livre", .66406f, 50, true, true) == Action::SLOW,
              "static 45/50 cm windows must not deadlock navigation");
static_assert(navigationAction("aproximando", .537982f, 45, true, true) == Action::SLOW &&
              navigationAction("livre", .643956f, 50, true, true) == Action::SLOW &&
              navigationAction("livre", .792185f, 55, true, true) == Action::SLOW,
              "real float32 regression windows must keep making cautious progress");
static_assert(navigationPwm(Action::SLOW, 50, .66406f) == SLOW_PWM &&
              navigationPwm(Action::SLOW, 30, .5f) == SLOW_PWM,
              "uncertain predictions must use the minimum forward duty");
static_assert(navigationAction("bloqueado", .99f, 60, true, true) == Action::STOP,
              "a confident blocked prediction must still stop");
static_assert(navigationAction("livre", .99f, 60, true, true) == Action::FORWARD,
              "confident free space must resume cruising");
static_assert(navigationAction("aproximando", .99f, 30, false, true) == Action::STOP,
              "invalid readings must stop in the approach band");
static_assert(navigationAction("livre", .99f, 30, true, false) == Action::STOP,
              "the distance envelope must never bypass disarming");
static_assert(navigationAction(nullptr, .99f, 30, true, true) == Action::STOP,
              "missing labels must stop even in the approach band");
static_assert(navigationAction("unknown", .99f, 30, true, true) == Action::STOP,
              "unknown classes must stop");
static_assert(navigationAction("livre", __builtin_nanf(""), 30, true, true) == Action::STOP,
              "corrupt model scores must stop");
static_assert(navigationAction("livre", -0.1f, 50, true, true) == Action::STOP &&
              navigationAction("livre", 1.1f, 50, true, true) == Action::STOP &&
              navigationAction("unknown", .5f, 50, true, true) == Action::STOP &&
              navigationAction("livre", .5f, 50, true, false) == Action::STOP &&
              navigationAction("livre", .5f, 50, false, true) == Action::STOP,
              "cautious fallback must not bypass invalid predictions, disarming or sensor faults");
static_assert(navigationPwm(Action::SLOW, 40) == APPROACH_PWM &&
              navigationPwm(Action::SLOW, 30) == 200 &&
              navigationPwm(Action::SLOW, 20.1f) == SLOW_PWM,
              "approach PWM must decrease with distance");
static_assert(navigationPwm(Action::SLOW, 60) == APPROACH_PWM &&
              navigationPwm(Action::STOP, 10) == 0 &&
              navigationPwm(Action::REVERSE, 10) == CRUISE_PWM,
              "model slowdown, stop and reverse must have distinct duties");
static_assert(mustStopForDistance(20.0f, true, 20.0f),
              "the stop-distance boundary must stop without waiting for inference");
static_assert(mustStopForDistance(10.0f, true, 20.0f),
              "near obstacles must stop even if the model predicts free space");
static_assert(!mustStopForDistance(20.1f, true, 20.0f),
              "distance alone must not block readings above the stop boundary");
static_assert(mustStopForDistance(100.0f, false, 20.0f),
              "invalid readings must stop regardless of their numeric value");
static_assert(mustStopForDistance(__builtin_nanf(""), true, 20.0f),
              "NaN distance must not permit movement");
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
