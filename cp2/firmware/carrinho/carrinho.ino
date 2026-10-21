#include <Arduino.h>
#include "config.h"
#include "policy.h"
#include "navigation.h"
#include "motors.h"
#include "ultrasonic.h"
#include "network_status.h"
#if !COLLECTION_MODE
#include "inference.h"
#endif

float samples[WINDOW_SAMPLES];
size_t sampleCount = 0;
unsigned long nextSample = 0;
bool armed = false;
bool autoStartPending = !COLLECTION_MODE;
unsigned long bootTimestamp = 0;
Avoidance avoidance;
Action currentAction = Action::STOP;
float currentDistance = -1.0f;
bool currentValid = false;
const char* currentLabel = "aguardando";
float currentConfidence = 0.0f;
bool predictionReady = false;
int currentPwm = 0;
unsigned long readingTimestamp = 0;
NetworkStatus networkStatus;
motors::MotorDriver motorDriver;
ultrasonic::Sensor distanceSensor;

void publishStatus() {
  networkStatus.update(currentDistance, currentValid, armed, currentAction,
                       currentLabel, currentConfidence, readingTimestamp);
}

void stopMotors() {
  motorDriver.stop();
  if (currentAction != Action::STOP) {
    Serial.printf("# drive,action=0,pwm=0,distance=%.2f\n", currentDistance);
  }
  currentAction = Action::STOP;
  currentPwm = 0;
}

void applyAction(Action action) {
  const int duty = navigationPwm(action, currentDistance, currentConfidence);
  if (action == currentAction && duty == currentPwm) return;
  motorDriver.drive(action, duty);
  currentAction = action;
  currentPwm = duty;
  Serial.printf("# drive,action=%d,pwm=%d,distance=%.2f\n",
                static_cast<int>(action), duty, currentDistance);
}

bool processSerialCommands() {
  bool handledCommand = false;
  while (Serial.available()) {
    const char command = Serial.read();
    if (command == 'S' || command == 's') {
      armed = false;
      autoStartPending = false;
      avoidance.cancel();
      stopMotors();
      sampleCount = 0;
      predictionReady = false;
      Serial.println("# disarmed");
      handledCommand = true;
    }
#if !COLLECTION_MODE
    else if (command == 'A' || command == 'a') {
      armed = true;
      autoStartPending = false;
      avoidance.cancel();
      stopMotors();
      sampleCount = 0;
      predictionReady = false;
      Serial.println("# armed");
      handledCommand = true;
    }
#endif
  }
  if (handledCommand) publishStatus();
  return handledCommand;
}

void inferCurrentWindow() {
#if !COLLECTION_MODE
  WindowPrediction prediction;
  if (!inferWindow(samples, prediction)) {
    predictionReady = false;
    stopMotors();
    currentLabel = "erro_inferencia";
    currentConfidence = 0.0f;
    publishStatus();
    return;
  }
  const bool commandArrivedDuringInference = processSerialCommands();
  const bool enabled = armed && motorDriver.ready() && !commandArrivedDuringInference;
  const Action action = navigationAction(prediction.label, prediction.confidence,
                                         currentDistance, currentValid, enabled);
  predictionReady = enabled;
  currentLabel = prediction.label;
  currentConfidence = prediction.confidence;
  applyAction(action);
  publishStatus();
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
  Serial.println(COLLECTION_MODE ? "# collection: motores desligados" : "# TinyML autonomo: partida em 5s; S para/cancela; A rearma");
  bootTimestamp = millis();
  nextSample = millis();
  if (!networkStatus.begin()) Serial.println("# wifi,task_error: serial_continues");
}

void loop() {
  processSerialCommands();
  const unsigned long now = millis();
#if !COLLECTION_MODE
  if (autoStartPending && now - bootTimestamp >= AUTO_START_MS) {
    autoStartPending = false;
    armed = motorDriver.ready();
    sampleCount = 0;
    predictionReady = false;
    Serial.println("# auto_armed");
    publishStatus();
  }
  if (avoidance.active()) {
    if (!armed || !currentValid || now - readingTimestamp > 300) {
      avoidance.cancel();
      predictionReady = false;
      stopMotors();
    } else {
      const Action action = avoidance.tick(now);
      if (action != currentAction) {
        applyAction(action);
        Serial.printf("# avoidance,action=%d\n", static_cast<int>(action));
        publishStatus();
      }
      if (!avoidance.active()) {
        predictionReady = false;
        Serial.println("# avoidance,done: nova janela antes de avancar");
      }
    }
    sampleCount = 0;
  }
#endif
  if (static_cast<long>(now - nextSample) < 0) return;
  // Nao recuperar atraso com leituras em rajada: descartar janela irregular.
  if (now - nextSample > 20) {
    sampleCount = 0;
    predictionReady = false;
    avoidance.cancel();
    stopMotors();
  }
  nextSample = now + SAMPLE_MS;
  const float distance = distanceSensor.readCm();
  const bool valid = distanceSensor.isValid(distance);
  currentDistance = distance;
  currentValid = valid;
  readingTimestamp = now;
  Serial.printf("%lu,%.3f,%d\n", now, distance, valid ? 1 : 0);
  if (!valid || (!avoidance.active() && mustStopForDistance(distance, valid, OBSTACLE_STOP_CM))) {
    stopMotors();
    currentLabel = !valid ? "sensor_invalido" :
                   (distance < EMERGENCY_CM ? "emergencia" : "obstaculo_proximo");
    currentConfidence = 0.0f;
    if (!valid) {
      predictionReady = false;
      avoidance.cancel();
      sampleCount = 0;
      publishStatus();
      return;
    }
  }
#if !COLLECTION_MODE
  if (!avoidance.active() && armed && motorDriver.ready() && valid &&
      distance <= OBSTACLE_STOP_CM) {
    stopMotors();
    predictionReady = false;
    sampleCount = 0;
    avoidance.start(millis());
    Serial.println("# avoidance,start: distancia <= 20cm; pausa, re, pausa, giro direito, pausa");
  }
#endif
  if (avoidance.active()) {
    publishStatus();
    return;
  }
  if (predictionReady) {
    applyAction(navigationAction(currentLabel, currentConfidence, distance, valid,
                                 armed && motorDriver.ready()));
  }
  samples[sampleCount++] = distance;
  if (sampleCount == WINDOW_SAMPLES) {
    inferCurrentWindow();
#if !COLLECTION_MODE
    // O proximo intervalo comeca apos a inferencia para evitar atraso artificial.
    nextSample = millis() + SAMPLE_MS;
#endif
    sampleCount = 0;
  }
  publishStatus();
}
