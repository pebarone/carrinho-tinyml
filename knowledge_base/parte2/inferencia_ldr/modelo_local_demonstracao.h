#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>
#include <math.h>

/* Demonstracao local com dados sinteticos. NAO e exportacao Edge Impulse.
 * Quantizacao int8 implementada neste script; nao e quantizacao do EI.
 * Input (1) -> Dense 20 ReLU -> Dense 10 ReLU -> logits (2).
 * Classes: 0 escuro, 1 claro.
 * Treino: 20 ciclos, Adam, learning rate 0.005.
 */

#define MODEL_PROJECT "DEMO_LOCAL_SINTETICA"
#define MODEL_N_CLASSES 2

static const char *const MODEL_LABELS[MODEL_N_CLASSES] = {"escuro", "claro"};

static const int8_t MODEL_W1[20] = {
    81, -84, 39, 81, 41, -50, -30, -50, -5, -3,
    84, -13, 107, 75, -1, -53, -82, 75, 127, 54
};
static const int32_t MODEL_B1[20] = {
    -2261, 0, 7539, -2320, 7244, 0, 0, 0, 0, -1414,
    -2298, 0, -2872, -2211, 0, 0, 0, -2019, -3468, -1661
};
static const int32_t MODEL_M1 = 22279;
static const int MODEL_SHIFT1 = 21;

static const int8_t MODEL_W2[10][20] = {
    {119, -22, -16, 70, 54, 35, 21, 11, 25, -52, 107, 59, 101, 71, 7, 48, -5, 124, 80, 15},
    {3, 60, 62, 123, 38, -32, 28, 62, 39, -22, 92, -9, 115, 32, 61, -3, 52, 127, 34, 92},
    {-53, 16, 60, -79, 79, -49, 27, 23, 21, -13, -87, -67, -78, -84, 8, -9, 5, -33, -6, -44},
    {33, 61, -29, -79, -14, -55, -30, -24, -33, 28, -49, 67, -14, 36, -4, 65, 9, 27, 21, 45},
    {54, 1, -26, 44, -71, 17, -19, -36, 38, -11, -5, -2, -13, 32, 41, 38, -2, -70, -16, 39},
    {-45, 18, 46, -42, 93, 36, 3, 37, 48, 0, 4, -17, -99, -77, 58, -22, -25, 8, -94, -60},
    {-43, -63, -64, -29, 15, -60, 18, 46, -24, -39, -42, -49, -63, -66, 17, 11, -37, 41, -62, -31},
    {33, 16, -34, 24, 19, 20, -29, -15, 49, -41, -28, 59, -5, -65, 2, 2, -42, -62, -64, 13},
    {-80, 23, 99, -101, 56, -53, 4, 46, 41, -35, -57, -32, 14, -88, 52, -55, -41, -97, -58, -95},
    {12, -5, -43, -14, 39, 42, 63, -56, 38, 23, 30, 28, 30, -16, 63, 61, 4, -32, -26, -57}
};
static const int32_t MODEL_B2[10] = {
    -3117, -8428, 10143, -1935, -1875, 10983, 0, 0, 10964, 0
};
static const int32_t MODEL_M2 = 18504;
static const int MODEL_SHIFT2 = 23;

static const int8_t MODEL_W3[2][10] = {
    {-114, -43, 97, -8, 44, 125, -2, 43, 120, -47},
    {127, 88, -115, -52, -37, -39, 12, 61, -45, 18}
};
static const int32_t MODEL_B3[2] = {
    1739, -1739
};
static const int32_t MODEL_M3 = 21792;
static const int MODEL_SHIFT3 = 22;
static const float LOGIT_SCALE = 2.41421245e-02f;

static inline int32_t model_mul_shift(int64_t acc, int32_t mult, int shift) {
  int64_t numero = acc * (int64_t)mult;
  int64_t vies = 1LL << (shift - 1);
  if (numero >= 0) {
    return (int32_t)((numero + vies) >> shift);
  }
  return (int32_t)(-(((-numero) + vies) >> shift));
}

static inline int model_classify(float feature, float *score_escuro, float *score_claro) {
  int32_t x;
  int8_t h1[20];
  int8_t h2[10];
  int8_t logits[2];
  int i;
  int k;
  int classe;

  if (feature < 0.0f) {
    feature = 0.0f;
  } else if (feature > 1.0f) {
    feature = 1.0f;
  }
  x = (int32_t)(feature * 127.0f + 0.5f);
  if (x > 127) {
    x = 127;
  }

  for (i = 0; i < 20; i++) {
    int64_t acc = (int64_t)MODEL_B1[i] + (int64_t)MODEL_W1[i] * (int64_t)x;
    int32_t y = 0;
    if (acc > 0) {
      y = model_mul_shift(acc, MODEL_M1, MODEL_SHIFT1);
      if (y < 0) {
        y = 0;
      } else if (y > 127) {
        y = 127;
      }
    }
    h1[i] = (int8_t)y;
  }

  for (i = 0; i < 10; i++) {
    int64_t acc = (int64_t)MODEL_B2[i];
    int32_t y = 0;
    for (k = 0; k < 20; k++) {
      acc += (int64_t)MODEL_W2[i][k] * (int64_t)h1[k];
    }
    if (acc > 0) {
      y = model_mul_shift(acc, MODEL_M2, MODEL_SHIFT2);
      if (y < 0) {
        y = 0;
      } else if (y > 127) {
        y = 127;
      }
    }
    h2[i] = (int8_t)y;
  }

  for (i = 0; i < 2; i++) {
    int64_t acc = (int64_t)MODEL_B3[i];
    int32_t y;
    for (k = 0; k < 10; k++) {
      acc += (int64_t)MODEL_W3[i][k] * (int64_t)h2[k];
    }
    y = model_mul_shift(acc, MODEL_M3, MODEL_SHIFT3);
    if (y > 127) {
      y = 127;
    } else if (y < -127) {
      y = -127;
    }
    logits[i] = (int8_t)y;
  }

  classe = (logits[1] > logits[0]) ? 1 : 0;
  if (score_escuro != 0 || score_claro != 0) {
    float z0 = (float)logits[0] * LOGIT_SCALE;
    float z1 = (float)logits[1] * LOGIT_SCALE;
    float m = (z0 > z1) ? z0 : z1;
    float e0 = expf(z0 - m);
    float e1 = expf(z1 - m);
    float soma = e0 + e1;
    if (score_escuro != 0) {
      *score_escuro = e0 / soma;
    }
    if (score_claro != 0) {
      *score_claro = e1 / soma;
    }
  }
  return classe;
}

#endif
