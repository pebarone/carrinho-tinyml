/*
 * Captura unica para o projeto Wokwi da Parte 1.
 *
 * Liga o AO do wokwi-photoresistor-sensor ao GPIO36. Imprime uma leitura
 * ADC a cada 5 ms; a cada 10 leituras (50 ms) imprime feature,label.
 *
 * Protocolo serial (115200 baud), CSV misto com cabecalho:
 *   tipo,valor,label
 *   raw,<adc>,
 *   feature,<feature>,<label>
 *
 * A linha "raw" e dado cru de simulacao. O modulo Wokwi reduz AO quando
 * a luz aumenta; feature = 1 - media/4095 mantem 0=escuro e 1=claro.
 * O ADC fica limitado a 0..4095 e o limiar e 0.45.
 */

static const int LDR_PIN = 36;
static const int AMOSTRAS_POR_JANELA = 10;
static const unsigned long PERIODO_MS = 5;
static const float ESCALA_ADC = 4095.0f;
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
  int bruto = analogRead(LDR_PIN);
  if (bruto < 0) {
    bruto = 0;
  } else if (bruto > 4095) {
    bruto = 4095;
  }

  Serial.print("raw,");
  Serial.print(bruto);
  Serial.println(',');

  somaJanela += bruto;
  amostrasNaJanela++;

  if (amostrasNaJanela == AMOSTRAS_POR_JANELA) {
    float media = (float)somaJanela / (float)AMOSTRAS_POR_JANELA;
    float feature = 1.0f - (media / ESCALA_ADC);
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
