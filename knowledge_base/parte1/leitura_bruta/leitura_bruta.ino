/*
 * Parte 1a — leitura bruta do LDR, sem pré-processamento.
 *
 * Placa: ESP32 Dev Module (WROOM-32)
 * Sensor: LDR GL5528 em divisor com 10 kΩ
 *
 *   3V3 ---- GL5528 ---- GPIO36 (VP, ADC1_CH0)
 *                          |
 *                         10 kΩ
 *                          |
 *                         GND
 *
 * Mais luz, menor a resistência do LDR, maior a tensão em GPIO36.
 * Monitor serial: 115200. Cada linha é o ADC de 12 bits (0 a 4095).
 */

static const int LDR_PIN = 36;
static const unsigned long PERIODO_MS = 100;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);
}

void loop() {
  int bruto = analogRead(LDR_PIN);
  Serial.println(bruto);
  delay(PERIODO_MS);
}
