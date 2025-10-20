#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"
#ifdef USES_P101

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

//#######################################################################################################
//#################################### Plugin 101: SERiAL CLIENT  #######################################
//#######################################################################################################

#define PLUGIN_101
#define PLUGIN_ID_101 101
#define PLUGIN_NAME_101 "Communication - SERiAL CLiENT"
#define PLUGIN_VALUENAME1_101 "NET"
#define PLUGIN_VALUENAME2_101 "DARA"
#define PLUGIN_VALUENAME3_101 "BRUT"
#define MAX_SRV_CLIENTS 5

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#include <ETH.h>
#endif

#define SRV_IP_BUFF_SIZE_P101 19

#define CLi_Model ExtraTaskSettings.TaskDevicePluginConfig[0]
#define CLi_ASCII ExtraTaskSettings.TaskDevicePluginConfig[1]
#define CLi_Data ExtraTaskSettings.TaskDevicePluginConfig[2]
#define CLi_CRLF ExtraTaskSettings.TaskDevicePluginConfig[3]
#define CLi_Mod ExtraTaskSettings.TaskDevicePluginConfig[4]
#define CLi_TCPIP_Port ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define CLi_Ind ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define CLi_IP ExtraTaskSettings.TaskDeviceMesage[0]

WiFiClient client;
boolean Plugin_101_init = false;
boolean gonder = false;

boolean Plugin_101(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  switch (function) {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_101;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
        //Device[deviceCount].Type = DEVICE_TYPE_SERIAL;
        //Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_STRING;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }
    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_101);
        break;
      }
    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_101));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_101));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_101));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        byte choice0 = CLi_Model;
        String options0[2];
        options0[0] = F("Tartım Sistemi");
        options0[1] = F("Barkod Okuyucu");
        int optionValues0[2] = {0, 1};
        addFormSelector(F("Aygıt Tipi"), F("plugin_101_model"), 2, options0, optionValues0, choice0);
        if (CLi_Model == 0) {
          /*byte choice1 = CLi_Mod;
          String options1[7];
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
          int optionValues1[7] = {0, 1, 2, 3, 4, 5, 6};
          addFormSelector(F("Mod"), F("plugin_101_mod"), 7, options1, optionValues1, choice1);*/          
          indikator_secimi(event, CLi_Ind, F("plugin_101_indikator"));
          addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
          addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));
          addFormCheckBox(F("Data Format Aktif"), F("plugin_101_ozel"), CLi_Data);
          addFormNote(F("<font color='red'>Terazi Verisinin Önüne APP İsmini Ekler.</font>"));
        } else {
          addFormNumericBox(F("Son Byte"), F("son_byte_0"), ExtraTaskSettings.TaskDeviceSonByte, 0, 255);
          addFormNote(F("<font color='red'>Barkod Okuyucu Aktif.</font>"));
        }
        addFormTextBox(F("SERVER IP "), getPluginCustomArgName(0), CLi_IP, SRV_IP_BUFF_SIZE_P101);
        addFormNumericBox(F("TCP PORT"), F("plugin_101_port"), CLi_TCPIP_Port, 1, 65535);
        addFormSubHeader(F("Data Ayarları"));
        addFormCheckBox(F("ASCII"), F("plugin_101_ascii"), CLi_ASCII);
        addFormNote(F("<font color='red'>Terazi Verisindeki Anlamsız Karakterleri Siler.</font>"));
        addFormCheckBox(F("CR-LF"), F("plugin_101_crlf"), CLi_CRLF);
        addFormNote(F("<font color='red'>Terazi Verisinin Sonuna r n Karakterlerini Ekler.</font>"));
        success = true;
        break;
      }
    case PLUGIN_WEBFORM_SAVE:
      {
        CLi_Model = getFormItemInt(F("plugin_101_model"));
        CLi_TCPIP_Port = getFormItemInt(F("plugin_101_port"));
        CLi_ASCII = isFormItemChecked(F("plugin_101_ascii"));
        CLi_CRLF = isFormItemChecked(F("plugin_101_crlf"));
        if (CLi_Model == 0) {
          CLi_Ind = getFormItemInt(F("plugin_101_indikator"));
          PCONFIG(4) = isFormItemChecked(F("duzenle"));
          CLi_Data = isFormItemChecked(F("plugin_101_ozel"));
          indikator_secimi_kaydet(event, CLi_Ind, PCONFIG(4));
        } else
            ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte_0"));
        strncpy_webserver_arg(CLi_IP, getPluginCustomArgName(0));
        success = true;
        break;
      }
    case PLUGIN_INIT:
    { 
      Settings.WebAPP = 101;
      Plugin_101_init = true;
      success = true;
      break;
    }
    case PLUGIN_FIFTY_PER_SECOND:
    {
      if (CLi_Model == 1) {
        if ((millis() > hataTimer_l + 200 ) && (gonder)) {
          if (CLi_CRLF)
            tartimString_s += "\r\n";
          WiFiClient client;
          client.setTimeout(15);
          client.connect(CLi_IP, CLi_TCPIP_Port);
          client.print(tartimString_s);
          XML_BARKOD_S = tartimString_s;
          SendUDPCommand(0, (const char *)tartimString_s.c_str(), tartimString_s.length());
          tartimString_s = "";
          gonder = false;
        }
      }
      success = true;
      break;
    }
    case PLUGIN_ONCE_A_SECOND:
    {
      serial_error(event, 0, "");
      success = true;
      break;
    }
#ifdef ESP32
#if FEATURE_ETHERNET || defined(HAS_WIFI)
    case PLUGIN_SERIAL_IN:
    {
        while (Serial1.available()) {
          gonder = true;
          hataTimer_l = millis();
          char inChar = Serial1.read();
          if (inChar == 255)  // binary data...
          {
            Serial1.flush();
            break;
          }
          if (CLi_ASCII) {
            if (isprint(inChar))
              tartimString_s += (String)inChar;
          } else
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            WiFiClient client;
            client.setTimeout(15);
            client.connect(CLi_IP, CLi_TCPIP_Port);
            if (CLi_Model == 0) {
              if (Settings.Tersle)
                tersle(event, tartimString_s);
              isaret(event, CLi_Ind, tartimString_s);
              formul_seri(event, tartimString_s, CLi_Ind);
              String mesaj_s;
              if (CLi_Data)
                mesaj_s = Settings.Name;
              mesaj_s += XML_NET_S;
              if (CLi_CRLF)
                mesaj_s += "\r\n";
              client.print(mesaj_s);
            } else {
              if (CLi_CRLF)
                tartimString_s += "\r\n";
              client.print(tartimString_s);
              XML_BARKOD_S = tartimString_s;
            }
            tartimString_s = "";
            Serial1.flush();
          }
        }
        success = true;
        break;
      }
#endif
/*#ifdef HAS_WIFI
    case PLUGIN_SERIAL_IN:
      {
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255)  // binary data...
          {
            Serial.flush();
            break;
          }
          if (CLi_ASCII) {
            if (isprint(inChar))
              tartimString_s += (String)inChar;
          } else
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (WiFi.status() ==  WL_CONNECTED) {
              client.connect(CLi_IP, CLi_TCPIP_Port);
            }
            if (CLi_Model == 0) {
              if (Settings.Tersle)
                tersle(event, tartimString_s);
              isaret(event, CLi_Ind, tartimString_s);
              formul_seri(event, tartimString_s, CLi_Ind);
              String mesaj_s;
              if (CLi_Data)
                mesaj_s = ExtraTaskSettings.TaskDeviceName; //Settings.Name;
              mesaj_s += XML_NET_S;
              if (WiFi.status() ==  WL_CONNECTED) {
                client.print(mesaj_s);
              }
            } else {
              if (CLi_CRLF)
                tartimString_s += "\r\n";
                if (WiFi.status() ==  WL_CONNECTED) {
                  client.print(tartimString_s);
                }
              XML_BARKOD_S = tartimString_s;
            }
            unsigned long timeout = millis();
            if (WiFi.status() ==  WL_CONNECTED) {
              if (client.available() == 0) {
                if (millis() - timeout > 5000) {
                  client.stop();
                  delay(10);
                  break;
                }
              }
            }
            tartimString_s = "";
            Serial.flush();
          }
        }
        success = true;
        break;
      }
#endif*/
#endif
#ifdef ESP8266
case PLUGIN_SERIAL_IN:
      {
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255)  // binary data...
          {
            Serial.flush();
            break;
          }
          if (CLi_ASCII) {
            if (isprint(inChar))
              tartimString_s += (String)inChar;
          } else
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            WiFiClient client;
            client.connect(CLi_IP, CLi_TCPIP_Port);
            if (CLi_Model == 0) {
              if (Settings.Tersle)
                tersle(event, tartimString_s);
              isaret(event, CLi_Ind, tartimString_s);
              formul_seri(event, tartimString_s, CLi_Ind);
              String mesaj_s;
              if (CLi_Data)
                mesaj_s = Settings.Name;
              mesaj_s += XML_NET_S;
              if (CLi_CRLF)
                mesaj_s += "\r\n";
              client.print(mesaj_s);

            } else {
              if (CLi_CRLF)
                tartimString_s += "\r\n";
              client.print(tartimString_s);
              XML_BARKOD_S = tartimString_s;
            }
            unsigned long timeout = millis();
            if (client.available() == 0) {
              if (millis() - timeout > 5000) {
                client.stop();
                delay(10);
                break;
              }
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
#endif  // USES_P101