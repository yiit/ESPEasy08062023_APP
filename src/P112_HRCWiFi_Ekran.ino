#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P112

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

#include "MD_Parola.h"
#include "MD_MAX72xx.h"

#include "src/Helpers/web_api.h"

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
//#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

// Defining size, and output pins
#define MAX_DEVICES 5

#define CLK_PIN   14  // SCK
#define DATA_PIN  13  // MOSI
#define CS_PIN    27  // CS
// MISO 25
String hrcmesaj_degisken = "";
bool baglanti_yok = false;

MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

MD_MAX72XX::fontType_t newFont[] PROGMEM = 
{
	0, 	// 0
	0, 	// 1
	0, 	// 2
	0, 	// 3
	0, 	// 4
	0, 	// 5
	0, 	// 6
	0, 	// 7
	0, 	// 8
	0, 	// 9
	0, 	// 10
	0, 	// 11
	0, 	// 12
	0, 	// 13
	0, 	// 14
	0, 	// 15
	0, 	// 16
	0, 	// 17
	0, 	// 18
	0, 	// 19
	0, 	// 20
	0, 	// 21
	0, 	// 22
	0, 	// 23
	0, 	// 24
	0, 	// 25
	0, 	// 26
	0, 	// 27
	0, 	// 28
	0, 	// 29
	0, 	// 30
	0, 	// 31
	5, 0, 0, 0, 0, 0, 	// 32
	0, 	// 33
	0, 	// 34
	0, 	// 35
	0, 	// 36
	0, 	// 37
	0, 	// 38
	0, 	// 39
	0, 	// 40
	0, 	// 41
	0, 	// 42
	0, 	// 43
	0, 	// 44
	0, 	// 45
	2, 192, 192, 	// 46
	0, 	// 47
	5, 126, 255, 195, 255, 126, 	// 48
	5, 0, 4, 6, 255, 255, 	// 49sayı
	5, 198, 231, 243, 223, 206, 	// 50
	5, 66, 195, 137, 255, 118, 	// 51
	5, 112, 120, 76, 254, 255, 	// 52
	5, 95, 223, 155, 251, 115, 	// 53
	5, 126, 255, 137, 251, 114, 	// 54
	5, 3, 195, 243, 63, 15, 	// 55
	5, 110, 255, 219, 255, 110, 	// 56
	5, 78, 223, 155, 255, 126, 	// 57
	0, 	// 58
	0, 	// 59
	0, 	// 60
	0, 	// 61
	0, 	// 62
	0, 	// 63
	0, 	// 64
	0, 	// 65
	0, 	// 66
	0, 	// 67
	0, 	// 68
	0, 	// 69
	0, 	// 70
	0, 	// 71
	0, 	// 72
	0, 	// 73
	0, 	// 74
	0, 	// 75
	0, 	// 76
	0, 	// 77
	0, 	// 78
	0, 	// 79
	0, 	// 80
	0, 	// 81
	0, 	// 82
	0, 	// 83
	0, 	// 84
	0, 	// 85
	0, 	// 86
	0, 	// 87
	0, 	// 88
	0, 	// 89
	0, 	// 90
	0, 	// 91
	0, 	// 92
	0, 	// 93
	0, 	// 94
	0, 	// 95
	0, 	// 96
	0, 	// 97
	0, 	// 98
	0, 	// 99
	0, 	// 100
	0, 	// 101
	0, 	// 102
	0, 	// 103
	0, 	// 104
	0, 	// 105
	0, 	// 106
	0, 	// 107
	0, 	// 108
	0, 	// 109
	0, 	// 110
	0, 	// 111
	0, 	// 112
	0, 	// 113
	0, 	// 114
	0, 	// 115
	0, 	// 116
	0, 	// 117
	0, 	// 118
	0, 	// 119
	0, 	// 120
	0, 	// 121
	0, 	// 122
	0, 	// 123
	0, 	// 124
	0, 	// 125
	0, 	// 126
	0, 	// 127
	0, 	// 128
	0, 	// 129
	0, 	// 130
	0, 	// 131
	0, 	// 132
	0, 	// 133
	0, 	// 134
	0, 	// 135
	0, 	// 136
	0, 	// 137
	0, 	// 138
	0, 	// 139
	0, 	// 140
	0, 	// 141
	0, 	// 142
	0, 	// 143
	0, 	// 144
	0, 	// 145
	0, 	// 146
	0, 	// 147
	0, 	// 148
	0, 	// 149
	0, 	// 150
	0, 	// 151
	0, 	// 152
	0, 	// 153
	0, 	// 154
	0, 	// 155
	0, 	// 156
	0, 	// 157
	0, 	// 158
	0, 	// 159
	0, 	// 160
	0, 	// 161
	0, 	// 162
	0, 	// 163
	0, 	// 164
	0, 	// 165
	0, 	// 166
	0, 	// 167
	0, 	// 168
	0, 	// 169
	0, 	// 170
	0, 	// 171
	0, 	// 172
	0, 	// 173
	0, 	// 174
	0, 	// 175
	0, 	// 176
	0, 	// 177
	0, 	// 178
	0, 	// 179
	0, 	// 180
	0, 	// 181
	0, 	// 182
	0, 	// 183
	0, 	// 184
	0, 	// 185
	0, 	// 186
	0, 	// 187
	0, 	// 188
	0, 	// 189
	0, 	// 190
	0, 	// 191
	0, 	// 192
	0, 	// 193
	0, 	// 194
	0, 	// 195
	0, 	// 196
	0, 	// 197
	0, 	// 198
	0, 	// 199
	0, 	// 200
	0, 	// 201
	0, 	// 202
	0, 	// 203
	0, 	// 204
	0, 	// 205
	0, 	// 206
	0, 	// 207
	0, 	// 208
	0, 	// 209
	0, 	// 210
	0, 	// 211
	0, 	// 212
	0, 	// 213
	0, 	// 214
	0, 	// 215
	0, 	// 216
	0, 	// 217
	0, 	// 218
	0, 	// 219
	0, 	// 220
	0, 	// 221
	0, 	// 222
	0, 	// 223
	0, 	// 224
	0, 	// 225
	0, 	// 226
	0, 	// 227
	0, 	// 228
	0, 	// 229
	0, 	// 230
	0, 	// 231
	0, 	// 232
	0, 	// 233
	0, 	// 234
	0, 	// 235
	0, 	// 236
	0, 	// 237
	0, 	// 238
	0, 	// 239
	0, 	// 240
	0, 	// 241
	0, 	// 242
	0, 	// 243
	0, 	// 244
	0, 	// 245
	0, 	// 246
	0, 	// 247
	0, 	// 248
	0, 	// 249
	0, 	// 250
	0, 	// 251
	0, 	// 252
	0, 	// 253
	0, 	// 254
	0, 	// 255
};

MD_MAX72XX::fontType_t dotmatrix_5x8[] PROGMEM = 
{
	0, 	// 0  
	0, 	// 1  
	0, 	// 2  
	0, 	// 3  
	0, 	// 4  
	0, 	// 5  
	0, 	// 6  
	0, 	// 7  
	0, 	// 8  
	0, 	// 9  
	0, 	// 10  
	0, 	// 11  
	0, 	// 12  
	0, 	// 13  
	0, 	// 14  
	0, 	// 15  
	0, 	// 16  
	0, 	// 17  
	0, 	// 18  
	0, 	// 19  
	0, 	// 20  
	0, 	// 21  
	0, 	// 22  
	0, 	// 23  
	0, 	// 24  
	0, 	// 25  
	0, 	// 26  
	0, 	// 27  
	0, 	// 28  
	0, 	// 29  
	0, 	// 30  
	0, 	// 31  
	5, 0, 0, 0, 0, 0, 	// 32  
	0, 	// 33  
	0, 	// 34  
	0, 	// 35  
	0, 	// 36  
	0, 	// 37  
	0, 	// 38  
	0, 	// 39  
	0, 	// 40  
	0, 	// 41  
	0, 	// 42  
	0, 	// 43  
	0, 	// 44  
	5, 16, 16, 16, 16, 16, 	// 45  
	1, 128, 	// 46  
	0, 	// 47  
	5, 126, 129, 129, 129, 126, 	// 48  
	5, 0, 0, 2, 255, 0, 	// 49  sayı
	5, 242, 137, 137, 137, 134, 	// 50  
	5, 66, 129, 137, 137, 118, 	// 51  
	5, 56, 36, 34, 255, 32, 	// 52  
	5, 79, 137, 137, 137, 113, 	// 53  
	5, 126, 137, 137, 137, 114, 	// 54  
	5, 1, 1, 225, 25, 7, 	// 55  
	5, 118, 137, 137, 137, 118, 	// 56  
	5, 70, 137, 137, 137, 126, 	// 57  
	0, 	// 58  
	0, 	// 59  
	0, 	// 60  
	0, 	// 61  
	0, 	// 62  
	0, 	// 63  
	0, 	// 64  
	6, 252, 34, 33, 34, 252, 0, 	// 65  
	5, 255, 137, 137, 137, 118, 	// 66  
	5, 126, 129, 129, 129, 66, 	// 67  
	5, 255, 129, 129, 129, 126, 	// 68  
	5, 255, 137, 137, 137, 137, 	// 69  
	5, 255, 9, 9, 9, 1, 	// 70  
	5, 126, 129, 145, 145, 98, 	// 71  
	5, 255, 16, 16, 16, 255, 	// 72  
	5, 0, 0, 255, 0, 0, 	// 73  
	5, 64, 128, 128, 128, 127, 	// 74  
	5, 255, 8, 20, 34, 193, 	// 75  
	5, 255, 128, 128, 128, 128, 	// 76  
	5, 255, 6, 24, 6, 255, 	// 77  
	5, 255, 6, 24, 96, 255, 	// 78  
	5, 126, 129, 129, 129, 126, 	// 79  
	5, 255, 17, 17, 17, 14, 	// 80  
	0, 	// 81  
	5, 255, 17, 49, 81, 142, 	// 82  
	5, 70, 137, 137, 137, 114, 	// 83  
	5, 1, 1, 255, 1, 1, 	// 84  
	5, 127, 128, 128, 128, 127, 	// 85  
	5, 7, 56, 192, 56, 7, 	// 86  
	0, 	// 87  
	0, 	// 88  
	5, 3, 12, 240, 12, 3, 	// 89  
	5, 225, 145, 137, 133, 131, 	// 90  
	0, 	// 91  
	0, 	// 92  
	0, 	// 93  
	0, 	// 94  
	0, 	// 95  
	0, 	// 96  
	0, 	// 97  
	0, 	// 98  
	0, 	// 99  
	0, 	// 100  
	0, 	// 101  
	0, 	// 102  
	0, 	// 103  
	0, 	// 104  
	0, 	// 105  
	0, 	// 106  
	0, 	// 107  
	0, 	// 108  
	0, 	// 109  
	0, 	// 110  
	0, 	// 111  
	0, 	// 112  
	0, 	// 113  
	0, 	// 114  
	0, 	// 115  
	0, 	// 116  
	0, 	// 117  
	0, 	// 118  
	0, 	// 119  
	0, 	// 120  
	0, 	// 121  
	0, 	// 122  
	0, 	// 123  
	0, 	// 124  
	0, 	// 125  
	0, 	// 126  
	0, 	// 127  
	0, 	// 128  
	0, 	// 129  
	0, 	// 130  
	0, 	// 131  
	0, 	// 132  
	0, 	// 133  
	0, 	// 134  
	0, 	// 135  
	0, 	// 136  
	0, 	// 137  
	0, 	// 138  
	0, 	// 139  
	0, 	// 140  
	0, 	// 141  
	0, 	// 142  
	0, 	// 143  
	0, 	// 144  
	0, 	// 145  
	0, 	// 146  
	0, 	// 147  
	0, 	// 148  
	0, 	// 149  
	0, 	// 150  
	0, 	// 151  
	0, 	// 152  
	0, 	// 153  
	0, 	// 154  
	0, 	// 155  
	0, 	// 156  
	0, 	// 157  
	0, 	// 158  
	0, 	// 159  
	0, 	// 160  
	0, 	// 161  
	0, 	// 162  
	0, 	// 163  
	0, 	// 164  
	0, 	// 165  
	0, 	// 166  
	0, 	// 167  
	0, 	// 168  
	0, 	// 169  
	0, 	// 170  
	0, 	// 171  
	0, 	// 172  
	0, 	// 173  
	0, 	// 174  
	0, 	// 175  
	0, 	// 176  
	0, 	// 177  
	0, 	// 178  
	0, 	// 179  
	0, 	// 180  
	0, 	// 181  
	0, 	// 182  
	0, 	// 183  
	0, 	// 184  
	0, 	// 185  
	0, 	// 186  
	0, 	// 187  
	0, 	// 188  
	0, 	// 189  
	0, 	// 190  
	0, 	// 191  
	0, 	// 192  
	0, 	// 193  
	0, 	// 194  
	0, 	// 195  
	0, 	// 196  
	0, 	// 197  
	0, 	// 198  
	0, 	// 199  
	0, 	// 200  
	0, 	// 201  
	0, 	// 202  
	0, 	// 203  
	0, 	// 204  
	0, 	// 205  
	0, 	// 206  
	0, 	// 207  
	0, 	// 208  
	0, 	// 209  
	0, 	// 210  
	0, 	// 211  
	0, 	// 212  
	0, 	// 213  
	0, 	// 214  
	0, 	// 215  
	0, 	// 216  
	0, 	// 217  
	0, 	// 218  
	0, 	// 219  
	0, 	// 220  
	0, 	// 221  
	0, 	// 222  
	0, 	// 223  
	0, 	// 224  
	0, 	// 225  
	0, 	// 226  
	0, 	// 227  
	0, 	// 228  
	0, 	// 229  
	0, 	// 230  
	0, 	// 231  
	0, 	// 232  
	0, 	// 233  
	0, 	// 234  
	0, 	// 235  
	0, 	// 236  
	0, 	// 237  
	0, 	// 238  
	0, 	// 239  
	0, 	// 240  
	0, 	// 241  
	0, 	// 242  
	0, 	// 243  
	0, 	// 244  
	0, 	// 245  
	0, 	// 246  
	0, 	// 247  
	0, 	// 248  
	0, 	// 249  
	0, 	// 250  
	0, 	// 251  
	0, 	// 252  
	0, 	// 253  
	0, 	// 254  
	0, 	// 255
};

//#######################################################################################################
//##################################### Plugin 112: HRC  ################################################
//#######################################################################################################

#define PLUGIN_112
#define PLUGIN_ID_112 112
#define PLUGIN_NAME_112 "Screen - HRCWiFi"
#define PLUGIN_VALUENAME1_112 "NET"
#define PLUGIN_VALUENAME2_112 "DARA"
#define PLUGIN_VALUENAME3_112 "BRUT"
#define PLUGIN_VALUENAME4_112 "ADET"
#define PLUGIN_VALUENAME5_112 "ADETGR"
#define PLUGIN_VALUENAME6_112 "PLUNO"
#define PLUGIN_VALUENAME7_112 "B.FIYAT"
#define PLUGIN_VALUENAME8_112 "TUTAR"

#define HRCWiFi_Model ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define HRCWiFi_Indikator ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define HRCWiFi_Mod ExtraTaskSettings.TaskDevicePluginConfigLong[2]
#define HRCWiFi_Parlaklik ExtraTaskSettings.TaskDevicePluginConfigLong[3]

String tartimdatahrcwifi;
int hrcwifi_stabil_sayisi;


//#define HRC_Gecikme ExtraTaskSettings.TaskDevicePluginConfigLong[3]
#define sayac1_pasif ExtraTaskSettings.TaskDevicePluginConfigLong[4]
#define sayac2_pasif ExtraTaskSettings.TaskDevicePluginConfigLong[5]
#define durus_gecikmesi ExtraTaskSettings.TaskDevicePluginConfigLong[6]
#define sayac3_pasif ExtraTaskSettings.TaskDevicePluginConfigLong[7]
#define sayac4_pasif ExtraTaskSettings.TaskDevicePluginConfigLong[8]

String tartimdata_112;
String mesaj1_data_112;
String mesaj2_data_112;

unsigned long timer_112 = 0;
unsigned long timergecikme_112 = 3000;

bool mesaj1aktif_112 = true;
bool mesaj2aktif_112 = false;

String file_data_hrc_112;

#include "OneButton.h"
#include "EEPROM.h"

#ifdef ESP32
#if FEATURE_ETHERNET
//OneButton HRC_button1_112(14, true, true);
//OneButton HRC_button2_112(15, true, true);
OneButton HRC_button1_112(2, true, true);
OneButton HRC_button2_112(4, true, true);
OneButton HRC_button3_112(14, true, true);
OneButton HRC_button4_112(15, true, true);
#endif
#ifdef HAS_WIFI
OneButton HRC_button1_112(21, true, true);
OneButton HRC_button2_112(22, true, true);
#endif
#ifdef HAS_BLE
OneButton hrcwifi_button1(21, true, true);
OneButton hrcwifi_button2(22, true, true);
#endif
#endif

boolean Plugin_112_init = false;
unsigned long sayac1_aktif_112 = 0;
unsigned long sayac2_aktif_112 = 0;
unsigned long sayac3_aktif_112 = 0;
unsigned long sayac4_aktif_112 = 0;

boolean durus_aktif_112 = true;
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
}
*/
void hrcwifi_click1() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "hrcwifisayacart#");
  if (sayac1_aktif_112 < millis()) {
    int sayac = XML_SAYAC_1_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_1_S = String(sayac);
    dtostrf(XML_SAYAC_1_S.toInt(), 4, 0, XML_SAYAC_1_C);
    int sayac_sonsuz = XML_SAYAC_1_SONSUZ_S.toInt();
    XML_SAYAC_1_SONSUZ_S = String(sayac_sonsuz++);
    EEPROM.writeLong(100, uint32_t(XML_SAYAC_1_S.toInt()));
    EEPROM.writeLong(110, uint32_t(XML_SAYAC_1_SONSUZ_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_1_GECIKME_S = String(millis() - sayac1_aktif_112);
    sayac1_aktif_112 = millis() + sayac1_pasif;
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

void hrcwifi_longPressStart1() {
  if (sayac1_aktif_112 < millis()) {
    int sayac = XML_SAYAC_1_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_1_S = String(sayac);
    dtostrf(XML_SAYAC_1_S.toInt(), 4, 0, XML_SAYAC_1_C);
    int sayac_sonsuz = XML_SAYAC_1_SONSUZ_S.toInt();
    XML_SAYAC_1_SONSUZ_S = String(sayac_sonsuz++);
    EEPROM.writeLong(100, uint32_t(XML_SAYAC_1_S.toInt()));
    EEPROM.writeLong(110, uint32_t(XML_SAYAC_1_SONSUZ_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_1_GECIKME_S = String(millis() - sayac1_aktif_112);
    sayac1_aktif_112 = millis() + sayac1_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
	sendClickDataToServer();
  }
}

/*void hrcwifi_longPressStop1() {
  int sayac = XML_SAYAC_1_S.toInt();
  sayac = sayac + 1;
  XML_SAYAC_1_S = String(sayac);
  dtostrf(XML_SAYAC_1_S.toInt(), 4, 0, XML_SAYAC_1_C);
  EEPROM.writeLong(100, uint32_t(XML_SAYAC_1_S.toInt()));
  EEPROM.commit();
}*/

void hrcwifi_click2() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "hrcwifisayacart");
  if (sayac2_aktif_112 < millis() ) {
    int sayac = XML_SAYAC_2_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_2_S = String(sayac);
    dtostrf(XML_SAYAC_2_S.toInt(), 4, 0, XML_SAYAC_2_C);
    EEPROM.writeLong(104, uint32_t(XML_SAYAC_2_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_2_GECIKME_S = String(millis() - sayac2_aktif_112);
    sayac2_aktif_112 = millis() + sayac2_pasif;
	sendClickDataToServer();
  }
}

void hrcwifi_longPressStart2() {
  if (sayac2_aktif_112 < millis() ) {
    int sayac = XML_SAYAC_2_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_2_S = String(sayac);
    dtostrf(XML_SAYAC_2_S.toInt(), 4, 0, XML_SAYAC_2_C);
    EEPROM.writeLong(104, uint32_t(XML_SAYAC_2_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_2_GECIKME_S = String(millis() - sayac2_aktif_112);
    sayac2_aktif_112 = millis() + sayac2_pasif;
	sendClickDataToServer();
  }
}

/*void hrcwifi_longPressStop2() {
  int sayac = XML_SAYAC_2_S.toInt();
  sayac = sayac + 1;
  XML_SAYAC_2_S = String(sayac);
  dtostrf(XML_SAYAC_2_S.toInt(), 4, 0, XML_SAYAC_2_C);
  EEPROM.writeLong(104, uint32_t(XML_SAYAC_2_S.toInt()));
  EEPROM.commit();
}*/

void hrcwifi_click3() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "hrcwifisayacart#");
  if (sayac3_aktif_112 < millis()) {
    int sayac = XML_SAYAC_3_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_3_S = String(sayac);
    dtostrf(XML_SAYAC_3_S.toInt(), 4, 0, XML_SAYAC_3_C);
    EEPROM.writeLong(118, uint32_t(XML_SAYAC_3_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_3_GECIKME_S = String(millis() - sayac3_aktif_112);
    sayac3_aktif_112 = millis() + sayac3_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
	sendClickDataToServer();
  }
}

void hrcwifi_longPressStart3() {
  if (sayac3_aktif_112 < millis()) {
    int sayac = XML_SAYAC_3_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_3_S = String(sayac);
    dtostrf(XML_SAYAC_3_S.toInt(), 4, 0, XML_SAYAC_3_C);
    EEPROM.writeLong(118, uint32_t(XML_SAYAC_3_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_3_GECIKME_S = String(millis() - sayac3_aktif_112);
    sayac3_aktif_112 = millis() + sayac3_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
	sendClickDataToServer();
  }
}
void hrcwifi_click4() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "hrcwifisayacart#");
  if (sayac4_aktif_112 < millis()) {
    int sayac = XML_SAYAC_4_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_4_S = String(sayac);
    dtostrf(XML_SAYAC_4_S.toInt(), 4, 0, XML_SAYAC_4_C);
    EEPROM.writeLong(126, uint32_t(XML_SAYAC_4_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_4_GECIKME_S = String(millis() - sayac4_aktif_112);
    sayac4_aktif_112 = millis() + sayac4_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
	sendClickDataToServer();
  }
}

void hrcwifi_longPressStart4() {
  if (sayac4_aktif_112 < millis()) {
    int sayac = XML_SAYAC_4_S.toInt();
    sayac = sayac + 1;
    XML_SAYAC_4_S = String(sayac);
    dtostrf(XML_SAYAC_4_S.toInt(), 4, 0, XML_SAYAC_4_C);
    EEPROM.writeLong(126, uint32_t(XML_SAYAC_4_S.toInt()));
    EEPROM.commit();
    XML_SAYAC_4_GECIKME_S = String(millis() - sayac4_aktif_112);
    sayac4_aktif_112 = millis() + sayac4_pasif;
    /*String dosya = "/";
    dosya += String(node_time.month());
    dosya += "_" + String(node_time.year());
    dosya += ".csv";
    //if (fileExists(dosya)) {
    fs::File logFile = ESPEASY_FS.open(dosya, "a+");
    if (logFile)
      logFile.println(node_time.getDateString('.')+","+node_time.getTimeString(':')+","+XML_SAYAC_1_S+",LONG");
    logFile.close();*/
	sendClickDataToServer();
  }
}

boolean Plugin_112(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_112;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        //Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 8;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_112);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_112));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_112));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_112));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_112));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_112));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_112));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_112));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_112));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice0 = HRCWiFi_Model;
        String options0[1];
        options0[0] = F("HRCMINI");
        //options0[1] = F("HRCMESAJ");
        int optionValues0[1];
        optionValues0[0] = 0;
        //optionValues0[1] = 1;
        addFormSelector(F("MODEL"), F("plugin_112_model"), 1, options0, optionValues0, choice0);
        addFormNumericBox(F("Sayac1 Pasif MiliSaniye"), F("plugin_112_sayac1_pasif"), sayac1_pasif, 0, 999999);
        addFormNumericBox(F("Sayac2 Pasif MiliSaniye"), F("plugin_112_sayac2_pasif"), sayac2_pasif, 0, 999999);
        addFormNumericBox(F("Duruş Gecikmesi MiliSaniye"), F("plugin_112_durus_gecikmesi"), durus_gecikmesi, 0, 99999999);

		addFormSubHeader(F("HRC AYARLARI"));
        indikator_secimi(event, HRCWiFi_Indikator, F("plugin_112_indikator"));
        byte choice1 = HRCWiFi_Mod;
        String options1[10];
        options1[0] = F("SÜREKLi VERi (TEK SATIRLI VERi)");
        options1[1] = F("TERAZiDEN OTOMATiK (TEK SATIRLI VERi)");
        options1[2] = F("DENGELi OTOMATiK (TEK SATIRLI VERi)");
        options1[3] = F("TERAZiDEN TUŞ iLE (ÇOK SATIRLI VERi)");
        options1[4] = F("YAZICIDAN TUŞ iLE (ÇOK SATIRLI VERi)");
        options1[5] = F("KUMANDA");
#ifdef HAS_BLUETOOTH
        options1[6] = F("BLUETOOTH 2.1");
#endif
        options1[7] = F("WiFi ALICI");
        options1[8] = F("KOMUT");
        options1[9] = F("SANAL USB");
        int optionValues1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        addFormSelector(F("MOD"), F("plugin_112_mod"), 10, options1, optionValues1, choice1);

		addFormNumericBox(F("PARLAKLIK"), F("plugin_112_parlaklik"), HRCWiFi_Parlaklik, 0, 15);
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        HRCWiFi_Model = getFormItemInt(F("plugin_112_model"));
        HRCWiFi_Indikator = getFormItemInt(F("plugin_112_indikator"));
        HRCWiFi_Mod = getFormItemInt(F("plugin_112_mod"));
		HRCWiFi_Parlaklik = getFormItemInt(F("plugin_112_parlaklik"));
		sayac1_pasif = getFormItemInt(F("plugin_112_sayac1_pasif"));
        sayac2_pasif = getFormItemInt(F("plugin_112_sayac2_pasif"));
        durus_gecikmesi = getFormItemInt(F("plugin_112_durus_gecikmesi"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));
        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, HRCWiFi_Indikator, PCONFIG(4));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
		HRC_button1_112.attachClick(hrcwifi_click1);
        //HRC_button1_112.attachDoubleClick(hrc_click1);
        HRC_button1_112.attachLongPressStart(hrcwifi_longPressStart1);
        //HRC_button1_112.attachLongPressStop(hrcwifi_longPressStop1);
        HRC_button2_112.attachClick(hrcwifi_click2);
        //HRC_button2_112.attachDoubleClick(hrcwifi_click2);
        HRC_button2_112.attachLongPressStart(hrcwifi_longPressStart2);
        //HRC_button2_112.attachLongPressStop(hrcwifi_longPressStop2);
        HRC_button1_112.tick();
        HRC_button2_112.tick();
        P.begin();
        P.setIntensity(HRCWiFi_Parlaklik); // Parlaklık ayarı (0 - 15 arası)
        //P.displayClear();  // Ekranı temizleme
        P.setFont(dotmatrix_5x8);
		if (P.displayAnimate())
		  P.displayText(Settings.Name,PA_RIGHT,50,200,PA_PRINT,PA_NO_EFFECT);
        //P.displayClear();
		if (P.displayAnimate())
		  P.displayText(XML_SAYAC_1_C,PA_LEFT,0,200,PA_PRINT,PA_NO_EFFECT);
		hrcmesaj_degisken = "";
		Settings.WebAPP = 112;
        success = true;
        break;
      }

	  case PLUGIN_FIFTY_PER_SECOND:
      {
        HRC_button1_112.tick();
        HRC_button2_112.tick();
        long durus_zamani = millis() - sayac1_aktif_112;
        if ((durus_zamani > durus_gecikmesi) && (durus_aktif_112)) {
          XML_DURUS_ZAMANI_S = node_time.getDateTimeString('-',':',' ');
          durus_aktif_112 = false;
        }
        else if ((durus_zamani < durus_gecikmesi) && (!durus_aktif_112))
          durus_aktif_112 = true;
        success = true;
        break;
      }

	case PLUGIN_TEN_PER_SECOND:
	  {
		/*if (HRCWiFi_Mod == 7) {
		  if ((WiFi.status() == 6) || (WiFi.status() == 5) || (WiFi.status() == 4) || (WiFi.status() == 1) || (WiFi.status() == 0)) {
	  		if (P.displayAnimate())
		      P.displayText("BAGLANTI YOK",PA_LEFT,50,200,PA_SCROLL_LEFT,PA_SCROLL_LEFT);
 	      }
		  else if (WiFi.status() == 2) {
		    if (P.displayAnimate())
              P.displayText("BAGLANTI TARANDI",PA_LEFT,50,200,PA_SCROLL_LEFT,PA_SCROLL_LEFT);
	      }
		  else if (WiFi.status() == 3) {
			if (P.displayAnimate()) {
	  		  if (XML_NET_S != hrcmesaj_degisken) {
                hrcmesaj_degisken = XML_NET_S;
	  	        P.displayText(XML_NET_C,PA_LEFT,0,0,PA_PRINT,PA_NO_EFFECT);
			  }
			}
		  }
		  else {
		    if (P.displayAnimate())
              P.displayText("BAGLANTI KURULDU",PA_LEFT,50,200,PA_SCROLL_LEFT,PA_SCROLL_LEFT);		
		  }
		}*/
		//else {
      	  /*if (P.displayAnimate()) {
    	    if (XML_SAYAC_1_S != hrcmesaj_degisken) {
              hrcmesaj_degisken = XML_SAYAC_1_S;
	  	      P.displayText(XML_SAYAC_1_C,PA_RIGHT,0,0,PA_PRINT,PA_NO_EFFECT);
		    }
		  }*/
		 if (P.displayAnimate()) {
    	    if (XML_NET_S != hrcmesaj_degisken) {
              hrcmesaj_degisken = XML_NET_S;
	  	      P.displayText(XML_NET_C,PA_RIGHT,0,0,PA_PRINT,PA_NO_EFFECT);
		    }
		  }
		//} 
		success = true;
		break;
	  }
	
    case PLUGIN_ONCE_A_SECOND:
      {
        serial_error(event, HRCWiFi_Mod, "");
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if ((Settings.UDPPort > 0) && (string.length() > 0 )) {
          string.replace("\n", "");
          string.replace("\r", "");
          string.replace(String(char(ExtraTaskSettings.TaskDeviceSonByte)), "");
          hataTimer_l = millis();
          udp_client(event, HRCWiFi_Indikator, string, HRCWiFi_Mod);
          //Serial.println(string);
          string = "";          
        }
        success = true;
        break;
      }

	  /*case PLUGIN_SERIAL_IN:
      {
        while (Serial1.available()) {
          char inChar = Serial1.read();
          if (inChar == 255)  // binary data...
          {
            Serial1.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          if ((inChar == ExtraTaskSettings.TaskDeviceSonByte) && (tartimString_s.length() > 1)) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, HRCWiFi_Indikator, tartimString_s);
            if ((HRCWiFi_Mod == 3) || (HRCWiFi_Mod == 4))
              formul_kontrol(event, tartimString_s, HRCWiFi_Mod, true);
            else if (HRCWiFi_Mod == 6) {
              paketVeri_s += tartimString_s;
              oto_yazdir = true;
            } else
              formul_seri(event, tartimString_s, HRCWiFi_Indikator);
            tartimString_s = "";
            Serial1.flush();
          }
        }
        success = true;
        break;
      }*/

      /*case PLUGIN_TEN_PER_SECOND:
      {
        char XML_NET_C[9];
        char XML_DARA_C[9];
        char XML_BRUT_C[9];
        XML_TARIH_S = node_time.getDateString('-');
        XML_SAAT_S  = node_time.getTimeString(':');
        String s = tartimdatahrcwifi;
        if (XML_NET_S == "ERROR")
          s = "mesWF HATA ";
        else {
          dtostrf(XML_NET_S.toFloat(),  (Settings.net_bitis_byte - Settings.net_bas_byte),   Settings.nokta_byte, XML_NET_C);
          dtostrf(XML_DARA_S.toFloat(), (Settings.dara_bitis_byte - Settings.dara_bas_byte), Settings.nokta_byte, XML_DARA_C);
          dtostrf(XML_BRUT_S.toFloat(), (Settings.net_bitis_byte - Settings.net_bas_byte),   Settings.nokta_byte, XML_BRUT_C);
          //switch (HRC_Mod) {
            //case 0:
          if (millis() > stabilTimer_l) {
            stabilTimer_l = millis() + 200;
            stabilTartim_s = XML_NET_S;
            if (hrcwifi_stabil_sayisi >= 2) {
              hrcwifi_stabil_sayisi = 0;
              XML_STABIL_S = "()";
            } else {
              hrcwifi_stabil_sayisi = 0;
              XML_STABIL_S = "(>";
            }
          } else {
            if (stabilTartim_s == XML_NET_S)
              hrcwifi_stabil_sayisi++;
          }
          s.replace("%tarih%", XML_TARIH_S);
          s.replace("%saat%", XML_SAAT_S);
          s.replace("%adet%", XML_ADET_S);
          s.replace("%stabil%", XML_STABIL_S);
          s.replace("%net%", XML_NET_C);
          s.replace("%dara%", XML_DARA_C);
          s.replace("%brut%", XML_BRUT_C);
        }
        dmd.drawString(1,0,(XML_NET_S));
        //break;
        //}
        success = true;
        break;
      }*/

      /*case PLUGIN_ONCE_A_SECOND:
      {
        serial_error(event, HRCWiFi_Mod, "hata");
        dmd.drawString(1,0,F(" HATA   "));
        success = true;
        break;
      }*/

      /*case PLUGIN_WRITE:
      {
        */
      /*if (HRC_Mod == 5) {
          String strings[3];
          LoadCustomTaskSettings(event->TaskIndex, strings, 3, CUSTOMTASK_STR_SIZE_P120);
          safe_strncpy(fyzartKomut, strings[0], ART_BUFF_SIZE_P120);
          safe_strncpy(fyztopKomut, strings[1], TOP_BUFF_SIZE_P120);
          safe_strncpy(fyztekKomut, strings[2], TEK_BUFF_SIZE_P120);
        }
        string.replace("\n", "");
        string.replace("\r", "");
        string.replace(String(char(Settings.son_byte)), "");
        if ((String(fyzartKomut).length() > 0) && (string.substring(Settings.net_bas_byte, (String(fyzartKomut).length() + Settings.net_bas_byte)) == String(fyzartKomut)))
         ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
        if ((String(fyztopKomut).length() > 0) && (string.substring(Settings.net_bas_byte, (String(fyztopKomut).length() + Settings.net_bas_byte)) == String(fyztopKomut)))
         ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
        if ((String(fyztekKomut).length() > 0) && (string.substring(Settings.net_bas_byte, (String(fyztekKomut).length() + Settings.net_bas_byte)) == String(fyztekKomut)))
         ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
        else*/
      /*udp_client(event, HRCWiFi_Indikator, string, HRCWiFi_Mod);
        string = "";
        success = true;
        break;
      }*/

    /*case PLUGIN_SERIAL_IN:
      {
        float bol = 1;
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255)  // binary data...
          {
            Serial.flush();
            break;
          }
          tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            isaret(event, HRCWiFi_Indikator, tartimString_s);
            if ((HRCWiFi_Mod == 3) || (HRCWiFi_Mod == 4))
              formul_kontrol(event, tartimString_s, HRCWiFi_Mod, true);
            else
              formul_seri(event, tartimString_s, HRCWiFi_Indikator);
            tartimString_s = "";
            Serial.flush();
          }
        }
        //dmd.drawString(1, 0, XML_NET_C, 7, GRAPHICS_NORMAL);
        dmd.drawString(1, 0, XML_NET_C);
        success = true;
        break;
      }*/
  }
  return success;
}
#endif