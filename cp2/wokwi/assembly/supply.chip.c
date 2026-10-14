#include "wokwi-api.h"

void chip_init(void) {
  /* Pins label a nominal supply in the diagram; neither pin sources voltage. */
  (void)pin_init("PLUS", INPUT);
  (void)pin_init("GND", INPUT);
}
