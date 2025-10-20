#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P122

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
//##################################### Plugin 122: FYZ  ################################################
//#######################################################################################################
#define PLUGIN_122
#define PLUGIN_ID_122 122
#define PLUGIN_NAME_122 "Printer - FYZ KEPCE"
#define PLUGIN_VALUENAME1_122 "NET"
#define PLUGIN_VALUENAME2_122 "DARA"
#define PLUGIN_VALUENAME3_122 "BRUT"
#define PLUGIN_VALUENAME4_122 "ADET"
#define PLUGIN_VALUENAME5_122 "ADETGR"
#define PLUGIN_VALUENAME6_122 "PLUNO"
#define PLUGIN_VALUENAME7_122 "B.FIYAT"
#define PLUGIN_VALUENAME8_122 "TUTAR"
#define PLUGIN_VALUENAME9_122 "NET_2"
#define PLUGIN_VALUENAME10_122 "DARA_2"

#define FYZ_Model ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define FYZ_Indikator ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define FYZ_Mod ExtraTaskSettings.TaskDevicePluginConfigLong[2]
#define FYZ_Buton1 ExtraTaskSettings.TaskDevicePluginConfigLong[3]
#define FYZ_Buton2 ExtraTaskSettings.TaskDevicePluginConfigLong[4]
#define FYZ_Gecikme ExtraTaskSettings.TaskDevicePluginConfigLong[5]
#define FYZ_Sayac ExtraTaskSettings.TaskDevicePluginConfigLong[6]

#define FYZ_Kopya ExtraTaskSettings.TaskDevicePluginConfig[0]
#define FYZ_Logo ExtraTaskSettings.TaskDevicePluginConfig[1]

#define FYZ_art_komut ExtraTaskSettings.TaskDeviceMesage[0]
#define FYZ_tek_komut ExtraTaskSettings.TaskDeviceMesage[1]
#define FYZ_top_komut ExtraTaskSettings.TaskDeviceMesage[2]

#define FYZ_Hedef PCONFIG_FLOAT(0)

int veri_sayac_p122 = 0;
bool yazdir_aktif_p122 = false;

boolean Plugin_122(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  switch (function) {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_122;
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
        string = F(PLUGIN_NAME_122);
        break;
      }
    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_122));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_122));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_122));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_122));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_122));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_122));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_122));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_122));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_122));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_122));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        addFormSubHeader(F("FYZ Ayarları"));
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
        byte choice2 = ExtraTaskSettings.TaskDevicePrint[0];
        int optionValues2[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        addFormSelector(F("Tek Fiş"), F("plugin_122_tek_prn"), 10, options2, optionValues2, choice2);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Fiş Dizayn Menüsüne Git"));
        byte choice3 = ExtraTaskSettings.TaskDevicePrint[1];
        addFormSelector(F("Artı Fiş"), F("plugin_122_art_prn"), 10, options2, optionValues2, choice3);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Fiş Dizayn Menüsüne Git"));
        byte choice5 = ExtraTaskSettings.TaskDevicePrint[3];
        addFormSelector(F("Ek Fiş"), F("plugin_122_ek_prn"), 10, options2, optionValues2, choice5);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[3]], F("Fiş Dizayn Menüsüne Git"));
        byte choice4 = ExtraTaskSettings.TaskDevicePrint[2];
        addFormSelector(F("Toplam Fiş"), F("plugin_122_top_prn"), 10, options2, optionValues2, choice4);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[2]], F("Fiş Dizayn Menüsüne Git"));
        addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));
        success = true;
        break;
      }
    case PLUGIN_WEBFORM_SAVE:
      {
        ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_122_tek_prn"));
        ExtraTaskSettings.TaskDevicePrint[1] = getFormItemInt(F("plugin_122_art_prn"));
        ExtraTaskSettings.TaskDevicePrint[3] = getFormItemInt(F("plugin_122_ek_prn"));
        ExtraTaskSettings.TaskDevicePrint[2] = getFormItemInt(F("plugin_122_top_prn"));
        options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.tek_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[1]].toCharArray(Settings.art_prn, options2[ExtraTaskSettings.TaskDevicePrint[1]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[3]].toCharArray(Settings.ek_prn,  options2[ExtraTaskSettings.TaskDevicePrint[3]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[2]].toCharArray(Settings.top_prn, options2[ExtraTaskSettings.TaskDevicePrint[2]].length() + 1);
        success = true;
        break;
      }
    case PLUGIN_INIT:
      {
        Settings.WebAPP = 122;
        success = true;
        break;
      }
      /*case PLUGIN_TEN_PER_SECOND:
      {
        if (irrecv.decode(&results)) {
          //serialPrintUint64(results.value, HEX);
          IRDA_DATA_S = String(int(results.value));
          //Serial.println(IRDA_DATA_S);
          irrecv.resume();  // Receive the next value
        }
        delay(100);
        success = true;
        break;
      }*/
      /*case PLUGIN_TEN_PER_SECOND:
      {
        uint8_t i;
        if (WiFiConnected()) {
          if (FyzServer->hasClient())
          {
            for (i = 0; i < MAX_SRV_CLIENTS; i++) {
              if (!FyzClients[i]) {
                FyzClients[i] = FyzServer->available();
                continue;
              }
            }
          }
          for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
            while (FyzClients[i].available() && Serial.availableForWrite() > 0) {
              Serial.write(FyzClients[i].read());
            }
          }
        }
        success = true;
        break;
      }*/
/*#ifdef CAS_VERSION
    case PLUGIN_ONE_HUNDRED_PER_SECOND:
      {
        if (FYZ_Mod == 2) {
          if (webapinettartim_son == webapinettartim) {
            fyz_stabil_sayisi++;
          } else
            fyz_stabil_sayisi = 0;
          if (fyz_stabil_sayisi >= (FYZ_Gecikme * 10)) {
            XML_STABIL_S = "ST";
            if ((webapinettartim > FYZ_Hedef) && (hayvan_modu == 0)) {
              ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
              hayvan_modu = 2;
            }
          } else {
            XML_STABIL_S = "US";
            if ((webapinettartim < FYZ_Hedef) && (hayvan_modu == 2))
              hayvan_modu = 0;
          }
        }
        success = true;
        break;
      }
#endif*/
    case PLUGIN_FIFTY_PER_SECOND:
      {
        if ((((millis() - hataTimer_l) > 1000) && (yazdir_aktif_p122)) || (veri_sayac_p122 > 16)) {
          ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
          veri_sayac_p122 = 0;
          yazdir_aktif_p122 = false;
          //Serial.println("yazdir!");
        }
        else if (((millis() - hataTimer_l) > 1000) && (!yazdir_aktif_p122)) {
          veri_sayac_p122 = 0;
          yazdir_aktif_p122 = false;
          //Serial.println("yazdirma!");
        }
        success = true;
        break;
      }
    case PLUGIN_SERIAL_IN:
      {
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255) {
            Serial.flush();
            break;
          }
          if ((((int)inChar >= 32) && ((int)inChar < 127)) || (((int)inChar == 199) || ((int)inChar == 231) || ((int)inChar == 214) || ((int)inChar == 246) || ((int)inChar == 220) || ((int)inChar == 252) || ((int)inChar == 208) || ((int)inChar == 240) || ((int)inChar == 221) || ((int)inChar == 253) || ((int)inChar == 222) || ((int)inChar == 254) || ((int)inChar == 195)))
            tartimString_s += (String)inChar;
          if (inChar == 13) {//ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();              
            /*if (tartimString_s.length() >= 1) 
              XML_DATA[veri_sayac_p122] = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[veri_sayac_p122], (tartimString_s.length() - 1));
            else
              XML_DATA[veri_sayac_p122] = "";
            yazdir_aktif_p122 = true;*/
            if (tartimString_s.length() >= 1) {
              if (veri_sayac_p122 == 0) {
                if (tartimString_s.length() >= 1)  
                  XML_V0 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (tartimString_s.length()));
                else
                  XML_V0 = "";
              }
              else if (veri_sayac_p122 == 1) {
                if (tartimString_s.length() >= 1)  
                  XML_V1 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[1], (tartimString_s.length()));
                else
                  XML_V1 = "";
              }
              else if (veri_sayac_p122 == 2) {
                if (tartimString_s.length() >= 1)
                  XML_V2 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[2], (tartimString_s.length()));
                else
                  XML_V2 = "";
              }
              else if (veri_sayac_p122 == 3) {
                if (tartimString_s.length() >= 1)
                  XML_V3 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[3], (tartimString_s.length()));
                else 
                  XML_V3 = "";
              }
              else if (veri_sayac_p122 == 4) {
                if (tartimString_s.length() >= 1)
                  XML_V4 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[4], (tartimString_s.length()));
                else 
                  XML_V4 = "";
              }
              else if (veri_sayac_p122 == 5) {
                if (tartimString_s.length() >= 1)
                  XML_V5 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[5], (tartimString_s.length()));
                else 
                  XML_V5 = "";
              }
              else if (veri_sayac_p122 == 6) {
                if (tartimString_s.length() >= 1)
                  XML_V6 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[6], (tartimString_s.length()));
                else 
                  XML_V6 = "";
              }
              else if (veri_sayac_p122 == 7) {
                if (tartimString_s.length() >= 1)
                  XML_V7 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[7], (tartimString_s.length()));
                else 
                  XML_V7 = "";
              }
              else if (veri_sayac_p122 == 8) {
                if (tartimString_s.length() >= 1)
                  XML_V8 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[8], (tartimString_s.length()));
                else 
                  XML_V8 = "";
              }
              else if (veri_sayac_p122 == 9) {
                if (tartimString_s.length() >= 1)
                  XML_V9 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[9], (tartimString_s.length()));
                else 
                  XML_V9 = "";
              }
              else if (veri_sayac_p122 == 10) {
                if (tartimString_s.length() >= 1)
                  XML_V10 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[10], (tartimString_s.length()));
                else 
                  XML_V10 = "";
              }
              else if (veri_sayac_p122 == 11) {
                if (tartimString_s.length() >= 1)
                  XML_V11 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[11], (tartimString_s.length()));
                else 
                  XML_V11 = "";
              }
              else if (veri_sayac_p122 == 12) {
                 if (tartimString_s.length() >= 1)
                  XML_V12 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[12], (tartimString_s.length()));
                else 
                  XML_V12 = "";
              }
              else if (veri_sayac_p122 == 13) {
                if (tartimString_s.length() >= 1)
                  XML_V13 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[13], (tartimString_s.length()));
                else 
                  XML_V13 = "";
              }
              else if (veri_sayac_p122 == 14) {
                if (tartimString_s.length() >= 1)
                  XML_V14 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[14], (tartimString_s.length()));
                else 
                  XML_V14 = "";
              }
              else if (veri_sayac_p122 == 15) {
                if (tartimString_s.length() >= 1) 
                  XML_V15 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[15], (tartimString_s.length()));
                else 
                  XML_V15 = "";
              }
              else if (veri_sayac_p122 == 16) {
                if (tartimString_s.length() >= 1)
                  XML_V16 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[16], (tartimString_s.length()));
                else 
                  XML_V16 = "";
                yazdir_aktif_p122 = true;
              }
            }
            tartimString_s = "";
            Serial.flush();
            veri_sayac_p122++;
          }
        }
        success = true;
        break;
      }
  }
  return success;
}
#endif