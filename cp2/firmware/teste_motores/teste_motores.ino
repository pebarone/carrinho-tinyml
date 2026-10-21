#include <Arduino.h>

// Mesmo mapa de pinos de carrinho/config.h.
constexpr int ENA_PIN = 25;
constexpr int IN1_PIN = 26;
constexpr int IN2_PIN = 27;
constexpr int ENB_PIN = 33;
constexpr int IN3_PIN = 32;
constexpr int IN4_PIN = 21;
constexpr unsigned long DRIVE_MS = 1000;
constexpr unsigned long PAUSE_MS = 300;

bool running = false;
unsigned int phase = 0;
unsigned long phaseStarted = 0;

void stopMotors() {
  digitalWrite(ENA_PIN, LOW);
  digitalWrite(ENB_PIN, LOW);
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
}

void applyPhase() {
  // Desabilita a ponte antes de trocar o sentido.
  stopMotors();
  if (phase == 0 || phase == 2) {
    const bool forward = phase == 0;
    digitalWrite(IN1_PIN, forward ? HIGH : LOW);
    digitalWrite(IN2_PIN, forward ? LOW : HIGH);
    digitalWrite(IN3_PIN, forward ? HIGH : LOW);
    digitalWrite(IN4_PIN, forward ? LOW : HIGH);
    digitalWrite(ENA_PIN, HIGH);
    digitalWrite(ENB_PIN, HIGH);
    Serial.println(forward ? "# motores,frente,1000ms" : "# motores,tras,1000ms");
  } else {
    Serial.println("# motores,pausa,300ms");
  }
  phaseStarted = millis();
}

void setup() {
  const int pins[] = {ENA_PIN, ENB_PIN, IN1_PIN, IN2_PIN, IN3_PIN, IN4_PIN};
  for (int pin : pins) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
  }
  stopMotors();
  Serial.begin(115200);
  Serial.println("# teste_motores,parado; A=iniciar; S=parar; sem controle de angulo");
}

void loop() {
  while (Serial.available() > 0) {
    const char command = Serial.read();
    if (command == 'S' || command == 's') {
      running = false;
      stopMotors();
      Serial.println("# motores,desarmado");
    } else if (command == 'A' || command == 'a') {
      running = true;
      phase = 0;
      applyPhase();
    } else if (command != '\r' && command != '\n' && command != ' ') {
      Serial.println("# comando_invalido; A=iniciar; S=parar");
    }
  }
  if (!running) return;
  const unsigned long duration = (phase == 0 || phase == 2) ? DRIVE_MS : PAUSE_MS;
  if (millis() - phaseStarted >= duration) {
    phase = (phase + 1) % 4;
    applyPhase();
  }
}
