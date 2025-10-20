#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P121

#include "src/Commands/InternalCommands.h"
#include "src/ESPEasyCore/ESPEasyNetwork.h"
#include "src/ESPEasyCore/Controller.h"
#include "src/CustomBuild/CompiletimeDefines.h"

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
#include "src/Globals/SecuritySettings.h"

#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/Memory.h"
#include "src/Helpers/StringConverter.h"
#include "src/Helpers/StringParser.h"
#include "src/Helpers/Networking.h"
#include "src/Helpers/StringGenerator_System.h"

#include "ESPEasy_common.h"
#include "ESPEasy-Globals.h"
//#######################################################################################################
//##################################### Plugin 121: FYZPLU ##############################################
//#######################################################################################################

#define PLUGIN_121
#define PLUGIN_ID_121 121
#define PLUGIN_NAME_121 "Printer - FYZPLU"
#define PLUGIN_VALUENAME1_121 "NET"
#define PLUGIN_VALUENAME2_121 "DARA"
#define PLUGIN_VALUENAME3_121 "BRUT"
#define PLUGIN_VALUENAME4_121 "ADET"
#define PLUGIN_VALUENAME5_121 "ADETGR"
#define PLUGIN_VALUENAME6_121 "PLUNO"
#define PLUGIN_VALUENAME7_121 "B.FIYAT"
#define PLUGIN_VALUENAME8_121 "TUTAR"
#define PLUGIN_VALUENAME9_121 "NET_2"
#define PLUGIN_VALUENAME10_121 "DARA_2"
#define MAX_SRV_CLIENTS 5

#define HEDEF_ADDR_SIZE_P121 8


#define MES_BUFF_SIZE_P121 19
#define HEDEF_BUFF_SIZE_P121 9

#define FYZPLU_Model ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define FYZPLU_Indikator ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define FYZPLU_Mod ExtraTaskSettings.TaskDevicePluginConfigLong[2]
#define FYZPLU_Buton1 ExtraTaskSettings.TaskDevicePluginConfigLong[3]
#define FYZPLU_Buton2 ExtraTaskSettings.TaskDevicePluginConfigLong[4]
#define FYZPLU_Kopya ExtraTaskSettings.TaskDevicePluginConfigLong[5]
#define FYZPLU_Logo ExtraTaskSettings.TaskDevicePluginConfigLong[6]
#define FYZPLU_Gecikme ExtraTaskSettings.TaskDevicePluginConfigLong[7]

#define FYZPLU_art_komut ExtraTaskSettings.TaskDeviceMesage[0]
#define FYZPLU_tek_komut ExtraTaskSettings.TaskDeviceMesage[1]
#define FYZPLU_top_komut ExtraTaskSettings.TaskDeviceMesage[2]

#define FYZPLU_Hedef PCONFIG_FLOAT(0)

//WiFiServer *FyzServer;
//WiFiClient FyzClients[MAX_SRV_CLIENTS];
/*#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

//const uint16_t kRecvPin = 13;
const uint16_t kRecvPin = 14;

IRrecv irrecv(kRecvPin);

decode_results results;
*/

#include "OneButton.h"

#ifdef ESP8266
OneButton FYZPLU_button1(12, true);
OneButton FYZPLU_button2(14, true);
OneButton FYZPLU_button3(13, true);
#endif
#ifdef ESP32
#if FEATURE_ETHERNET
OneButton FYZPLU_button1(14, true);//OneButton FYZPLU_button1(12, true);
OneButton FYZPLU_button2(15, true);//OneButton FYZPLU_button2(14, true);
#endif
#ifdef HAS_WIFI
OneButton FYZPLU_button1(21, true);//OneButton FYZPLU_button1(12, true);
OneButton FYZPLU_button2(22, true);//OneButton FYZPLU_button2(14, true);
//OneButton FYZPLU_button3(13, true);
//OneButton FYZPLU_button4(26, true);
//OneButton FYZPLU_button5(27, true);
#endif
/*#ifdef HAS_BLE
OneButton FYZPLU_button1(21, true, true);
OneButton FYZPLU_button2(22, true, true);
OneButton FYZPLU_button3(13, true, true);
OneButton FYZPLU_button4(26, true, true);
OneButton FYZPLU_button5(27, true, true);
#endif*/
#endif

void FYZPLU_click1() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzpluart#1");
}

void FYZPLU_longPressStart1() {
  Serial1.println(F("System Info"));
  Serial1.println(F("      VERSIYON 1.2"));
	Serial1.print(F("  IP Address    : ")); Serial1.println(formatIP(NetworkLocalIP()));
	Serial1.print(F("  Build         : ")); Serial1.println(String(get_build_nr()) + '/' + getSystemBuildString());
	Serial1.print(F("  Name          : ")); Serial1.println(Settings.getName());
	Serial1.print(F("  Unit          : ")); Serial1.println(String(static_cast<int>(Settings.Unit)));
	Serial1.print(F("  WifiSSID      : ")); Serial1.println(SecuritySettings.WifiSSID);
	Serial1.print(F("  WifiKey       : ")); Serial1.println(SecuritySettings.WifiKey);
  Serial1.print("\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
}

void FYZPLU_longPressStop1() {
}

void FYZPLU_click2() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzpluart#2");
}

/*void FYZPLU_longPressStart2() {
  Settings.UseSerial = true;
}

void FYZPLU_longPressStop2() {
  Settings.UseSerial = false;
}*/

void FYZPLU_click3() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzpluart#3");
}

/*void FYZPLU_longPressStart3() {
  Settings.UseSerial = true;
}

void FYZPLU_longPressStop3() {
  Settings.UseSerial = false;
}*/
#if defined(ESP32)
void FYZPLU_click4() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzpluart#4");
}

/*void FYZPLU_longPressStart3() {
  Settings.UseSerial = true;
}

void FYZPLU_longPressStop3() {
  Settings.UseSerial = false;
}*/

void FYZPLU_click5() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzpluart#0");
}

/*void FYZPLU_longPressStart3() {
  Settings.UseSerial = true;
}

void FYZPLU_longPressStop3() {
  Settings.UseSerial = false;
}*/
#endif

boolean Plugin_121(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_121;
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
        string = F(PLUGIN_NAME_121);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_121));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_121));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_121));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_121));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_121));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_121));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_121));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_121));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_121));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_121));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        addFormSubHeader(F("FYZPLU Ayarları"));
        byte choice0 = FYZPLU_Model;
        String options0[4];
        options0[0] = F("FYZ58H");
        options0[1] = F("FYZ80");
        options0[2] = F("FYZ58Mobil");
        options0[3] = F("FYZ80Mobil");
        int optionValues0[4] = {0, 1, 2, 3};
        addFormSelector(F("Model"), F("plugin_121_model"), 4, options0, optionValues0, choice0);
        addFormCheckBox(F("Kopya Aktif"), F("plugin_121_kopya"), FYZPLU_Kopya);
        byte choice1 = FYZPLU_Mod;
        String options1[7];
        options1[0] = F("SÜREKLi VERi (TEK SATIRLI VERi)");
        options1[1] = F("TERAZiDEN OTOMATiK (TEK SATIRLI VERi)");
        options1[2] = F("DENGELi OTOMATiK (TEK SATIRLI VERi)");
        options1[3] = F("TERAZiDEN TUŞ iLE (ÇOK SATIRLI VERi)");
        options1[4] = F("YAZICIDAN TUŞ iLE KONTROL(ÇOK SATIRLI VERi)");
        options1[5] = F("KUMANDA");
        options1[6] = F("VERi PAKETi");
        int optionValues1[7] = {0, 1, 2, 3, 4, 5, 6};
        addFormSelector(F("Mod"), F("plugin_121_mod"), 7, options1, optionValues1, choice1);
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
            if (fname.startsWith(F("/fyz")) || fname.startsWith(F("fyz"))) {
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
        addFormSelector(F("Tek Fiş"), F("plugin_121_plu_tek_prn"), 10, options2, optionValues2, choice2);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Fiş Dizayn Menüsüne Git"));
        byte choice3 = ExtraTaskSettings.TaskDevicePrint[1];
        addFormSelector(F("Artı Fiş"), F("plugin_121_plu_art_prn"), 10, options2, optionValues2, choice3);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Fiş Dizayn Menüsüne Git"));
        byte choice5 = ExtraTaskSettings.TaskDevicePrint[3];
        addFormSelector(F("Ek Fiş"), F("plugin_121_plu_ek_prn"), 10, options2, optionValues2, choice5);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[3]], F("Fiş Dizayn Menüsüne Git"));
        byte choice4 = ExtraTaskSettings.TaskDevicePrint[2];
        addFormSelector(F("Toplam Fiş"), F("plugin_121_plu_top_prn"), 10, options2, optionValues2, choice4);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[2]], F("Fiş Dizayn Menüsüne Git"));
        if (FYZPLU_Mod == 2) {
          addFormTextBox(F("Hedef Kilogram"), F("plugin_121_hedef"), String(FYZPLU_Hedef), HEDEF_BUFF_SIZE_P121);
          addFormNumericBox(F("Gecikme Saniye"), F("plugin_121_gecikme"), FYZPLU_Gecikme, 0, 999999);
        } else if (FYZPLU_Mod == 5) {
          addFormTextBox(F("Artı Komutu"), getPluginCustomArgName(0), FYZPLU_art_komut, MES_BUFF_SIZE_P121);
          addFormTextBox(F("Toplam Komutu"), getPluginCustomArgName(1), FYZPLU_tek_komut, MES_BUFF_SIZE_P121);
          addFormTextBox(F("Tek Komutu"), getPluginCustomArgName(2), FYZPLU_top_komut, MES_BUFF_SIZE_P121);
        }
        addFormSubHeader(F("İndikatör Ayarları"));
        indikator_secimi(event, FYZPLU_Indikator, F("plugin_121_indikator"));
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("Baslangıç-Bitiş Datasının Değişimine İzin Verir."));
        success = true;
        break;
      }
    case PLUGIN_WEBFORM_SAVE:
      {
        FYZPLU_Model = getFormItemInt(F("plugin_121_model"));
        FYZPLU_Indikator = getFormItemInt(F("plugin_121_indikator"));
        FYZPLU_Mod = getFormItemInt(F("plugin_121_mod"));
        FYZPLU_Gecikme = getFormItemInt(F("plugin_121_gecikme"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));
        PCONFIG_FLOAT(0) = getFormItemFloat(F("plugin_121_hedef"));
        if (FYZPLU_Mod == 5) {
          strncpy_webserver_arg(FYZPLU_art_komut, getPluginCustomArgName(0));
          strncpy_webserver_arg(FYZPLU_top_komut, getPluginCustomArgName(1));
          strncpy_webserver_arg(FYZPLU_tek_komut, getPluginCustomArgName(2));
        }
        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, FYZPLU_Indikator, PCONFIG(4));
        ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_121_plu_tek_prn"));
        ExtraTaskSettings.TaskDevicePrint[1] = getFormItemInt(F("plugin_121_plu_art_prn"));
        ExtraTaskSettings.TaskDevicePrint[3] = getFormItemInt(F("plugin_121_plu_ek_prn"));
        ExtraTaskSettings.TaskDevicePrint[2] = getFormItemInt(F("plugin_121_plu_top_prn"));
        options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.plu_tek_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[1]].toCharArray(Settings.plu_art_prn, options2[ExtraTaskSettings.TaskDevicePrint[1]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[3]].toCharArray(Settings.plu_ek_prn,  options2[ExtraTaskSettings.TaskDevicePrint[3]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[2]].toCharArray(Settings.plu_top_prn, options2[ExtraTaskSettings.TaskDevicePrint[2]].length() + 1);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        FYZPLU_button1.attachClick(FYZPLU_click1);
        FYZPLU_button1.attachLongPressStart(FYZPLU_longPressStart1);
        FYZPLU_button1.attachLongPressStop(FYZPLU_longPressStop1);
        FYZPLU_button2.attachClick(FYZPLU_click2);
        //FYZPLU_button2.attachLongPressStart(FYZPLU_longPressStart2);
        //FYZPLU_button2.attachLongPressStop(FYZPLU_longPressStop2);
        //FYZPLU_button3.attachLongPressStart(FYZPLU_longPressStart3);
        //FYZPLU_button3.attachLongPressStop(FYZPLU_longPressStop3);
        FYZPLU_button1.tick();
        FYZPLU_button2.tick();
#if defined(ESP32)
#ifdef HAS_WIFI
        //FYZPLU_button3.attachClick(FYZPLU_click3);
        //FYZPLU_button4.attachClick(FYZPLU_click4);
        //FYZPLU_button5.attachClick(FYZPLU_click5);
        //FYZPLU_button3.tick();
        //FYZPLU_button4.tick();
        //FYZPLU_button5.tick();
#endif
#endif
#if defined(ESP8266)
        pinMode(16, OUTPUT);
#endif
        Settings.WebAPP = 121;
        success = true;
        break;
      }
      /*case PLUGIN_TEN_PER_SECOND:
      {
        uint8_t i;
        if (WiFiConnected()) {
          if (FyzpluServer->hasClient())
          {
            for (i = 0; i < MAX_SRV_CLIENTS; i++) {
              if (!FyzpluClients[i]) {
                FyzpluClients[i] = FyzpluServer->available();
                continue;
              }
            }
          }
          for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
            while (FyzpluClients[i].available() && Serial.availableForWrite() > 0) {
              Serial.write(FyzpluClients[i].read());
            }
          }
        }
        success = true;
        break;
      }*/
    //case PLUGIN_FIFTY_PER_SECOND:
    case PLUGIN_TEN_PER_SECOND:
      {
        switch (FYZPLU_Mod) {
          case 0:
            FYZPLU_button1.tick();
            FYZPLU_button2.tick();
#if defined(ESP32)
#ifdef HAS_WIFI
            //FYZPLU_button3.tick();
            //FYZPLU_button4.tick();
            //FYZPLU_button5.tick();
#endif
#endif
            break;
          case 2:
            if ((webapinettartim > FYZPLU_Hedef) && (hayvan_modu == 0)) {
              stabilTimer_l = millis() + (FYZPLU_Gecikme * 1000);
              StabilTartim_f = webapinettartim;
              hayvan_modu = 1;
            }
            if ((millis() > stabilTimer_l) && (hayvan_modu == 1)) {
              if (StabilTartim_f == webapinettartim) {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                hayvan_modu = 2;
              } else
                hayvan_modu = 0;
            }
            if (((webapinettartim < FYZPLU_Hedef) && (webapinettartim > 0.001)) && (hayvan_modu == 2))
              hayvan_modu = 0;
            break;
        }
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        String komut = "fyzplu,";
        komut += XML_PLU_NO_S;
        serial_error(event, FYZPLU_Mod, komut);
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        udp_client(event, FYZPLU_Indikator, string, FYZPLU_Mod);
        success = true;
        break;
      }

#ifdef ESP32
#if FEATURE_ETHERNET
    case PLUGIN_SERIAL_IN:
      {
         while (Serial1.available()) {
          char inChar = Serial1.read();
          if (inChar == 255)  // binary data...
          {
            Serial1.flush();
            break;
          }
          tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, FYZPLU_Indikator, tartimString_s);
            if ((FYZPLU_Mod == 3) || (FYZPLU_Mod == 4))
              formul_kontrol(event, tartimString_s, FYZPLU_Mod, true);
            else {
              formul_seri(event, tartimString_s, FYZPLU_Indikator);
              if (FYZPLU_Mod == 5) {
                if ((String(FYZPLU_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZPLU_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZPLU_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                if ((String(FYZPLU_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZPLU_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZPLU_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                if ((String(FYZPLU_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZPLU_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZPLU_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                XML_NET_S = "";
              }
              if ((FYZPLU_Mod == 1) && (webapinettartim > 0.0001))
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
            }
            tartimString_s = "";
            Serial1.flush();
          }
        }
        success = true;
        break;
      }
#endif
#if defined(HAS_WIFI) || defined(HAS_BLE)
    case PLUGIN_SERIAL_IN:
      {
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
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, FYZPLU_Indikator, tartimString_s);
            if ((FYZPLU_Mod == 3) || (FYZPLU_Mod == 4))
              formul_kontrol(event, tartimString_s, FYZPLU_Mod, true);
            else {
              formul_seri(event, tartimString_s, FYZPLU_Indikator);
              if (FYZPLU_Mod == 5) {
                if ((String(FYZPLU_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZPLU_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZPLU_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                if ((String(FYZPLU_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZPLU_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZPLU_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                if ((String(FYZPLU_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZPLU_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZPLU_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                XML_NET_S = "";
              }
              if ((FYZPLU_Mod == 1) && (webapinettartim > 0.0001))
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
            }
            tartimString_s = "";
            Serial.flush();
          }
        }
        success = true;
        break;
      }
#endif
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
          tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, FYZPLU_Indikator, tartimString_s);
            if ((FYZPLU_Mod == 3) || (FYZPLU_Mod == 4))
              formul_kontrol(event, tartimString_s, FYZPLU_Mod, true);
            else {
              formul_seri(event, tartimString_s, FYZPLU_Indikator);
              if (FYZPLU_Mod == 5) {
                if ((String(FYZPLU_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZPLU_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZPLU_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                if ((String(FYZPLU_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZPLU_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZPLU_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                if ((String(FYZPLU_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZPLU_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZPLU_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                XML_NET_S = "";
              }
              if ((FYZPLU_Mod == 1) && (webapinettartim > 0.0001))
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
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
#endif

//ocs-sx 58880
//invld noacc nodel
//Dikomsan
//ETXSTXadd:       01   CRLF
//n/w:      0.09 gCRLF
//u/w:       0    CRLF
//pcs:       0   CRLF