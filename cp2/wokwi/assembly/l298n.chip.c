#include "wokwi-api.h"

#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  pin_t ena;
  pin_t in1;
  pin_t in2;
  pin_t in3;
  pin_t in4;
  pin_t enb;
  pin_t out1;
  pin_t out2;
  pin_t out3;
  pin_t out4;
} chip_state_t;

/* Digital logic proxy only; it does not model voltage, current, or torque. */
static void update_outputs(chip_state_t *chip) {
  const bool enabled_a = pin_read(chip->ena) == HIGH;
  const bool enabled_b = pin_read(chip->enb) == HIGH;

  pin_write(chip->out1, enabled_a && pin_read(chip->in1) == HIGH);
  pin_write(chip->out2, enabled_a && pin_read(chip->in2) == HIGH);
  pin_write(chip->out3, enabled_b && pin_read(chip->in3) == HIGH);
  pin_write(chip->out4, enabled_b && pin_read(chip->in4) == HIGH);
}

static void chip_pin_change(void *user_data, pin_t pin, uint32_t value) {
  (void)pin;
  (void)value;
  update_outputs((chip_state_t *)user_data);
}

void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(*chip));
  if (chip == NULL) return;

  chip->ena = pin_init("ENA", INPUT);
  chip->in1 = pin_init("IN1", INPUT);
  chip->in2 = pin_init("IN2", INPUT);
  chip->in3 = pin_init("IN3", INPUT);
  chip->in4 = pin_init("IN4", INPUT);
  chip->enb = pin_init("ENB", INPUT);

  chip->out1 = pin_init("OUT1", OUTPUT_LOW);
  chip->out2 = pin_init("OUT2", OUTPUT_LOW);
  chip->out3 = pin_init("OUT3", OUTPUT_LOW);
  chip->out4 = pin_init("OUT4", OUTPUT_LOW);

  /* Supply pins are labels only; this model does not source voltage. */
  (void)pin_init("GND", INPUT);
  (void)pin_init("5V", INPUT);
  (void)pin_init("Vs12V", INPUT);

  const pin_watch_config_t watch = {
      .edge = BOTH,
      .pin_change = chip_pin_change,
      .user_data = chip,
  };
  pin_watch(chip->ena, &watch);
  pin_watch(chip->in1, &watch);
  pin_watch(chip->in2, &watch);
  pin_watch(chip->in3, &watch);
  pin_watch(chip->in4, &watch);
  pin_watch(chip->enb, &watch);

  update_outputs(chip);
}
