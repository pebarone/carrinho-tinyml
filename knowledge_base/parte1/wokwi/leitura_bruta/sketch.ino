/*
 * Parte 1a — leitura crua simulada do fotoresistor Wokwi.
 *
 * Monitor serial: 115200 baud. Uma leitura ADC de 12 bits por linha,
 * a cada 100 ms. O circuito e o controle de iluminancia estao em
 * diagram.json; a simulacao nao e uma medicao de bancada.
 */

static const int LDR_PIN = 36;
static const unsigned long PERIODO_MS = 100;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
  Serial.println("adc");
}

void loop() {
  Serial.println(analogRead(LDR_PIN));
  delay(PERIODO_MS);
}
