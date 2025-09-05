// DataUploader.cpp
#include "DataUploader.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define DEBUG_UPLOAD 1           // 1=לוגים מלאים, 0=שקט
#define BODY_PREVIEW_LIMIT 1024  // כמה תווים להדפיס מה‑JSON (למניעת הצפה)

static String truncateForLog(const String& s, size_t maxLen) {
  if (s.length() <= maxLen) return s;
  String out = s.substring(0, maxLen);
  out += F("... [truncated] (");
  out += s.length();
  out += F(" bytes total)");
  return out;
}

bool DataUploader::upload(const String& baseUrl, const String& patientId, const std::vector<Measurement>& data) {
  if (data.empty()) {
#if DEBUG_UPLOAD
    Serial.println(F("[UPLOAD] Skip: no data"));
#endif
    return false;
  }

  // לבטל // כפול אם בשוגג נשאר סלאש ב‑baseUrl
  String url = baseUrl;
  if (url.endsWith("/")) url.remove(url.length() - 1);
  url += "/";
  url += patientId;
  url += "/data";

#if DEBUG_UPLOAD
  Serial.println(F("========== UPLOAD BEGIN =========="));
  Serial.printf("[UPLOAD] URL: %s\n", url.c_str());
  Serial.printf("[UPLOAD] Samples: %u\n", (unsigned)data.size());
#endif

  // בניית JSON
  const size_t N = data.size();
  DynamicJsonDocument doc(JSON_ARRAY_SIZE(N) + N*JSON_OBJECT_SIZE(9) + 512);
  JsonArray arr = doc.to<JsonArray>();
  for (const auto& m : data) {
    JsonObject o = arr.createNestedObject();
    o["distance"]      = m.distance;
    o["speed"]         = m.speed;
    o["handPressureL"] = m.handPressureL;
    o["handPressureR"] = m.handPressureR;
    o["footLiftL"]     = m.footLiftL;
    o["footLiftR"]     = m.footLiftR;
    o["timestamp"]     = m.timestamp;
  }

  String body;
#if DEBUG_UPLOAD
  // יפה לעין כשמדבגים:
  serializeJsonPretty(doc, body);
#else
  serializeJson(doc, body);
#endif

#if DEBUG_UPLOAD
  Serial.printf("[UPLOAD] Body size: %u bytes\n", (unsigned)body.length());
  Serial.println(F("[UPLOAD] Body preview:"));
  Serial.println(truncateForLog(body, BODY_PREVIEW_LIMIT));
#endif

  // הכנת HTTPS
  WiFiClientSecure client;
  client.setInsecure(); // TODO: לייצור להגדיר root CA במקום setInsecure()
  HTTPClient http;
  http.setTimeout(15000); // 15s

  if (!http.begin(client, url)) {
#if DEBUG_UPLOAD
    Serial.println(F("[UPLOAD] http.begin() failed"));
    Serial.println(F("==========  UPLOAD END  =========="));
#endif
    return false;
  }

  http.addHeader(F("Content-Type"), F("application/json"));

  // שליחה
  int code = http.POST(body);
  String resp = http.getString();

#if DEBUG_UPLOAD
  Serial.printf("[UPLOAD] HTTP status: %d\n", code);
  Serial.println(F("[UPLOAD] Response body:"));
  Serial.println(resp);
#endif

  http.end();

#if DEBUG_UPLOAD
  Serial.println(F("==========  UPLOAD END  =========="));
#endif

  return code >= 200 && code < 300;
}
