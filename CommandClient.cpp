// CommandClient.cpp
#include "CommandClient.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

Command CommandClient::poll() {
  Command out;
  WiFiClientSecure client; client.setInsecure();
  HTTPClient http;

  if (!http.begin(client, _url)) return out;

  int code = http.GET();
  Serial.printf("GET %s -> %d\n", _url.c_str(), code);
  if (code == 200) {
    String payload = http.getString();
    Serial.println("Payload: " + payload);

    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, payload);
    if (!err) {
      if (doc["command"].is<JsonObject>()) {
        JsonObject obj = doc["command"];
        out.command   = obj["command"]  | "";   // "start"/"stop"/"idle"
        out.patientId = obj["patientId"].as<String>();  // null -> ""
      } else if (doc["command"].is<const char*>()) {
        out.command   = (const char*)doc["command"];
       out.patientId = doc["patientId"].as<String>();
      }
      out.valid = out.command.length() > 0;
      Serial.printf("Parsed cmd='%s' pid='%s'\n", out.command.c_str(), out.patientId.c_str());
    } else {
      Serial.printf("JSON parse error: %s\n", err.c_str());
    }
  } else {
    Serial.println("Command GET failed");
  }
  http.end();
  return out;
}