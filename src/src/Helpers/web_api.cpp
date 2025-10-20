#include "src/Helpers/web_api.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <FS.h>
#include "../../ESPEasy_common.h"
#include "../../ESPEasy-Globals.h"
#include "src/Globals/ESPEasy_time.h"
#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Globals/Settings.h"

String getServerUrl() {
  String ip = String(Settings.Web_ServisAdres);
  if (!ip.length()) {
    return "http://192.168.1.251:5000/api/Data/PostData"; // fallback
  }
  if (ip.indexOf(':') != -1) {
    return "http://" + ip + "/api/Data/PostData";
  }
  return "http://" + ip + ":5000/api/Data/PostData";
}

const char* backupFile = "/pending_data.txt";
const char* overflowLog = "/overflow_log.txt";
#define MAX_QUEUE 9999
#define MAX_LINE_LENGTH 512

typedef uint32_t u32;

u32 crc32(const uint8_t* data, size_t length) {
  u32 crc = 0xFFFFFFFF;
  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : crc >> 1;
    }
  }
  return ~crc;
}

bool parseLine(String line, String& jsonOut) {
  int sep = line.lastIndexOf('|');
  if (sep == -1) return false;
  String json = line.substring(0, sep);
  u32 crcStored = strtoul(line.substring(sep + 1).c_str(), nullptr, 10);
  u32 crcActual = crc32((const uint8_t*)json.c_str(), json.length());
  if (crcStored != crcActual) return false;
  jsonOut = json;
  return true;
}

bool sendHttpPostFromParsedJson(const String& jsonString) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, jsonString);
  if (err) {
    addLogMove(LOG_LEVEL_INFO, "[HATA] Ge\u00e7ersiz JSON sat\u0131r\u0131.");
    return false;
  }
  HTTPClient http;
  String url = getServerUrl();
  String zaman = doc["zaman"].as<String>();
  zaman.replace(" ", "%20");
  url += "?cihaz_id=" + doc["cihaz_id"].as<String>() +
         "&sayac1=" + String(doc["sayac1"].as<int>()) +
         "&sayac2=" + String(doc["sayac2"].as<int>()) +
         "&sayac3=" + String(doc["sayac3"].as<int>()) +
         "&sayac4=" + String(doc["sayac4"].as<int>()) +
         "&zaman=" + zaman;
  addLogMove(LOG_LEVEL_INFO, "🌐 [RETRY] URL: " + url);
  http.begin(url);
  http.setTimeout(1000);
  http.addHeader("Content-Length", "0");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");  // 🟢 EKLENDİ


  int code = http.POST("");
  String response = http.getString();
  http.end();
  addLogMove(LOG_LEVEL_INFO, "[HTTP] Kod: " + String(code));
  addLogMove(LOG_LEVEL_INFO, "📩 Gelen Cevap: " + response);
  bool isSuccess = false;

// JSON gibi parse etmeye çalış (tek tırnak olsa bile)
if (response.indexOf("İşlem başarılı") != -1) {
  isSuccess = true;
}

return (code == 200 && isSuccess);

}

void appendToFileWithCRC(const String& json) {
  File checkFile = ESPEASY_FS.open(backupFile, "r");
  if (!checkFile) return;
  int lineCount = 0;
  while (checkFile.available()) {
    checkFile.readStringUntil('\n');
    lineCount++;
  }
  checkFile.close();
  if (lineCount >= MAX_QUEUE) {
    File log = ESPEASY_FS.open(overflowLog, "a");
    if (log) {
      log.println(json);
      log.close();
    }
    return;
  }
  File file = ESPEASY_FS.open(backupFile, "a");
  if (file) {
    u32 crc = crc32((const uint8_t*)json.c_str(), json.length());
    file.printf("%s|%lu\n", json.c_str(), crc);
    file.close();
  }
}

void flushPendingQueueIfNeeded() {
  if (WiFi.status() != WL_CONNECTED) return;
  File file = ESPEASY_FS.open(backupFile, "r");
  if (!file) return;
  std::vector<String> stillPending;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.isEmpty()) continue;
    String json;
    if (!parseLine(line, json)) continue;
    if (!sendHttpPostFromParsedJson(json)) {
      stillPending.push_back(line);
      continue;
    }
  }
  file.close();
  file = ESPEASY_FS.open(backupFile, "w");
  for (String& line : stillPending) {
    file.println(line);
  }
  file.close();
}

void sendClickDataToServer() {
  StaticJsonDocument<256> doc;
  char buffer[20];
  doc["cihaz_id"] = Settings.Name;
  doc["sayac1"] = XML_SAYAC_1_S.toInt();
  doc["sayac2"] = XML_SAYAC_2_S.toInt();
  doc["sayac3"] = 0;
  doc["sayac4"] = 0;
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           node_time.year(), node_time.month(), node_time.day(),
           node_time.hour(), node_time.minute(), node_time.second());
  doc["zaman"] = buffer;
  String json;
  serializeJson(doc, json);
  flushPendingQueueIfNeeded();
  if (!sendHttpPostFromParsedJson(json)) appendToFileWithCRC(json);
}
