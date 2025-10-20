#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"
#include "src/Globals/ESPEasy_Console.h"

#ifdef USES_P100

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

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#include <ETH.h>
#endif
//#######################################################################################################
//#################################### Plugin 100: SERiAL SERVER  #######################################
//#######################################################################################################

#define PLUGIN_100
#define PLUGIN_ID_100 100
#define PLUGIN_NAME_100 "Communication - SERiAL SERVER"
#define PLUGIN_VALUENAME1_100  "NET"
#define PLUGIN_VALUENAME2_100  "DARA"
#define PLUGIN_VALUENAME3_100  "BRUT"
#define PLUGIN_VALUENAME4_100  "ADET"
#ifdef ESP32
#define PLUGIN_VALUENAME5_100  "ADETGR"
#define PLUGIN_VALUENAME6_100  "PLUNO"
#define PLUGIN_VALUENAME7_100  "B.FIYAT"
#define PLUGIN_VALUENAME8_100  "TUTAR"
#define PLUGIN_VALUENAME9_100  "NET_2"
#define PLUGIN_VALUENAME10_100 "DARA_2"
#endif

#define SRV_ASCII      ExtraTaskSettings.TaskDevicePluginConfig[0]
#define SRV_DATA_AKTIF ExtraTaskSettings.TaskDevicePluginConfig[1]
#define SRV_VINC       ExtraTaskSettings.TaskDevicePluginConfig[2]

#define SRV_INDIKATOR  ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define SRV_PORT       ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define SRV_MOD        ExtraTaskSettings.TaskDevicePluginConfigLong[2]

#include "OneButton.h"

#ifdef ESP32
#if FEATURE_ETHERNET
OneButton SRV_button1(14, true, true);
#endif
#ifdef HAS_WIFI
OneButton SRV_button1(21, true, true);
#endif
#endif

String file_data_srv;
boolean Plugin_100_init = false;
WiFiServer *sernetServer;
WiFiClient sernetClients;

String Serial_data;
int data_sayac_i = 0;

String Srv_data;

int int_value;
float float_value;
bool bool_value = true;

/*
uint8_t broadcastAddress_send[6];
esp_now_peer_info_t peerInfo_send;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  pinMode(espnow_led, OUTPUT);
  status == ESP_NOW_SEND_SUCCESS ? digitalWrite(espnow_led, HIGH) : digitalWrite(espnow_led, LOW);
}

void initESP_NOW_CLi() {
  int data[6];
  sscanf(String(Settings.espnow_mac_address).c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &data[0], &data[1], &data[2], &data[3], &data[4], &data[5]);
  //broadcastAddress_send[6] = ((uint8_t)data[0], (uint8_t)data[1], (uint8_t)data[2], (uint8_t)data[3], (uint8_t)data[4], (uint8_t)data[5]);
  broadcastAddress_send[0] = (uint8_t)data[0];
  broadcastAddress_send[1] = (uint8_t)data[1];
  broadcastAddress_send[2] = (uint8_t)data[2];
  broadcastAddress_send[3] = (uint8_t)data[3];
  broadcastAddress_send[4] = (uint8_t)data[4];
  broadcastAddress_send[5] = (uint8_t)data[5];
  if (esp_now_init() != ESP_OK) {
    Serial.printf("Error initializing ESP-NOW\r\n");
    return;
  }
  //Serial.write(broadcastAddress_send,6);
  memcpy(peerInfo_send.peer_addr, broadcastAddress_send, sizeof(broadcastAddress_send));
  peerInfo_send.channel = 1;  
  peerInfo_send.encrypt = false;
  if (esp_now_add_peer(&peerInfo_send) != ESP_OK){
    Serial.printf("Failed to add peer\r\n");
    return;
  }
}*/

void SRV_longPressStart1() {
  WifiAPMode = true;
}

boolean Plugin_100(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  //static uint8_t connectionState = 0;
  switch (function) {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_100;
        //Device[deviceCount].Type = DEVICE_TYPE_SERIAL;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        //Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_STRING;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
        Device[deviceCount].FormulaOption = true;
#ifdef ESP8266
        Device[deviceCount].ValueCount = 4;
#endif
#ifdef ESP32
        Device[deviceCount].ValueCount = 10;
#endif
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }
    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_100);
        break;
      }
    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_100));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_100));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_100));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_100));
#ifdef ESP32
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_100));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_100));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_100));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_100));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_100));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_100));
#endif
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        addFormNumericBox(F("TCP PORT"), F("plugin_100_port"), SRV_PORT, 1, 65535);
        addFormCheckBox(F("ASCII"), F("plugin_100_ascii"), SRV_ASCII);
        //addFormCheckBox(F("CR-LF"), F("plugin_100_crlf"), PCONFIG(1));
        addFormCheckBox(F("Data Format Aktif"), F("plugin_100_ozel"), SRV_DATA_AKTIF);
        addFormCheckBox(F("Vinç Modu"), F("plugin_100_vinc"), SRV_VINC);
        byte choice1 = SRV_MOD;
        String options1[10];
        options1[0] = F("SÜREKLi VERi (TEK SATIRLI VERi)");
        options1[1] = F("TERAZiDEN OTOMATiK (TEK SATIRLI VERi)");
#ifdef CAS_VERSION
        options1[2] = F("DENGELi OTOMATiK (TEK SATIRLI VERi)");
#endif
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
        addFormSelector(F("Mod"), F("plugin_100_mod"), 10, options1, optionValues1, choice1);
        if (SRV_DATA_AKTIF) {
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
              if (fname.startsWith(F("/srv")) || fname.startsWith(F("srv"))) {
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
          addFormSelector(F("Data Format"), F("plugin_100_seridata_prn"), 10, options2, optionValues2, choice2);
          addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Data Dizayn Menüsüne Git"));
        }
        addFormSubHeader(F("İndikatör Ayarları"));
        indikator_secimi(event, SRV_INDIKATOR, F("plugin_100_indikator"));
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));
        success = true;
        break;
      }
    case PLUGIN_WEBFORM_SAVE:
      {
        SRV_INDIKATOR  = getFormItemInt(F("plugin_100_indikator"));
        SRV_PORT       = getFormItemInt(F("plugin_100_port"));
        SRV_MOD        = getFormItemInt(F("plugin_100_mod"));
        SRV_ASCII      = isFormItemChecked(F("plugin_100_ascii"));
        SRV_DATA_AKTIF = isFormItemChecked(F("plugin_100_ozel"));
        SRV_VINC       = isFormItemChecked(F("plugin_100_vinc"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));
        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, SRV_INDIKATOR, PCONFIG(4));
        if (SRV_DATA_AKTIF) {
          ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_100_seridata_prn"));
          options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.seridata_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        }
        success = true;
        break;
      }
    case PLUGIN_INIT:
      {
       #if defined(HAS_WIFI)
        pinMode(14, OUTPUT);
        pinMode(27, OUTPUT);
        digitalWrite(14, HIGH);
        digitalWrite(27, HIGH);
      #endif
        /*if (Settings.espnow_mod == 1)
          initESP_NOW_CLi();*/
        /*switch (SRV_INDIKATOR) {
          case 26:
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[0], PSTR("NW"));
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[1], PSTR("TW"));
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[2], PSTR("GW"));
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[3], PSTR("QTY"));
#ifdef ESP32
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[4], PSTR("APW"));
#endif
            break;
          case 33:
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[0], PSTR("NET"));
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[1], PSTR("Tare"));
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[2], PSTR("Gross"));
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[3], PSTR("PCS"));
#ifdef ESP32
            strcpy_P(ExtraTaskSettings.TaskDeviceFormula[4], PSTR("U/W"));
#endif
            break;
        }*/
        //else if (Settings.espnow_mod == 0) {
          if ((SRV_MOD != 6) && (SRV_MOD != 9)) {
            if (SRV_PORT != 0) {
              sernetServer = new WiFiServer(SRV_PORT);
              sernetServer->begin();
              sernetServer->setNoDelay(true);
              Plugin_100_init = true;
            }
          }
        //}
        //SRV_button1.attachLongPressStart(SRV_longPressStart1);
        //SRV_button1.tick();
        if (SRV_DATA_AKTIF) {
          if (fileExists(Settings.seridata_prn)) {
            fs::File form = tryOpenFile(Settings.seridata_prn, "r");
            while (form.position() < form.size()) {
              file_data_srv = form.readStringUntil('\r');
              file_data_srv.trim();
            }
            form.close();
          }
        }
        Settings.WebAPP = 100;
        success = true;
        break;
      }
    case PLUGIN_TEN_PER_SECOND:
      {
        /*if (Settings.espnow_mod == 1)
          esp_now_register_send_cb(OnDataSent);*/
        //SRV_button1.tick();
        //else if (Settings.espnow_mod == 0) {
        if (SRV_MOD != 9) {
          if (Plugin_100_init) {
            //if (!SRV_VINC) {
              if (sernetServer->hasClient()) {
                if (sernetClients) { sernetClients.stop(); }
                sernetClients = sernetServer->available();
              }
              if (sernetClients.connected()) {
                //connectionState = 1;
                //int count = sernetClients.available();
                while (sernetClients.available()) {
                  char inChar = sernetClients.read();
                  Srv_data += (String)inChar;
                  if (inChar == '\r') {
                    //if (Srv_data == "srvnet?")
                      sernetClients.write((byte *)XML_NET_S.c_str(), XML_NET_S.length());
                    Srv_data = "";
                  }
                  /*if (count > 0) {
                    //sernetClients.write((byte *)XML_NET_S.c_str(), XML_NET_S.length());
                    sernetClients.flush();
                  }
                  else {
                    if (connectionState == 1) {
                      connectionState = 0;
                      sernetClients.setTimeout(10);
                    }
                  }*/
                }
              }
            }
          //}
          if (!Plugin_100_init) {
            if (SRV_MOD != 6) {
              if (SRV_PORT != 0) {
                sernetServer = new WiFiServer(SRV_PORT);
                sernetServer->begin();
                sernetServer->setNoDelay(true);
                Plugin_100_init = true;
              }
            }
            Plugin_100_init = true;
          }
        }
        success = true;
        break;
      }
    case PLUGIN_ONCE_A_SECOND:
      { 
        if ((SRV_MOD == 0) || (SRV_MOD == 2) || (SRV_MOD == 4))
          serial_error(event, PCONFIG_LONG(2), "");
        success = true;
        break;
      }
    case PLUGIN_WRITE:
      {
        if (SRV_MOD == 7) {
          if (Settings.UDPPort > 0) {
            hataTimer_l = millis();
            tartimString_s = string;
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            else {
              isaret(event, SRV_INDIKATOR, tartimString_s);
              formul_seri(event, tartimString_s, SRV_INDIKATOR);
            }
            String file_data = file_data_srv;
            //file_data = string_convert(file_data);
            parseSystemVariables(file_data, false);
            //Serial_data = file_data;
            //Serial.print(file_data);
            //serialPrint(file_data);
            string = "";
          }
        }
        success = true;
        break;
      }
#if FEATURE_ETHERNET || defined(HAS_WIFI) || defined(HAS_BLUETOOTH)
    case PLUGIN_SERIAL_IN:
      {
        //if (Plugin_100_init) {
          while (Serial1.available()) {
            char inChar = Serial1.read();
            if (inChar == 255) {
              Serial1.flush();
              break;
            }
            if (SRV_ASCII) {
              if (isprint(inChar))
                tartimString_s += (String)inChar;
            } else
              tartimString_s += (String)inChar;
            if ((inChar == ExtraTaskSettings.TaskDeviceSonByte) && (tartimString_s.length() > 1)) {
              /*if (Settings.espnow_mod == 1) {
                esp_err_t result = esp_now_send(broadcastAddress_send, (uint8_t*) tartimString_s.c_str(), tartimString_s.length());
                if (result == ESP_OK) {
                //Serial.println("Sending confirmed");
                pinMode(espnow_led, OUTPUT);
                digitalWrite(espnow_led, HIGH);
                } else {
                //Serial.println("Sending error");
                pinMode(espnow_led, OUTPUT);
                digitalWrite(espnow_led, LOW);
                }
              }*/
              hataTimer_l = millis();
              if (Settings.Tersle)
                tersle(event, tartimString_s);
              else {
                isaret(event, SRV_INDIKATOR, tartimString_s);
                if ((SRV_MOD == 3) || (SRV_MOD == 4))
                  formul_kontrol(event, tartimString_s, SRV_INDIKATOR, false);
                else {
                  formul_seri(event, tartimString_s, SRV_INDIKATOR);
                  if (SRV_INDIKATOR == 10) {
                    if (tartimString_s.substring(0, 1) == "0")
                      XML_STABIL_S = "ST";
                    else
                      XML_STABIL_S = "US";
                  }
                  if ((SRV_INDIKATOR == 12) || (SRV_INDIKATOR == 16)) 
                    XML_STABIL_S = tartimString_s.substring(0, 2);
                  //Serial1.flush();
                }
              }
#ifdef HAS_BLUETOOTH
              //if (PCONFIG(2))
              //  SerialBT.print(String(XML_NET_C));
              //else
              //SerialBT.print(tartimString_s);
              //if (PCONFIG(1))
              //  SerialBT.println();
#endif
              if (!SRV_VINC) {
                String file_data = file_data_srv;
                //file_data = string_convert(file_data);
                parseSystemVariables(file_data, false);
                if ((SRV_MOD != 9) && (Plugin_100_init)) {
                  if (sernetClients) {
                    if (SRV_MOD != 8) {
                      if (SRV_DATA_AKTIF)
                        sernetClients.print(file_data);
                      else
                        sernetClients.print(tartimString_s);
                      //if (PCONFIG(1))
                        //sernetClients.println();
                      sernetClients.flush();
                      delay(1);
                    }
                  }
                  sendSysInfoUDP(1);
                }
                else {                
                  if (SRV_DATA_AKTIF)
                    Serial.print(file_data);
                  else
                    Serial.println(tartimdata_s);
                  Serial.flush();
                }
              }
              else {
                String UDP_data = XML_STABIL_S;
                UDP_data += tartimString_s;
                if (SRV_INDIKATOR == 9) {
                  UDP_data.replace(")", "0.");
                  UDP_data.replace("!", "1.");
                  UDP_data.replace("@", "2.");
                  UDP_data.replace("#", "3.");
                  UDP_data.replace("$", "4.");
                  UDP_data.replace("%", "5.");
                  UDP_data.replace("^", "6.");
                  UDP_data.replace("&", "7.");
                  UDP_data.replace("*", "8.");
                  UDP_data.replace("(", "9.");
                }
                SendUDPCommand(0, (const char *)UDP_data.c_str(), UDP_data.length());
                //SendUDPCommand(0, (const char *)XML_NET_C, 6);
              }           
              json_net = tartimString_s.substring(0, tartimString_s.length() - 1);
              #ifdef ESP_NOW_ACTIVE
                EspnowSendData(String(XML_NET_C));
              #endif
              tartimString_s = "";
              Serial1.flush();
            }
          }
        //}
        success = true;
        break;
      }
#endif
/*#ifdef HAS_WIFI
    case PLUGIN_SERIAL_IN:
      {
        if (Plugin_100_init) {
          while (Serial.available()) {
            char inChar = Serial.read();
            if (inChar == 255) {
              Serial.flush();
              break;
            }
            if (SRV_ASCII) {
              if (isprint(inChar))
                tartimString_s += (String)inChar;
            } else
              tartimString_s += (String)inChar;
            if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
              hataTimer_l = millis();
              if (Settings.Tersle)
                tersle(event, tartimString_s);
              isaret(event, SRV_INDIKATOR, tartimString_s);
              if ((SRV_MOD == 3) || (SRV_MOD == 4))
                formul_kontrol(event, tartimString_s, SRV_MOD, false);
              else {
                formul_seri(event, tartimString_s, SRV_MOD);
                if (SRV_INDIKATOR == 10) {
                  if (tartimString_s.substring(0, 1) == "0")
                    XML_STABIL_S = "ST";
                  else
                    XML_STABIL_S = "US";
                }
                if (SRV_INDIKATOR == 16)
                  XML_STABIL_S = tartimString_s.substring(0, 2);
                Serial.flush();
              }
              if (!SRV_VINC) {
                String file_data = file_data_srv;
                //file_data = string_convert(file_data);
                parseSystemVariables(file_data, false);
                if (SRV_DATA_AKTIF)
                  Serial.print(file_data);
                if (sernetClients) {
                  if (SRV_DATA_AKTIF)
                    sernetClients.print(file_data);
                  else
                    sernetClients.print(tartimString_s);
                  //if (PCONFIG(1))
                    //sernetClients.println();
                  sernetClients.flush();
                  delay(1);
                }
                sendSysInfoUDP(1);
              }
              if (SRV_VINC)
                SendUDPCommand(0, (const char *)tartimString_s.c_str(), tartimString_s.length());
              //sendData(event);
              tartimString_s = "";
              Serial.flush();
            }
          }
        }
        success = true;
        break;
      }
#endif*/
#ifdef ESP8266
    case PLUGIN_SERIAL_IN:
      {
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255) {
            Serial.flush();
            break;
          }
          if (PCONFIG(0)) {
            if (isprint(inChar))
              tartimString_s += (String)inChar;
          } else
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, PCONFIG_LONG(0), tartimString_s);
            if ((PCONFIG_LONG(2) == 3) || (PCONFIG_LONG(2) == 4))
              formul_kontrol(event, tartimString_s, PCONFIG_LONG(2), false);
            else {
              formul_seri(event, tartimString_s, PCONFIG_LONG(0));
              if (PCONFIG_LONG(0) == 10) {
                if (tartimString_s.substring(0, 1) == "0")
                  XML_STABIL_S = "ST";
                else
                  XML_STABIL_S = "US";
              }
              if (PCONFIG_LONG(0) == 16)
                XML_STABIL_S = tartimString_s.substring(0, 2);
              Serial.flush();
            }
            /*#ifdef ESP32
#ifdef HAS_BLUETOOTH
              if (PCONFIG(2))
                SerialBT.print(String(XML_NET_C));
              else
                SerialBT.print(tartimString_s);
              if (PCONFIG(1))
                SerialBT.println();
#endif
#endif*/

            if (PCONFIG_LONG(2) != 6) {
              if (!PCONFIG(3)) {
                String file_data = file_data_srv;
                //file_data = string_convert(file_data);
                parseSystemVariables(file_data, false);
                if (PCONFIG(2))
                  Serial.print(file_data);
                if ((sernetClients) && (Plugin_100_init)) {
                  if (PCONFIG(2))
                    sernetClients.print(file_data);
                  else
                    sernetClients.print(tartimString_s);
                  if (PCONFIG(1))
                    sernetClients.println();
                  sernetClients.flush();
                  delay(1);
                }
                sendSysInfoUDP(1);
              }
              if (PCONFIG(3))
                SendUDPCommand(0, (const char *)tartimString_s.c_str(), tartimString_s.length());
            }
            tartimString_s = "";
            Serial.flush();
          }
        }
        success = true;
        break;
      }
#endif
  }
  return success;
} 
#endif  // USES_P100

//esit PWI 1200 baud "B   100CR"
