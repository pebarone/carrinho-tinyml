/*
 * Demonstracao de inferencia local com dados sinteticos.
 * Este classificador NAO e uma exportacao do Edge Impulse.
 * Mesmo circuito: LDR GL5528 + 10 kOhm no GPIO36, ESP32 Dev Module/Wokwi.
 * A janela de 1 s é a mesma do pré-processamento da parte 1.
 * Monitor serial: 115200.
 */

#include "modelo_local_demonstracao.h"

static const int LDR_PIN = 36;
static const int JANELA = 10;
static const unsigned long PERIODO_MS = 100;
static const float ESCALA_ADC = 4095.0f;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  Serial.print("origem_modelo,");
  Serial.println("demo_local_sintetica_nao_edge_impulse");
  Serial.println("feature,classe,prob_escuro,prob_claro");
}

void loop() {
  long soma = 0;
  float feature;
  float p0 = 0.0f;
  float p1 = 0.0f;
  int classe;

  for (int i = 0; i < JANELA; i++) {
    int bruto = analogRead(LDR_PIN);
    if (bruto < 0) {
      bruto = 0;
    } else if (bruto > 4095) {
      bruto = 4095;
    }
    soma += bruto;
    delay(PERIODO_MS);
  }

  feature = (float)soma / (float)JANELA;
  if (feature < 0.0f) {
    feature = 0.0f;
  } else if (feature > ESCALA_ADC) {
    feature = ESCALA_ADC;
  }
  feature = feature / ESCALA_ADC;

  classe = model_classify(feature, &p0, &p1);
  Serial.print(feature, 4);
  Serial.print(',');
  Serial.print(MODEL_LABELS[classe]);
  Serial.print(',');
  Serial.print(p0, 3);
  Serial.print(',');
  Serial.println(p1, 3);
}
