#pragma once

#include "config.h"

#if !COLLECTION_MODE
#include "model_config.h"

static_assert(EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME == 1, "Use somente distance_cm");
static_assert(EI_CLASSIFIER_RAW_SAMPLE_COUNT == WINDOW_SAMPLES,
              "Configure janela de 10 leituras");
static_assert(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE == WINDOW_SAMPLES,
              "Configure janela de 10 leituras");
static_assert(EI_CLASSIFIER_INTERVAL_MS == SAMPLE_MS, "Configure 10 Hz");
static_assert(EI_CLASSIFIER_FREQUENCY == 1000 / SAMPLE_MS, "Configure 10 Hz");
static_assert(EI_CLASSIFIER_LABEL_COUNT == 3, "Configure as tres classes");
static_assert(EI_CLASSIFIER_PROJECT_ID == 1139087, "Use a exportacao CP2 correta");
static_assert(EI_CLASSIFIER_TFLITE_INPUT_DATATYPE == EI_CLASSIFIER_DATATYPE_INT8,
              "Use entrada quantizada int8");
static_assert(EI_CLASSIFIER_TFLITE_OUTPUT_DATATYPE == EI_CLASSIFIER_DATATYPE_INT8,
              "Use saida quantizada int8");

struct WindowPrediction {
  const char* label = nullptr;
  float confidence = 0.0f;
  int dspMs = 0;
  int classificationMs = 0;
};

inline bool inferWindow(const float* samples, WindowPrediction& prediction) {
  signal_t signal;
  ei_impulse_result_t result = {};
  if (numpy::signal_from_buffer(samples, WINDOW_SAMPLES, &signal) != 0) {
    Serial.println("# signal_error");
    return false;
  }
  const EI_IMPULSE_ERROR error = run_classifier(&signal, &result, false);
  if (error != EI_IMPULSE_OK) {
    Serial.printf("# inference_error=%d\n", error);
    return false;
  }

  size_t best = 0;
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
    Serial.printf("# score,%s,%.5f\n", result.classification[i].label,
                  result.classification[i].value);
    if (result.classification[i].value > result.classification[best].value) best = i;
  }
  prediction.label = result.classification[best].label;
  prediction.confidence = result.classification[best].value;
  prediction.dspMs = result.timing.dsp;
  prediction.classificationMs = result.timing.classification;
  return true;
}
#endif
