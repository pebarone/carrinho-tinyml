#include <Arduino.h>
#include "config.h"
#include "policy.h"
#include "motors.h"
#include "ultrasonic.h"
#if !COLLECTION_MODE
#include "inference.h"
#endif

float samples[WINDOW_SAMPLES];
size_t sampleCount = 0;
unsigned long nextSample = 0;
bool armed = false;
motors::MotorDriver motorDriver;
ultrasonic::Sensor distanceSensor;

bool processSerialCommands() {
  bool handledCommand = false;
  while (Serial.available()) {
    const char command = Serial.read();
    if (command == 'S' || command == 's') {
      armed = false;
      motorDriver.stop();
      sampleCount = 0;
      Serial.println("# disarmed");
      handledCommand = true;
    }
#if !COLLECTION_MODE
    else if (command == 'A' || command == 'a') {
      armed = true;
      sampleCount = 0;
      Serial.println("# armed");
      handledCommand = true;
    }
#endif
  }
  return handledCommand;
}

void inferCurrentWindow() {
#if !COLLECTION_MODE
  WindowPrediction prediction;
  if (!inferWindow(samples, prediction)) {
    motorDriver.stop();
    return;
  }
  const bool commandArrivedDuringInference = processSerialCommands();
  const Action action = decideAction(prediction.label, prediction.confidence, true,
                                     armed && motorDriver.ready() && !commandArrivedDuringInference,
                                     CONFIDENCE_MIN);
  motorDriver.drive(action);
  Serial.printf("# inference,%s,%.5f,action=%d,dsp_ms=%d,nn_ms=%d\n",
                prediction.label ? prediction.label : "unknown", prediction.confidence,
                static_cast<int>(action), prediction.dspMs, prediction.classificationMs);
#endif
}

void setup() {
  Serial.begin(115200);
  if (!motorDriver.begin()) {
#if !COLLECTION_MODE
    Serial.println("# pwm_error: motores desabilitados");
#endif
  }
  distanceSensor.begin();
  Serial.println("timestamp,distance_cm,valid");
  Serial.println(COLLECTION_MODE ? "# collection: motores desligados" : "# TinyML: A arma, S para");
  nextSample = millis();
}

void loop() {
  processSerialCommands();
  const unsigned long now = millis();
  if (static_cast<long>(now - nextSample) < 0) return;
  // Nao recuperar atraso com leituras em rajada: descartar janela irregular.
  if (now - nextSample > 20) {
    sampleCount = 0;
    motorDriver.stop();
  }
  nextSample = now + SAMPLE_MS;
  const float distance = distanceSensor.readCm();
  const bool valid = distanceSensor.isValid(distance);
  Serial.printf("%lu,%.3f,%d\n", now, distance, valid ? 1 : 0);
  if (!valid || distance < EMERGENCY_CM) {
    motorDriver.stop();
    if (!valid) {
      sampleCount = 0;
      return;
    }
  }
  samples[sampleCount++] = distance;
  if (sampleCount == WINDOW_SAMPLES) {
    // O intertravamento tambem vale se uma leitura anterior da janela foi critica.
    bool safe = true;
    for (float value : samples) {
      if (value < EMERGENCY_CM) safe = false;
    }
    if (safe) {
      inferCurrentWindow();
#if !COLLECTION_MODE
      // O proximo intervalo comeca apos a inferencia para evitar atraso artificial.
      nextSample = millis() + SAMPLE_MS;
#endif
    } else {
      motorDriver.stop();
    }
    sampleCount = 0;
  }
}
