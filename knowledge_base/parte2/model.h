#ifndef TINYML_PARTE2_EDGE_IMPULSE_MODEL_H
#define TINYML_PARTE2_EDGE_IMPULSE_MODEL_H

/*
 * Compatibility include for the official Arduino library exported by Edge
 * Impulse. Install biblioteca_arduino_edge_impulse.zip in the Arduino IDE
 * before including this file. The generated EON model and its weights live in
 * that library; this header is not a standalone model or a copy of the weights.
 */
#include <tinyml-ldr-pedrobarone_inferencing.h>

#if EI_CLASSIFIER_PROJECT_ID != 1120098
#error "The installed Edge Impulse library is not project 1120098."
#endif

#if EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE != 1 || EI_CLASSIFIER_LABEL_COUNT != 2
#error "Unexpected input or class count for the exported Edge Impulse model."
#endif

#if EI_CLASSIFIER_QUANTIZATION_ENABLED != 1
#error "The expected Edge Impulse deployment is quantized (int8)."
#endif

#endif // TINYML_PARTE2_EDGE_IMPULSE_MODEL_H
