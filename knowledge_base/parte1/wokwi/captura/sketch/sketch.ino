/*
 * Captura do fotoresistor no Wokwi: leitura crua e feature rotulada.
 * A cada 5 ms imprime raw,<adc>,. A cada 10 amostras (50 ms) imprime
 * feature,<feature>,<label>. Monitor serial: 115200 baud.
 *
 * O modulo Wokwi reduz AO com o aumento de luz, entao feature e o
 * complemento do ADC normalizado para que 0=escuro e 1=claro.
 */

static const int LDR_PIN = 36;
static const int AMOSTRAS_POR_JANELA = 10;
static const unsigned long PERIODO_MS = 5;
static const float ADC_MAX = 4095.0f;
static const float LIMIAR = 0.45f;

static int amostrasNaJanela = 0;
static long somaJanela = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  Serial.println("tipo,valor,label");
}

void loop() {
  int adc = analogRead(LDR_PIN);
  if (adc < 0) {
    adc = 0;
  } else if (adc > 4095) {
    adc = 4095;
  }

  Serial.print("raw,");
  Serial.print(adc);
  Serial.println(',');

  somaJanela += adc;
  amostrasNaJanela++;
  if (amostrasNaJanela == AMOSTRAS_POR_JANELA) {
    float media = (float)somaJanela / (float)AMOSTRAS_POR_JANELA;
    float feature = 1.0f - (media / ADC_MAX);
    int label = (feature < LIMIAR) ? 0 : 1;

    Serial.print("feature,");
    Serial.print(feature, 4);
    Serial.print(',');
    Serial.println(label);

    somaJanela = 0;
    amostrasNaJanela = 0;
  }

  delay(PERIODO_MS);
}
