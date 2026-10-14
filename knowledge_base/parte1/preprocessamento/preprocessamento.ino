/*
 * Parte 1b — pré-processamento e limiar claro/escuro.
 *
 * Mesmo circuito de leitura_bruta.ino (LDR GL5528, GPIO36, ESP32 Dev Module).
 * Monitor serial: 115200. A saída já sai no formato do CSV da aula:
 *   feature,label
 *
 * Pré-processamento (a aula pede pelo menos dois):
 *   1. Média móvel numa janela de 1 s (10 leituras x 100 ms)
 *   2. Limitação de faixa do ADC para 0..4095
 *   3. Normalização para 0..1 (feature = média / 4095)
 *
 * Classes, sobre a feature já normalizada:
 *   0 escuro  -> feature <  0.45   (abaixo de cerca de 14 lux neste divisor)
 *   1 claro   -> feature >= 0.45
 */

static const int LDR_PIN = 36;
static const int JANELA = 10;
static const unsigned long PERIODO_MS = 100;
static const float ESCALA_ADC = 4095.0f;
static const float LIMIAR = 0.45f;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  Serial.println("feature,label");
}

void loop() {
  long soma = 0;

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

  float media = (float)soma / (float)JANELA;
  if (media < 0.0f) {
    media = 0.0f;
  } else if (media > ESCALA_ADC) {
    media = ESCALA_ADC;
  }

  float feature = media / ESCALA_ADC;
  int label = (feature < LIMIAR) ? 0 : 1;

  Serial.print(feature, 4);
  Serial.print(',');
  Serial.println(label);
}
