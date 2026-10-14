/*
 * Inferencia com a biblioteca oficial do Edge Impulse, usando o LDR do Wokwi.
 * Instale biblioteca_arduino_edge_impulse.zip antes de compilar.
 * Circuito: fotoresistor AO -> GPIO36/VP, VCC -> 3V3, GND -> GND.
 * A feature reproduz o pre-processamento do Wokwi da Parte 1:
 *   1 - media de 10 leituras ADC / 4095
 */
#include <tinyml-ldr-pedrobarone_inferencing.h>

static const int LDR_PIN = 36;
static const int AMOSTRAS_JANELA = 10;
static const unsigned long INTERVALO_AMOSTRA_MS = 100;
static const float ADC_MAXIMO = 4095.0f;

static float ler_feature() {
  uint32_t soma_adc = 0;

  for (int i = 0; i < AMOSTRAS_JANELA; ++i) {
    int leitura = analogRead(LDR_PIN);
    if (leitura < 0) {
      leitura = 0;
    } else if (leitura > static_cast<int>(ADC_MAXIMO)) {
      leitura = static_cast<int>(ADC_MAXIMO);
    }
    soma_adc += static_cast<uint32_t>(leitura);
    delay(INTERVALO_AMOSTRA_MS);
  }

  const float media_adc = static_cast<float>(soma_adc) / AMOSTRAS_JANELA;
  return 1.0f - media_adc / ADC_MAXIMO;
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  Serial.println("feature,label_0,label_1,predicted");
}

void loop() {
  const float feature = ler_feature();
  float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = {feature};
  signal_t signal;
  ei_impulse_result_t result = {};

  const int signal_error = numpy::signal_from_buffer(
      features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
  if (signal_error != 0) {
    Serial.print("Erro ao preparar sinal: ");
    Serial.println(signal_error);
    return;
  }

  const EI_IMPULSE_ERROR inference_error = run_classifier(&signal, &result, false);
  if (inference_error != EI_IMPULSE_OK) {
    Serial.print("Erro de inferencia: ");
    Serial.println(static_cast<int>(inference_error));
    return;
  }

  size_t classe_predita = 0;
  for (size_t i = 1; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
    if (result.classification[i].value > result.classification[classe_predita].value) {
      classe_predita = i;
    }
  }

  Serial.print(feature, 4);
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
    Serial.print(',');
    Serial.print(result.classification[i].label);
    Serial.print('=');
    Serial.print(result.classification[i].value, 4);
  }
  Serial.print(",predicted=");
  Serial.println(result.classification[classe_predita].label);
}
