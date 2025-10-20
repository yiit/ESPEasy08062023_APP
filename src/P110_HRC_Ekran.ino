#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P110

#include "src/Commands/InternalCommands.h"
#include "src/ESPEasyCore/ESPEasyNetwork.h"
#include "src/ESPEasyCore/Controller.h"

#include "src/WebServer/ESPEasy_WebServer.h"
#include "src/WebServer/HTML_wrappers.h"
#include "src/WebServer/Markup.h"
#include "src/WebServer/Markup_Buttons.h"
#include "src/WebServer/Markup_Forms.h"
#include "src/WebServer/Lisans.h"
#include "src/WebServer/ToolsPage.h"
#include "src/WebServer/Rules.h"
#include "src/WebServer/LoadFromFS.h"

#include "src/Globals/CPlugins.h"
#include "src/Globals/Device.h"
#include "src/Globals/ExtraTaskSettings.h"
#include "src/Globals/Nodes.h"
#include "src/Globals/Plugins.h"
#include "src/Globals/Protocol.h"
#include "src/Globals/Settings.h"

#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/Memory.h"
#include "src/Helpers/StringConverter.h"
#include "src/Helpers/StringParser.h"
#include "src/Helpers/Networking.h"

#include "ESPEasy_common.h"
#include "ESPEasy-Globals.h"

#include "src/Helpers/web_api.h"

//#######################################################################################################
//##################################### Plugin 110: HRC  ################################################
//#######################################################################################################
#define PLUGIN_110
#define PLUGIN_ID_110 110
#define PLUGIN_NAME_110 "Screen - HRC"
#define PLUGIN_VALUENAME1_110 "NET"
#define PLUGIN_VALUENAME2_110 "DARA"
#define PLUGIN_VALUENAME3_110 "BRUT"
#define PLUGIN_VALUENAME4_110 "ADET"
#define PLUGIN_VALUENAME5_110 "ADETGR"
#define PLUGIN_VALUENAME6_110 "PLUNO"
#define PLUGIN_VALUENAME7_110 "B.FIYAT"
#define PLUGIN_VALUENAME8_110 "TUTAR"

#define HRC_Model ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define HRC_Indikator ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define HRC_Mod ExtraTaskSettings.TaskDevicePluginConfigLong[2]
//#define HRC_Gecikme ExtraTaskSettings.TaskDevicePluginConfigLong[3]
#define sayac1_pasif ExtraTaskSettings.TaskDevicePluginConfigLong[4]
#define sayac2_pasif ExtraTaskSettings.TaskDevicePluginConfigLong[5]
#define durus_gecikmesi ExtraTaskSettings.TaskDevicePluginConfigLong[6]
#define sayac3_pasif ExtraTaskSettings.TaskDevicePluginConfigLong[7]
#define sayac4_pasif ExtraTaskSettings.TaskDevicePluginConfigLong[8]

String tartimdata_110;
String mesaj1_data_110;
String mesaj2_data_110;
int hrc_stabil_sayisi;

unsigned long timer_110 = 0;
unsigned long timergecikme_110 = 3000;

bool mesaj1aktif_110 = true;
bool mesaj2aktif_110 = false;

String file_data_hrc;

#include "OneButton.h"
#include "EEPROM.h"

#ifdef ESP32
#if FEATURE_ETHERNET
//OneButton HRC_button1(14, true, true);
//OneButton HRC_button2(15, true, true);
OneButton HRC_button1(2, true, true);
OneButton HRC_button2(4, true, true);
OneButton HRC_button3(14, true, true);
OneButton HRC_button4(15, true, true);
#endif
#ifdef HAS_WIFI
OneButton HRC_button1(21, true, true);
OneButton HRC_button2(22, true, true);
#endif
#endif

boolean Plugin_110_init = false;
unsigned long sayac1_aktif_110 = 0;
unsigned long sayac2_aktif_110 = 0;
unsigned long sayac3_aktif_110 = 0;
unsigned long sayac4_aktif_110 = 0;

boolean durus_aktif_110 = true;
/*
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* serverUrl = "http://192.168.1.36:5000/api/Data/PostData";

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

bool isValidJson(const String& json) {
  StaticJsonDocument<256> doc;
  return !deserializeJson(doc, json);
}

bool sendHttpPost(const String& data) {
  HTTPClient http;
  http.begin(serverUrl);
  http.setTimeout(1000);  // ⏱️ Timeout süresi 1 saniye
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(data);
  String response = http.getString();
  http.end();

  char buf[256];
  snprintf(buf, sizeof(buf), "[HTTP] Kod: %d, Cevap: %s\n", code, response.c_str());
  String responseCode = String(buf);

  addLogMove(LOG_LEVEL_INFO, responseCode);

  if (code > 0 && code < 300) {
    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc, response);
    if (!err && doc["message"] == "İşlem başarılı.") {
      return true;
    } else {
      addLogMove(LOG_LEVEL_INFO,"[HATA] Beklenen yanıt alınamadı.");
    }
  } else if (code <= 0) {
    addLogMove(LOG_LEVEL_INFO,"[HATA] HTTP bağlantı başarısız");
  } else if (code >= 300) {
	snprintf(buf, sizeof(buf), "[HATA] Sunucu hata kodu: %d\n" + code);
	String responseCode = String(buf);

    addLogMove(LOG_LEVEL_INFO, responseCode);
  }

  return false;
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

    if (!sendHttpPost(json)) {
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

void sendData(String json) {
  if (!isValidJson(json)) return;
  flushPendingQueueIfNeeded();
  if (!sendHttpPost(json)) appendToFileWithCRC(json);
}

void sendClickDataToServer() {
  StaticJsonDocument<256> doc;
  doc["cihaz_id"] = "TZG01";
  doc["sayac1"] = XML_SAYAC_1_S.toInt();
  doc["sayac2"] = XML_SAYAC_2_S.toInt();
  doc["sayac3"] = 0;
  doc["sayac4"] = 0;

  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
           node_time.year(), node_time.month(), node_time.day(),
           node_time.hour(), node_time.minute(), node_time.second());
  doc["zaman"] = buffer;

  String json;
  serializeJson(doc, json);
  sendData(json);
}*/

void hrc_click1() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "hrcsayacart#");
  if (sayac1_aktif_110 < millis()) {
    int sayac = XML_SAYAC_1_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_1_S = String(sayac);
    dtostrf(XML_SAYAC_1_S.toInt(), 4, 0, XML_SAYAC_1_C);
    int sayac_sonsuz = XML_SAYAC_1_SONSUZ_S.toInt();
    XML_SAYAC_1_SONSUZ_S = String(sayac_sonsuz++);
    EEPROM.writeLong(100, uint32_t(XML_SAYAC_1_S.toInt()));
    EEPROM.writeLong(110, uint32_t(XML_SAYAC_1_SONSUZ_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_1_GECIKME_S = String(millis() - sayac1_aktif_110);
    sayac1_aktif_110 = millis() + sayac1_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
    sendClickDataToServer();  // 🔴 Veriyi sunucuya gönder
  }
}

void hrc_longPressStart1() {
  if (sayac1_aktif_110 < millis()) {
    int sayac = XML_SAYAC_1_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_1_S = String(sayac);
    dtostrf(XML_SAYAC_1_S.toInt(), 4, 0, XML_SAYAC_1_C);
    int sayac_sonsuz = XML_SAYAC_1_SONSUZ_S.toInt();
    XML_SAYAC_1_SONSUZ_S = String(sayac_sonsuz++);
    EEPROM.writeLong(100, uint32_t(XML_SAYAC_1_S.toInt()));
    EEPROM.writeLong(110, uint32_t(XML_SAYAC_1_SONSUZ_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_1_GECIKME_S = String(millis() - sayac1_aktif_110);
    sayac1_aktif_110 = millis() + sayac1_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
    sendClickDataToServer();  // 🔴 Veriyi sunucuya gönder
  }
}

/*void hrc_longPressStop1() {
  int sayac = XML_SAYAC_1_S.toInt();
  sayac = sayac + 1;
  XML_SAYAC_1_S = String(sayac);
  dtostrf(XML_SAYAC_1_S.toInt(), 4, 0, XML_SAYAC_1_C);
  EEPROM.writeLong(100, uint32_t(XML_SAYAC_1_S.toInt()));
  EEPROM.commit();
}*/

void hrc_click2() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "hrcsayacart");
  if (sayac2_aktif_110 < millis() ) {
    int sayac = XML_SAYAC_2_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_2_S = String(sayac);
    dtostrf(XML_SAYAC_2_S.toInt(), 4, 0, XML_SAYAC_2_C);
    EEPROM.writeLong(104, uint32_t(XML_SAYAC_2_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_2_GECIKME_S = String(millis() - sayac2_aktif_110);
    sayac2_aktif_110 = millis() + sayac2_pasif;
    sendClickDataToServer();  // 🔴 Veriyi sunucuya gönder
  }
}

void hrc_longPressStart2() {
  if (sayac2_aktif_110 < millis() ) {
    int sayac = XML_SAYAC_2_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_2_S = String(sayac);
    dtostrf(XML_SAYAC_2_S.toInt(), 4, 0, XML_SAYAC_2_C);
    EEPROM.writeLong(104, uint32_t(XML_SAYAC_2_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_2_GECIKME_S = String(millis() - sayac2_aktif_110);
    sayac2_aktif_110 = millis() + sayac2_pasif;
    sendClickDataToServer();  // 🔴 Veriyi sunucuya gönder
  }
}

/*void hrc_longPressStop2() {
  int sayac = XML_SAYAC_2_S.toInt();
  sayac = sayac + 1;
  XML_SAYAC_2_S = String(sayac);
  dtostrf(XML_SAYAC_2_S.toInt(), 4, 0, XML_SAYAC_2_C);
  EEPROM.writeLong(104, uint32_t(XML_SAYAC_2_S.toInt()));
  EEPROM.commit();
}*/

void hrc_click3() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "hrcsayacart#");
  if (sayac3_aktif_110 < millis()) {
    int sayac = XML_SAYAC_3_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_3_S = String(sayac);
    dtostrf(XML_SAYAC_3_S.toInt(), 4, 0, XML_SAYAC_3_C);
    EEPROM.writeLong(118, uint32_t(XML_SAYAC_3_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_3_GECIKME_S = String(millis() - sayac3_aktif_110);
    sayac3_aktif_110 = millis() + sayac3_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
    sendClickDataToServer();  // 🔴 Veriyi sunucuya gönder
  }
}

void hrc_longPressStart3() {
  if (sayac3_aktif_110 < millis()) {
    int sayac = XML_SAYAC_3_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_3_S = String(sayac);
    dtostrf(XML_SAYAC_3_S.toInt(), 4, 0, XML_SAYAC_3_C);
    EEPROM.writeLong(118, uint32_t(XML_SAYAC_3_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_3_GECIKME_S = String(millis() - sayac3_aktif_110);
    sayac3_aktif_110 = millis() + sayac3_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
    sendClickDataToServer();  // 🔴 Veriyi sunucuya gönder
  }
}
void hrc_click4() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "hrcsayacart#");
  if (sayac4_aktif_110 < millis()) {
    int sayac = XML_SAYAC_4_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_4_S = String(sayac);
    dtostrf(XML_SAYAC_4_S.toInt(), 4, 0, XML_SAYAC_4_C);
    EEPROM.writeLong(126, uint32_t(XML_SAYAC_4_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_4_GECIKME_S = String(millis() - sayac4_aktif_110);
    sayac4_aktif_110 = millis() + sayac4_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
    sendClickDataToServer();  // 🔴 Veriyi sunucuya gönder
  }
}

void hrc_longPressStart4() {
  if (sayac4_aktif_110 < millis()) {
    int sayac = XML_SAYAC_4_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_4_S = String(sayac);
    dtostrf(XML_SAYAC_4_S.toInt(), 4, 0, XML_SAYAC_4_C);
    EEPROM.writeLong(126, uint32_t(XML_SAYAC_4_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_4_GECIKME_S = String(millis() - sayac4_aktif_110);
    sayac4_aktif_110 = millis() + sayac4_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
    sendClickDataToServer();  // 🔴 Veriyi sunucuya gönder
  }
}

boolean Plugin_110(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_110;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        //Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 10;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_110);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_110));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_110));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_110));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_110));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_110));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_110));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_110));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_110));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormSubHeader(F("HRC Ayarları"));
        byte choice0 = HRC_Model;
        String options0[2];
        options0[0] = F("HRCMAXi");
        options0[1] = F("HRCMESAJ");
        int optionValues0[2] = {0, 1};
        addFormSelector(F("Model"), F("plugin_110_model"), 2, options0, optionValues0, choice0);
        /*byte choice1 = HRC_Mod;
        String options1[7];
        options1[0] = F("HRC TUŞ iLE");
        options1[1] = F("TERAZiDEN OTOMATiK");
        options1[2] = F("DENGELi OTOMATiK");
        options1[3] = F("TERAZiDEN TUŞ iLE");
        options1[4] = F("YAZICIDAN TUŞ iLE KONTROL(ÇOK SATIRLI VERi)");
        options1[5] = F("KUMANDA");
        options1[6] = F("VERi PAKETi");
        int optionValues1[6];
        optionValues1[0] = 0;
        optionValues1[1] = 1;
        optionValues1[2] = 2;
        optionValues1[3] = 3;
        optionValues1[4] = 4;
        optionValues1[5] = 5;
        optionValues1[6] = 6;
        addFormSelector(F("Mod"), F("plugin_110_mod"), 6, options1, optionValues1, choice1);
        addFormNumericBox(F("Gecikme Saniye"), F("plugin_110_gecikme"), HRC_Gecikme, 0, 999999);*/
        addFormNumericBox(F("Sayac1 Pasif MiliSaniye"), F("plugin_110_sayac1_pasif"), sayac1_pasif, 0, 999999);
        addFormNumericBox(F("Sayac2 Pasif MiliSaniye"), F("plugin_110_sayac2_pasif"), sayac2_pasif, 0, 999999);
        addFormNumericBox(F("Sayac3 Pasif MiliSaniye"), F("plugin_110_sayac3_pasif"), sayac3_pasif, 0, 999999);
        addFormNumericBox(F("Sayac4 Pasif MiliSaniye"), F("plugin_110_sayac4_pasif"), sayac4_pasif, 0, 999999);
        addFormNumericBox(F("Duruş Gecikmesi MiliSaniye"), F("plugin_110_durus_gecikmesi"), durus_gecikmesi, 0, 99999999);
#ifdef ESP8266
        fs::Dir filedata = ESPEASY_FS.openDir("rules");
        int fileno = 0;
        while (filedata.next()) {
          options2[fileno] = filedata.fileName();
          fileno++;
        }
#endif

#ifdef ESP32
          fs::File root = ESPEASY_FS.open(F("/rules"));
          fs::File file = root.openNextFile();
          int fileno = 0;
          while (file) {
            if (!file.isDirectory()) {
              const String fname(file.name());
              if (fname.startsWith(F("/hrc")) || fname.startsWith(F("hrc"))) {
                //int count = getCacheFileCountFromFilename(fname);
                options2[fileno] = "/rules/";
                options2[fileno] += file.name();
                fileno++;
              }
            }
            file = root.openNextFile();
          } 
#endif
        byte choice2 = ExtraTaskSettings.TaskDevicePrint[0];
        int optionValues2[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        addFormSelector(F("Mesaj1"), F("plugin_110_mesaj1"), 10, options2, optionValues2, choice2);                
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Hrc Dizayn Menüsüne Git"));    
        /*addFormSubHeader(F("İndikatör Ayarları"));
        indikator_secimi(event, HRC_Indikator, F("plugin_110_indikator"));
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));*/
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        HRC_Model = getFormItemInt(F("plugin_110_model"));
        HRC_Indikator = getFormItemInt(F("plugin_110_indikator"));
        //HRC_Mod = getFormItemInt(F("plugin_110_mod"));
        //HRC_Gecikme = getFormItemInt(F("plugin_110_gecikme"));
        sayac1_pasif = getFormItemInt(F("plugin_110_sayac1_pasif"));
        sayac2_pasif = getFormItemInt(F("plugin_110_sayac2_pasif"));
        sayac3_pasif = getFormItemInt(F("plugin_110_sayac3_pasif"));
        sayac4_pasif = getFormItemInt(F("plugin_110_sayac4_pasif"));
        durus_gecikmesi = getFormItemInt(F("plugin_110_durus_gecikmesi"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));
        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, HRC_Indikator, PCONFIG(4));

        ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_110_mesaj1"));
        options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.seridata_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        HRC_button1.attachClick(hrc_click1);
        //HRC_button1.attachDoubleClick(hrc_click1);
        HRC_button1.attachLongPressStart(hrc_longPressStart1);
        //HRC_button1.attachLongPressStop(hrc_longPressStop1);
        HRC_button2.attachClick(hrc_click2);
        //HRC_button2.attachDoubleClick(hrc_click2);
        HRC_button2.attachLongPressStart(hrc_longPressStart2);
        //HRC_button2.attachLongPressStop(hrc_longPressStop2);
        #if FEATURE_ETHERNET
        HRC_button3.attachClick(hrc_click3);
        //HRC_button1.attachDoubleClick(hrc_click1);
        HRC_button3.attachLongPressStart(hrc_longPressStart3);
        HRC_button4.attachClick(hrc_click4);
        //HRC_button1.attachDoubleClick(hrc_click1);
        HRC_button4.attachLongPressStart(hrc_longPressStart4);
        #endif
        HRC_button1.tick();
        HRC_button2.tick();
        #if FEATURE_ETHERNET
        HRC_button3.tick();
        HRC_button4.tick();
        #endif
        
        Serial1.println("\nsil");
#ifdef ESP32
#if FEATURE_ETHERNET
        if (!Plugin_110_init) {
          //delay(2000);
          //Serial1.println("mesHRCSAYAC ");
          //delay(2000);
          //Serial1.println("sil");
          //delay(200);
          Serial1.println("mesVER: 4.2 ");
          delay(2000);
          //Serial1.println("sil");
          //delay(200);
          Serial1.print("mes");
          Serial1.print(" ");
          Serial1.print(String(static_cast<int>(NetworkLocalIP()[0])));
          Serial1.print(".");
          Serial1.print(String(static_cast<int>(NetworkLocalIP()[1])));
          Serial1.println();
          delay(2000);
          //Serial1.println("sil");
          Serial1.print("mes");
          Serial1.print(" ");
          Serial1.print(String(static_cast<int>(NetworkLocalIP()[2])));
          Serial1.print(".");
          Serial1.print(String(static_cast<int>(NetworkLocalIP()[3])));
          Serial1.println();
          delay(2000);
          Serial1.println("sil");
        }
#endif
#endif
        /*if (fileExists(Settings.seridata_prn)) {
          fs::File form = tryOpenFile(Settings.seridata_prn, "r");
          while (form.position() < form.size()) {
            file_data_hrc = form.readStringUntil('\r');
            //file_data_hrc.trim();
          }
          form.close();
        }*/
        Settings.WebAPP = 110;
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        /*
        XML_TARIH_S = node_time.getDateString('-');
        XML_SAAT_S = node_time.getTimeString(':');
        if (fileExists(Settings.seridata_prn)) {
          fs::File mesaj1 = tryOpenFile(Settings.seridata_prn, "r");
          mesaj1_data_110 = mesaj1.readStringUntil('\n');
        }
        if (fileExists(Settings.art_prn)) {
          fs::File mesaj2 = tryOpenFile(Settings.art_prn, "r");
          mesaj2_data_110 = mesaj2.readStringUntil('\n');
        }
        parseSystemVariables(mesaj1_data_110, false);
        parseSystemVariables(mesaj2_data_110, false);
        */
        Serial1.print(mesaj1_data_110);
        Serial1.flush();
        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        HRC_button1.tick();
        HRC_button2.tick();
        #if FEATURE_ETHERNET
        HRC_button3.tick();
        HRC_button4.tick();
        #endif
        long durus_zamani = millis() - sayac1_aktif_110;
        if ((durus_zamani > durus_gecikmesi) && (durus_aktif_110)) {
          XML_DURUS_ZAMANI_S = node_time.getDateTimeString('-',':',' ');
          durus_aktif_110 = false;
        }
        else if ((durus_zamani < durus_gecikmesi) && (!durus_aktif_110))
          durus_aktif_110 = true;
        //char XML_NET_C[9];
        //char XML_DARA_C[9];
        //char XML_BRUT_C[9];
        //String s = tartimdata;
        //if (XML_NET_S == "ERROR")
        //  s = "mesWF HATA ";
        //else {
        //dtostrf(XML_NET_S.toFloat(),  (Settings.net_bitis_byte -  Settings.net_bas_byte),  Settings.nokta_byte, XML_NET_C);
        //dtostrf(XML_DARA_S.toFloat(), (Settings.dara_bitis_byte - Settings.dara_bas_byte), Settings.nokta_byte, XML_DARA_C);
        //dtostrf(XML_BRUT_S.toFloat(), (Settings.net_bitis_byte -  Settings.net_bas_byte),  Settings.nokta_byte, XML_BRUT_C);
        //switch (HRC_Mod) {
            //case 0:
        /*if (millis() > stabilTimer_l) {
          stabilTimer_l = millis() + 200;
          stabilTartim_s = XML_NET_S;
          if (hrc_stabil_sayisi >= 2)
            //hrc_stabil_sayisi = 0;
            XML_STABIL_S = "()";
          else
            XML_STABIL_S = "(>";
        } else {
          if (stabilTartim_s == XML_NET_S)
            hrc_stabil_sayisi++;
          else
            hrc_stabil_sayisi = 0;
          }
        //}
        timer = millis();
        if ((timer > timergecikme) && (mesaj1aktif_110)) {
          timergecikme = timer + HRC_Gecikme;
          mesaj1aktif_110 = false;
          mesaj2aktif = true;
        }
        if (mesaj1aktif_110)
          Serial.println(mesaj1_data_110);
        if ((timer > timergecikme) && (mesaj2aktif)) {
          timergecikme = timer + HRC_Gecikme;
          mesaj2aktif = false;
          mesaj1aktif_110 = true;
        }
        if (mesaj2aktif)
          Serial.println(mesaj2_data_110);
        //delay(HRC_Gecikme);
        Serial.flush();*/
        success = true;
        break;
      }

      case PLUGIN_ONCE_A_SECOND:
      {
        if (!Plugin_110_init)
          Plugin_110_init = true;
        XML_TARIH_S = node_time.getDateString('-');
        XML_SAAT_S = node_time.getTimeString(':');
        /*if (fileExists(Settings.seridata_prn)) {
          fs::File mesaj1 = tryOpenFile(Settings.seridata_prn, "r");
          mesaj1_data_110 = mesaj1.readStringUntil('\n');
          mesaj1.close();
        }*/
        //try
        //{
          fs::File mesaj1 = tryOpenFile(Settings.seridata_prn, "r");
          mesaj1_data_110 = mesaj1.readStringUntil('\n');
          mesaj1.close();
        //}
        /*catch(...)
        {
          
        }*/
        
        /*if (fileExists(Settings.art_prn)) {
          fs::File mesaj2 = tryOpenFile(Settings.art_prn, "r");
          mesaj2_data_110 = mesaj2.readStringUntil('\n');
        }*/
        parseSystemVariables(mesaj1_data_110, false);

        //serial_error(event, HRC_Mod, "hata");
        success = true;
        break;
      }

      case PLUGIN_WRITE:
      {
        //string.replace("\r","");
        //string.replace("\n","");
        //string.replace(String(char(Settings.son_byte)), "");
        XML_NET_S = string;
        String file_data = file_data_hrc;
        parseSystemVariables(file_data, false);
        if (string.length() > 1)
         Serial1.print(file_data);
        success = true;
        break;
      }

      /*case PLUGIN_SERIAL_IN:
      {
        //float bol = 1;
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255)  // binary data...
          {
            Serial.flush();
            break;
          }
          tartimString_s += (String)inChar;
          if (inChar == Settings.son_byte) {
            hataTimer_l = millis();
            //if (Settings.Tersle)
            //tersle();
            isaret(HRC_Indikator, tartimString_s);
            //if ((HRC_Mod == 3) || (HRC_Mod == 4))
            //formul_kontrol(event, HRC_Mod, true);
            //else {
            webapinettartim = isaret_f * (tartimString_s.substring(Settings.net_bas_byte, Settings.net_bitis_byte).toFloat());
            webapidaratartim = tartimString_s.substring(Settings.dara_bas_byte, Settings.dara_bitis_byte).toFloat();
            webapibruttartim = webapidaratartim + webapinettartim;
            webapiadet = (tartimString_s.substring(Settings.adet_bas_byte, Settings.adet_bitis_byte).toFloat());
            //webapiadetgr = (tartimString_s.substring(Settings.adetgr_bas_byte, Settings.adetgr_bitis_byte).toFloat());
            int carpan = 100;
              switch (int(ExtraTaskSettings.TaskDeviceValueDecimals[5])) {
                case 0: carpan = 1; break;
                case 1: carpan = 10; break;
                case 2: carpan = 100; break;
                case 3: carpan = 1000; break;
              }
            //int pluno = (tartimString_s.substring(Settings.pluno_bas_byte, Settings.pluno_bitis_byte).toFloat()) * carpan;
            //webapipluno = pluno;
            //webapibfiyat = (tartimString_s.substring(Settings.bfiyat_bas_byte, Settings.bfiyat_bitis_byte).toFloat());
            //webapitutar = (tartimString_s.substring(Settings.tutar_bas_byte, Settings.tutar_bitis_byte).toFloat());
            UserVar[event->BaseVarIndex] = webapinettartim;
            UserVar[event->BaseVarIndex + 1] = webapidaratartim;
            UserVar[event->BaseVarIndex + 2] = webapibruttartim;
            UserVar[event->BaseVarIndex + 3] = webapiadet;
            //UserVar[event->BaseVarIndex + 4] = webapiadetgr;
            //UserVar[event->BaseVarIndex + 5] = webapipluno;
            //UserVar[event->BaseVarIndex + 6] = webapibfiyat;
            //UserVar[event->BaseVarIndex + 7] = webapitutar;
            if ((HRC_Indikator == 20) || (HRC_Indikator == 3) || (HRC_Indikator == 31)) {
                switch (Settings.nokta_byte) {
                  case 0: bol = 1; break;
                  case 1: bol = 10; break;
                  case 2: bol = 100; break;
                  case 3: bol = 1000; break;
                }
              }
            XML_NET_S = String((webapinettartim), Settings.nokta_byte);
            XML_DARA_S = String((webapidaratartim), Settings.nokta_byte);
            XML_BRUT_S = String((webapibruttartim), Settings.nokta_byte);
            XML_ADET_S = String(webapiadet, 0);
            //XML_ADET_GRAMAJ_S = String(webapiadetgr, 0);
            //XML_PLU_NO_S = String(webapipluno, 0);
            //XML_BIRIM_FIYAT_S = String(webapibfiyat, 0);
            //XML_TUTAR_S = String(webapitutar, 0);
            //XML_TARIH_S = node_time.getDateString('-');
            //XML_SAAT_S = node_time.getTimeString(':');
            dtostrf(XML_NET_S.toFloat(), (Settings.net_bitis_byte - Settings.net_bas_byte), Settings.nokta_byte, XML_NET_C);
            dtostrf(XML_DARA_S.toFloat(), (Settings.dara_bitis_byte - Settings.dara_bas_byte), Settings.nokta_byte, XML_DARA_C);
            dtostrf(XML_BRUT_S.toFloat(), (Settings.net_bitis_byte - Settings.net_bas_byte), Settings.nokta_byte, XML_BRUT_C);
            dtostrf(XML_ADET_S.toFloat(), (Settings.adet_bitis_byte - Settings.adet_bas_byte), 0, XML_ADET_C);
            //}
            tartimString_s = "";
            Serial.flush();
          }
        }
        success = true;
        break;
      }*/
  }
  return success;
}
#endif