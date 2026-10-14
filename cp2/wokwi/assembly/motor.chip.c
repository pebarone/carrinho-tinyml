#include "wokwi-api.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 64

typedef struct {
  pin_t a;
  pin_t b;
  buffer_t display;
  uint32_t width;
  uint32_t height;
  bool seen_a;
  bool seen_b;
} chip_state_t;

static void set_pixel(uint8_t *pixels, uint32_t width, uint32_t height,
                      int x, int y, const uint8_t color[4]) {
  if (x < 0 || y < 0 || (uint32_t)x >= width || (uint32_t)y >= height) return;
  uint8_t *pixel = pixels + ((uint32_t)y * width + (uint32_t)x) * 4;
  memcpy(pixel, color, 4);
}

static void fill_circle(uint8_t *pixels, uint32_t width, uint32_t height,
                        int cx, int cy, int radius, const uint8_t color[4]) {
  for (int y = cy - radius; y <= cy + radius; y++) {
    for (int x = cx - radius; x <= cx + radius; x++) {
      const int dx = x - cx;
      const int dy = y - cy;
      if (dx * dx + dy * dy <= radius * radius) {
        set_pixel(pixels, width, height, x, y, color);
      }
    }
  }
}

static void draw_line(uint8_t *pixels, uint32_t width, uint32_t height,
                      int x0, int y0, int x1, int y1,
                      const uint8_t color[4]) {
  const int dx = abs(x1 - x0);
  const int sx = x0 < x1 ? 1 : -1;
  const int dy = -abs(y1 - y0);
  const int sy = y0 < y1 ? 1 : -1;
  int error = dx + dy;

  for (;;) {
    set_pixel(pixels, width, height, x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    const int twice_error = 2 * error;
    if (twice_error >= dy) {
      error += dy;
      x0 += sx;
    }
    if (twice_error <= dx) {
      error += dx;
      y0 += sy;
    }
  }
}

static void render(chip_state_t *chip) {
  uint8_t pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT * 4];
  const uint8_t background[] = {24, 29, 35, 255};
  const uint8_t tire[] = {13, 16, 20, 255};
  const uint8_t moving_forward[] = {43, 145, 94, 255};
  const uint8_t moving_reverse[] = {52, 112, 173, 255};
  const uint8_t stopped[] = {91, 99, 108, 255};
  const uint8_t hub[] = {225, 230, 233, 255};
  const uint8_t arrow[] = {255, 255, 255, 255};
  const uint8_t stop_bar[] = {244, 76, 76, 255};
  const uint32_t pixel_count = chip->width * chip->height;
  const int cx = (int)chip->width / 2;
  const int cy = (int)chip->height / 2;
  const bool a_high = chip->seen_a || pin_read(chip->a) == HIGH;
  const bool b_high = chip->seen_b || pin_read(chip->b) == HIGH;
  const uint8_t *wheel = a_high == b_high
                             ? stopped
                             : (a_high ? moving_forward : moving_reverse);

  for (uint32_t i = 0; i < pixel_count; i++) {
    memcpy(pixels + i * 4, background, 4);
  }

  fill_circle(pixels, chip->width, chip->height, cx, cy, 30, tire);
  fill_circle(pixels, chip->width, chip->height, cx, cy, 23, wheel);
  fill_circle(pixels, chip->width, chip->height, cx, cy, 7, hub);

  if (a_high == b_high) {
    draw_line(pixels, chip->width, chip->height, cx - 7, cy, cx + 7, cy,
              stop_bar);
    draw_line(pixels, chip->width, chip->height, cx - 7, cy + 2, cx + 7,
              cy + 2, stop_bar);
  } else if (a_high) {
    draw_line(pixels, chip->width, chip->height, cx - 12, cy, cx + 12, cy,
              arrow);
    draw_line(pixels, chip->width, chip->height, cx + 12, cy, cx + 4, cy - 8,
              arrow);
    draw_line(pixels, chip->width, chip->height, cx + 12, cy, cx + 4, cy + 8,
              arrow);
  } else {
    draw_line(pixels, chip->width, chip->height, cx + 12, cy, cx - 12, cy,
              arrow);
    draw_line(pixels, chip->width, chip->height, cx - 12, cy, cx - 4, cy - 8,
              arrow);
    draw_line(pixels, chip->width, chip->height, cx - 12, cy, cx - 4, cy + 8,
              arrow);
  }

  buffer_write(chip->display, 0, pixels, pixel_count * 4);
}

static void chip_pin_change(void *user_data, pin_t pin, uint32_t value) {
  chip_state_t *chip = user_data;
  if (pin == chip->a && value == HIGH) chip->seen_a = true;
  if (pin == chip->b && value == HIGH) chip->seen_b = true;
}

static void refresh_display(void *user_data) {
  chip_state_t *chip = user_data;
  render(chip);
  chip->seen_a = false;
  chip->seen_b = false;
}

void chip_init(void) {
  chip_state_t *chip = calloc(1, sizeof(*chip));
  if (chip == NULL) return;

  chip->a = pin_init("A", INPUT);
  chip->b = pin_init("B", INPUT);
  chip->display = framebuffer_init(&chip->width, &chip->height);

  const pin_watch_config_t watch = {
      .edge = BOTH,
      .pin_change = chip_pin_change,
      .user_data = chip,
  };
  pin_watch(chip->a, &watch);
  pin_watch(chip->b, &watch);
  // Draw at 10 Hz instead of redrawing the framebuffer on every PWM edge.
  const timer_config_t refresh = {
      .callback = refresh_display,
      .user_data = chip,
  };
  timer_start(timer_init(&refresh), 100000, true);
  render(chip);
}
