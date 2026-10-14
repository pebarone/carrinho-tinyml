#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "policy.h"
#if __has_include("wifi_config.h")
#include "wifi_config.h"
#else
#include "wifi_config.example.h"
#endif

// A rede tem tarefa propria: clientes HTTP lentos nao bloqueiam sensor/motores.
class NetworkStatus {
 public:
  bool begin() {
    if (CAR_WIFI_SSID[0] == '\0') {
      Serial.println("# wifi,not_configured");
      return true;
    }
    return xTaskCreatePinnedToCore(task, "car-network", 6144, this, 1, nullptr, 0) == pdPASS;
  }

  void update(float distance, bool valid, bool armed, Action action,
              const char* label, float confidence, unsigned long timestamp) {
    Snapshot next = {};
    next.distance = distance;
    next.valid = valid;
    next.armed = armed;
    next.action = static_cast<int>(action);
    next.confidence = isFiniteUnitScore(confidence) ? confidence : 0.0f;
    next.timestamp = timestamp;
    snprintf(next.label, sizeof(next.label), "%s", label ? label : "aguardando");
    portENTER_CRITICAL(&mutex_);
    snapshot_ = next;
    portEXIT_CRITICAL(&mutex_);
  }

 private:
  struct Snapshot {
    float distance = -1.0f;
    float confidence = 0.0f;
    bool valid = false;
    bool armed = false;
    int action = 0;
    unsigned long timestamp = 0;
    char label[24] = "aguardando";
  };
  Snapshot snapshot_;
  portMUX_TYPE mutex_ = portMUX_INITIALIZER_UNLOCKED;
  WebServer server_{80};

  void respond() {
    Snapshot copy;
    portENTER_CRITICAL(&mutex_);
    copy = snapshot_;
    portEXIT_CRITICAL(&mutex_);
    char distance[24];
    if (copy.valid) snprintf(distance, sizeof(distance), "%.3f", copy.distance);
    else snprintf(distance, sizeof(distance), "null");
    char response[320];
    snprintf(response, sizeof(response),
             "{\"timestamp_ms\":%lu,\"distance_cm\":%s,\"sensor_valid\":%s,"
             "\"armed\":%s,\"class\":\"%s\",\"confidence\":%.5f,\"action\":%d}",
             copy.timestamp, distance, copy.valid ? "true" : "false",
             copy.armed ? "true" : "false", copy.label, copy.confidence, copy.action);
    server_.sendHeader("Cache-Control", "no-store");
    server_.send(200, "application/json; charset=utf-8", response);
  }

  static void task(void* argument) {
    static_cast<NetworkStatus*>(argument)->run();
  }

  void run() {
    WiFi.persistent(false);
    WiFi.setHostname("carrinho-cp2");
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(CAR_WIFI_SSID, CAR_WIFI_PASSWORD);
    server_.on("/", HTTP_GET, [this]() { respond(); });
    server_.on("/state", HTTP_GET, [this]() { respond(); });
    bool serving = false;
    unsigned long lastAttempt = millis();
    Serial.println("# wifi,connecting_background");
    for (;;) {
      if (WiFi.status() == WL_CONNECTED) {
        if (!serving) {
          server_.begin();
          serving = true;
          Serial.print("# wifi,connected,http://");
          Serial.println(WiFi.localIP());
        }
        server_.handleClient();
      } else {
        if (serving) {
          server_.stop();
          serving = false;
          Serial.println("# wifi,disconnected");
        }
        if (millis() - lastAttempt >= 30000UL) {
          WiFi.reconnect();
          lastAttempt = millis();
          Serial.println("# wifi,retrying_background");
        }
      }
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
};
