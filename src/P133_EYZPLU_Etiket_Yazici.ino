#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
//#include "ESP32Ping.h"

#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P133

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
//##################################### Plugin 133: EYZPLU ##############################################
//#######################################################################################################

#define PLUGIN_133
#define PLUGIN_ID_133 133
#define PLUGIN_NAME_133 "Printer - EYZPLU"
#define PLUGIN_VALUENAME1_133 "NET"
#define PLUGIN_VALUENAME2_133 "DARA"
#define PLUGIN_VALUENAME3_133 "BRUT"
#define PLUGIN_VALUENAME4_133 "ADET"
#define PLUGIN_VALUENAME5_133 "ADETGR"
#define PLUGIN_VALUENAME6_133 "PLUNO"
#define PLUGIN_VALUENAME7_133 "B.FIYAT"
#define PLUGIN_VALUENAME8_133 "TUTAR"
#define PLUGIN_VALUENAME9_133 "NET_2"
#define PLUGIN_VALUENAME10_133 "DARA_2"
#define MAX_SRV_CLIENTS 5

#define CUSTOMTASK_STR_SIZE_P133 20

#define HEDEF_ADDR_SIZE_P133 8

#define MES_BUFF_SIZE_P133 19
#define HEDEF_BUFF_SIZE_P133 9

#define EYZPLU_Model ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define EYZPLU_Indikator ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define EYZPLU_Mod ExtraTaskSettings.TaskDevicePluginConfigLong[2]
#define EYZPLU_Gecikme ExtraTaskSettings.TaskDevicePluginConfigLong[3]

#define EYZPLU_Bartender ExtraTaskSettings.TaskPrintBartender

#define EYZPLU_art_komut ExtraTaskSettings.TaskDeviceMesage[0]
#define EYZPLU_tek_komut ExtraTaskSettings.TaskDeviceMesage[1]
#define EYZPLU_top_komut ExtraTaskSettings.TaskDeviceMesage[2]

#define EYZPLU_Hedef PCONFIG_FLOAT(0)

bool internet = false;

boolean Plugin_133(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_133;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 10;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_133);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_133));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_133));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_133));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_133));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_133));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_133));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_133));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_133));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_133));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_133));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
#ifdef CAS_VERSION
        addFormSubHeader(F("Yazıcı Ayarları"));
#else
        addFormSubHeader(F("EYZPLU Ayarları"));
#endif
        byte choice0 = EYZPLU_Model;
        String options0[5];
        options0[0] = F("EYZ72R");
        options0[1] = F("EYZ100");
        options0[2] = F("EYZ100R");
        options0[3] = F("EYZ72Mobil");
        options0[4] = F("EYZ100Mobil");
        int optionValues0[5];
        optionValues0[0] = 0;
        optionValues0[1] = 1;
        optionValues0[2] = 2;
        optionValues0[3] = 3;
        optionValues0[4] = 4;
        addFormSelector(F("Yazıcı Model"), F("plugin_133_model"), 5, options0, optionValues0, choice0);

        byte choice1 = EYZPLU_Mod;
        String options1[7];
        options1[0] = F("SÜREKLi VERi (TEK SATIRLI VERi)");
        options1[1] = F("TERAZiDEN OTOMATiK (TEK SATIRLI VERi)");
        options1[2] = F("DENGELi OTOMATiK (TEK SATIRLI VERi)");
        options1[3] = F("TERAZiDEN TUŞ iLE (ÇOK SATIRLI VERi)");
        options1[4] = F("YAZICIDAN TUŞ iLE KONTROL(ÇOK SATIRLI VERi)");
        options1[5] = F("KUMANDA");
        options1[6] = F("VERi PAKETi");
        int optionValues1[7];
        optionValues1[0] = 0;
        optionValues1[1] = 1;
        optionValues1[2] = 2;
        optionValues1[3] = 3;
        optionValues1[4] = 4;
        optionValues1[5] = 5;
        optionValues1[6] = 6;
        addFormSelector(F("Yazdırma Modu"), F("plugin_133_mod"), 7, options1, optionValues1, choice1);
        
        addFormCheckBox(F("Bartender prn"), F("plugin_133_bartender"), EYZPLU_Bartender);
#ifdef ESP32
        fs::File root = ESPEASY_FS.open(F("/rules"));
        fs::File file = root.openNextFile();
        int fileno = 0;
        while (file) {
          if (!file.isDirectory()) {
            const String fname(file.name());
            if (fname.startsWith(F("/eyz")) || fname.startsWith(F("eyz"))) {
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
        int optionValues2[10];
        optionValues2[0] = 0;
        optionValues2[1] = 1;
        optionValues2[2] = 2;
        optionValues2[3] = 3;
        optionValues2[4] = 4;
        optionValues2[5] = 5;
        optionValues2[6] = 6;
        optionValues2[7] = 7;
        optionValues2[8] = 8;
        optionValues2[9] = 9;
        addFormSelector(F("Tek Etiket"), F("plugin_133_tek_prn"), 10, options2, optionValues2, choice2);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Etiket Dizayn Menüsüne Git"));

        byte choice3 = ExtraTaskSettings.TaskDevicePrint[1];

        addFormSelector(F("Artı Etiket"), F("plugin_133_art_prn"), 10, options2, optionValues2, choice3);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Etiket Dizayn Menüsüne Git"));

        byte choice4 = ExtraTaskSettings.TaskDevicePrint[2];

        addFormSelector(F("Toplam Etiket"), F("plugin_133_top_prn"), 10, options2, optionValues2, choice4);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Etiket Dizayn Menüsüne Git"));

#if FEATURE_SD
        byte choice5 = ExtraTaskSettings.TaskDevicePrint[3];
        addFormSelector(F("SD data"), F("plugin_133_sd_prn"), 10, options2, optionValues2, choice5);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[3]], F("SD Data Dizayn Menüsüne Git"));
#endif    

        if (EYZPLU_Mod == 2) {
          addFormTextBox(F("Hedef Kilogram"), F("plugin_133_hedef"), String(EYZPLU_Hedef, 3), HEDEF_BUFF_SIZE_P133);
          addFormNumericBox(F("Gecikme Saniye"), F("plugin_133_gecikme"), EYZPLU_Gecikme, 0, 999999);
        } else if (EYZPLU_Mod == 5) {
          addFormTextBox(F("Artı Komutu"), getPluginCustomArgName(0), EYZPLU_art_komut, MES_BUFF_SIZE_P133);
          addFormTextBox(F("Toplam Komutu"), getPluginCustomArgName(1), EYZPLU_top_komut, MES_BUFF_SIZE_P133);
          addFormTextBox(F("Tek Komutu"), getPluginCustomArgName(2), EYZPLU_tek_komut, MES_BUFF_SIZE_P133);
        }
        addFormSubHeader(F("İndikatör Ayarları"));
        indikator_secimi(event, EYZPLU_Indikator, F("plugin_133_indikator"));
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        EYZPLU_Model = getFormItemInt(F("plugin_133_model"));
        EYZPLU_Indikator = getFormItemInt(F("plugin_133_indikator"));
        EYZPLU_Mod = getFormItemInt(F("plugin_133_mod"));
        EYZPLU_Gecikme = getFormItemInt(F("plugin_133_gecikme"));
        EYZPLU_Bartender = isFormItemChecked(F("plugin_133_bartender"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));

        PCONFIG_FLOAT(0) = getFormItemFloat(F("plugin_133_hedef"));
        if (EYZPLU_Indikator == 5) {
          strncpy_webserver_arg(EYZPLU_art_komut, getPluginCustomArgName(0));
          strncpy_webserver_arg(EYZPLU_top_komut, getPluginCustomArgName(1));
          strncpy_webserver_arg(EYZPLU_tek_komut, getPluginCustomArgName(2));
        }

        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, EYZPLU_Indikator, PCONFIG(4));

        ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_133_tek_prn"));
        ExtraTaskSettings.TaskDevicePrint[1] = getFormItemInt(F("plugin_133_art_prn"));
        ExtraTaskSettings.TaskDevicePrint[2] = getFormItemInt(F("plugin_133_top_prn"));
        ExtraTaskSettings.TaskDevicePrint[3] = getFormItemInt(F("plugin_133_sd_prn"));
        options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.tek_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[1]].toCharArray(Settings.art_prn, options2[ExtraTaskSettings.TaskDevicePrint[1]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[2]].toCharArray(Settings.top_prn, options2[ExtraTaskSettings.TaskDevicePrint[2]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[3]].toCharArray(Settings.sd_prn,  options2[ExtraTaskSettings.TaskDevicePrint[3]].length() + 1);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!PCONFIG(4)) {
          switch (EYZPLU_Indikator) {
            case 26:
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[0], PSTR("NW"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[1], PSTR("TW"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[2], PSTR("GW"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[3], PSTR("QTY"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[4], PSTR("APW"));
              break;
            case 27:
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[0], PSTR("NET"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[1], PSTR("Tare"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[2], PSTR("Gross"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[3], PSTR("PCS"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[4], PSTR("U/W"));
              break;
          }
        }
        Settings.WebAPP = 133;
        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        success = true;
        break;
      }

      case PLUGIN_TEN_PER_SECOND:
      {
        switch (EYZPLU_Mod) {
          case 0:
            break;
          case 2:
            if ((webapinettartim > EYZPLU_Hedef) && (hayvan_modu == 0)) {
              stabilTimer_l = millis() + (EYZPLU_Gecikme * 1000);
              StabilTartim_f = webapinettartim;
              hayvan_modu = 1;
            }
            if ((millis() > stabilTimer_l) && (hayvan_modu == 1)) {
              if (StabilTartim_f == webapinettartim) {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
                hayvan_modu = 2;
              } else {
                hayvan_modu = 0;
              }
            }
            if (((webapinettartim < EYZPLU_Hedef) && (webapinettartim > 0.001)) && (hayvan_modu == 2)) {
              hayvan_modu = 0;
            }
            break;
        }
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (Settings.UDPPort > 0) {
          string.replace("\n", "");
          string.replace("\r", "");
          string.replace(String(char(ExtraTaskSettings.TaskDeviceSonByte)), "");
          if ((String(EYZPLU_art_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_art_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart");
          else if ((String(EYZPLU_art_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_top_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
          else if ((String(EYZPLU_tek_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_tek_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
          else
            udp_client(event, EYZPLU_Indikator, string, EYZPLU_Mod);
          string = "";
        }
        success = true;
        break;
      }

#ifdef ESP32
    case PLUGIN_SERIAL_IN:
      {
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255)  // binary data...
          {
            Serial.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, EYZPLU_Indikator, tartimString_s);
            if ((EYZPLU_Mod == 3) || (EYZPLU_Mod == 4))
              formul_kontrol(event, tartimString_s, EYZPLU_Mod, true);
            else {
              formul_seri(event, tartimString_s, EYZPLU_Indikator);
              Serial.flush();
            }
            tartimString_s = "";
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