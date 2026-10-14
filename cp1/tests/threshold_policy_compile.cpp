#include "../firmware/carrinho_threshold/threshold_policy.h"

static_assert(decideThresholdAction(40.1f, true) == ThresholdAction::FORWARD,
              "distance above 40 cm must use cruise speed");
static_assert(decideThresholdAction(40.0f, true) == ThresholdAction::SLOW,
              "distance at 40 cm must use slow speed");
static_assert(decideThresholdAction(20.1f, true) == ThresholdAction::SLOW,
              "distance between 20 cm and 40 cm must use slow speed");
static_assert(decideThresholdAction(20.0f, true) == ThresholdAction::STOP,
              "distance at 20 cm must stop");
static_assert(decideThresholdAction(7.9f, true) == ThresholdAction::STOP,
              "emergency distance must stop");
static_assert(decideThresholdAction(-1.0f, false) == ThresholdAction::STOP,
              "invalid sensor data must stop");
static_assert(decideThresholdAction(__builtin_nanf(""), true) == ThresholdAction::STOP,
              "NaN distance must stop");
static_assert(decideThresholdAction(401.0f, true) == ThresholdAction::STOP,
              "distance above the sensor range must stop");
static_assert(decideThresholdAction(__builtin_huge_valf(), true) == ThresholdAction::STOP,
              "infinite distance must stop");
