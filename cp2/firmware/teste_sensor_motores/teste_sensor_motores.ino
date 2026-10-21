#include <Arduino.h>

// Arquivo independente para abrir diretamente no Arduino IDE.
// Mesmo mapa de pinos de cp2/firmware/carrinho/config.h.
constexpr int TRIG_PIN = 23;
constexpr int ECHO_PIN = 22;
constexpr int ENA_PIN = 25;
constexpr int IN1_PIN = 26;
constexpr int IN2_PIN = 27;
constexpr int ENB_PIN = 33;
constexpr int IN3_PIN = 32;
constexpr int IN4_PIN = 21;
constexpr unsigned long SENSOR_INTERVAL_MS = 250;
constexpr unsigned long ECHO_TIMEOUT_US = 25000;
constexpr unsigned long MOVEMENT_MS = 1000;
constexpr unsigned long REVERSAL_PAUSE_MS = 300;

// -1 = tras, 0 = parado, 1 = frente.
int direction = 0;
int pendingDirection = 0;
unsigned long stoppedAt = 0;
unsigned long movementStarted = 0;
unsigned long lastSensorRead = 0;

void stopMotors() {
  digitalWrite(ENA_PIN, LOW);
  digitalWrite(ENB_PIN, LOW);
  digitalWrite(IN1_PIN, LOW);
  digitalWrite(IN2_PIN, LOW);
  digitalWrite(IN3_PIN, LOW);
  digitalWrite(IN4_PIN, LOW);
  direction = 0;
  stoppedAt = millis();
}

void startMotors(int requestedDirection) {
  const bool forward = requestedDirection == 1;
  digitalWrite(IN1_PIN, forward ? HIGH : LOW);
  digitalWrite(IN2_PIN, forward ? LOW : HIGH);
  // Motor de OUT3/OUT4 montado com orientacao oposta ao de OUT1/OUT2.
  digitalWrite(IN3_PIN, forward ? LOW : HIGH);
  digitalWrite(IN4_PIN, forward ? HIGH : LOW);
  digitalWrite(ENA_PIN, HIGH);
  digitalWrite(ENB_PIN, HIGH);
  direction = requestedDirection;
  pendingDirection = 0;
  movementStarted = millis();
  Serial.println(forward ? "# motores,frente,1000ms" : "# motores,tras,1000ms");
}

void requestMovement(int requestedDirection) {
  if (direction == requestedDirection) {
    movementStarted = millis();
    Serial.println("# motores,tempo_renovado,1000ms");
    return;
  }
  if (direction != 0) stopMotors();
  // Garante intervalo sem energia entre sentidos opostos, sem bloquear a serial.
  if (millis() - stoppedAt < REVERSAL_PAUSE_MS) {
    pendingDirection = requestedDirection;
    Serial.println("# motores,aguardando_pausa");
  } else {
    startMotors(requestedDirection);
  }
}

void handleSerial() {
  while (Serial.available() > 0) {
    const char command = Serial.read();
    if (command == 'W' || command == 'w') {
      requestMovement(1);
    } else if (command == 'S' || command == 's') {
      requestMovement(-1);
    } else if (command == 'X' || command == 'x') {
      pendingDirection = 0;
      stopMotors();
      Serial.println("# motores,parado");
    } else if (command != '\r' && command != '\n' && command != ' ') {
      pendingDirection = 0;
      stopMotors();
      Serial.println("# comando_invalido; W=frente; S=tras; X=parar");
    }
  }
}

void updateMotors() {
  const unsigned long now = millis();
  if (direction != 0 && now - movementStarted >= MOVEMENT_MS) {
    stopMotors();
    Serial.println("# motores,parado,tempo_esgotado");
  }
  if (pendingDirection != 0 && now - stoppedAt >= REVERSAL_PAUSE_MS) {
    startMotors(pendingDirection);
  }
}

void printSensor() {
  // Mesma sequencia de leitura usada em carrinho/ultrasonic.h.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  const unsigned long pulseUs = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  // Atende parada/tempo limite imediatamente apos a leitura (ate 25 ms).
  handleSerial();
  updateMotors();
  const float cm = pulseUs * 0.0343f / 2.0f;
  const bool valid = pulseUs > 0 && cm >= 2.0f && cm <= 400.0f;
  Serial.print(millis());
  Serial.print(',');
  if (valid) Serial.print(cm, 1);
  else Serial.print("NA");
  Serial.print(',');
  Serial.print(pulseUs);
  Serial.print(',');
  Serial.print(pulseUs == 0 ? "SEM_ECHO" : (valid ? "OK" : "FORA_DA_FAIXA"));
  Serial.print(',');
  Serial.println(direction == 0 ? "PARADO" : (direction == 1 ? "FRENTE" : "TRAS"));
}

void setup() {
  const int pins[] = {ENA_PIN, ENB_PIN, IN1_PIN, IN2_PIN, IN3_PIN, IN4_PIN};
  for (int pin : pins) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
  }
  stopMotors();
  digitalWrite(TRIG_PIN, LOW);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(115200);
  Serial.println("# teste_sensor_motores; W=frente; S=tras; X=parar");
  Serial.println("# Cada movimento dura 1s; sensor apenas informa, nao controla motores.");
  Serial.println("tempo_ms,distancia_cm,echo_us,sensor,motores");
}

void loop() {
  handleSerial();
  updateMotors();
  if (millis() - lastSensorRead >= SENSOR_INTERVAL_MS) {
    lastSensorRead = millis();
    printSensor();
  }
}
