/*
 * Parte 1b — media, limitacao de faixa, normalizacao e limiar.
 *
 * Entrada: fotoresistor Wokwi no GPIO36, conforme diagram.json. Neste
 * modulo a tensao AO diminui quando a iluminancia aumenta, entao a
 * normalizacao e invertida para manter 0=escuro e 1=claro.
 * A saida serial e CSV: feature,label, uma janela de 10 leituras
 * a cada 100 ms (aproximadamente 1 s).
 *
 * label 0: feature < 0.45 (escuro)
 * label 1: feature >= 0.45 (claro)
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

  float feature = 1.0f - (media / ESCALA_ADC);
  int label = (feature < LIMIAR) ? 0 : 1;

  Serial.print(feature, 4);
  Serial.print(',');
  Serial.println(label);
}
