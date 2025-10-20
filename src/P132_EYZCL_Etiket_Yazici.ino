#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P132

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
//##################################### Plugin 132: EYZ #################################################
//#######################################################################################################

#define PLUGIN_132
#define PLUGIN_ID_132 132
#define PLUGIN_NAME_132 "Printer - EYZCL"
#define PLUGIN_VALUENAME0_132 "V0_FORM"
#define PLUGIN_VALUENAME1_132 "V1_?"
#define PLUGIN_VALUENAME2_132 "V2_BARKOD"
#define PLUGIN_VALUENAME3_132 "V3_iSiM1"
#define PLUGIN_VALUENAME4_132 "V4_SNO"
#define PLUGIN_VALUENAME5_132 "V5_iSiM2"
#define PLUGIN_VALUENAME6_132 "V6_iSiM3"
#define PLUGIN_VALUENAME7_132 "V7_NET"
#define PLUGIN_VALUENAME8_132 "V8_B_FiYAT"
#define PLUGIN_VALUENAME9_132 "V9_TUTAR"
#define PLUGIN_VALUENAME10_132 "V10_P_TARiH"
#define PLUGIN_VALUENAME11_132 "V11_P_SAAT"
#define PLUGIN_VALUENAME12_132 "V12_S_TARiH"
#define PLUGIN_VALUENAME13_132 "V13_S_SAAT"
#define PLUGIN_VALUENAME14_132 "V14_MESAJ1"
#define PLUGIN_VALUENAME15_132 "V15_MESAJ2"
#define PLUGIN_VALUENAME16_132 "V16_MESAJ3"
#define PLUGIN_VALUENAME17_132 "V17_MESAJ4"
#define PLUGIN_VALUENAME18_132 "V18_MESAJ5"
#define PLUGIN_VALUENAME19_132 "V19_MESAJ6"
#define PLUGIN_VALUENAME20_132 "V20_MESAJ7"
#define PLUGIN_VALUENAME21_132 "V21_MESAJ8"
#define PLUGIN_VALUENAME22_132 "V22_MESAJ9"
#define PLUGIN_VALUENAME23_132 "V23_MESAJ10"
#define PLUGIN_VALUENAME24_132 "V24_iSiM4"
#define PLUGIN_VALUENAME25_132 "V25_iSiM5"
#define PLUGIN_VALUENAME26_132 "V26_BRUT"
#define PLUGIN_VALUENAME27_132 "V27_DARA"
#define PLUGIN_VALUENAME28_132 "V28_Z"
#define PLUGIN_VALUENAME29_132 "V29_BOS"

#define EYZCL_Model ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define EYZCL_Mod ExtraTaskSettings.TaskDevicePluginConfigLong[1]

#define EYZCL_Bartender ExtraTaskSettings.TaskPrintBartender

int veri_sayac = 0;
bool yazdir_aktif = false;

boolean Plugin_132(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_132;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 30;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_132);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME0_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME1_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME2_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME3_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME4_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME5_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME6_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME7_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME8_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME9_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[10], PSTR(PLUGIN_VALUENAME10_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[11], PSTR(PLUGIN_VALUENAME11_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[12], PSTR(PLUGIN_VALUENAME12_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[13], PSTR(PLUGIN_VALUENAME13_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[14], PSTR(PLUGIN_VALUENAME14_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[15], PSTR(PLUGIN_VALUENAME15_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[16], PSTR(PLUGIN_VALUENAME16_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[17], PSTR(PLUGIN_VALUENAME17_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[18], PSTR(PLUGIN_VALUENAME18_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[19], PSTR(PLUGIN_VALUENAME19_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[20], PSTR(PLUGIN_VALUENAME20_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[21], PSTR(PLUGIN_VALUENAME21_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[22], PSTR(PLUGIN_VALUENAME22_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[23], PSTR(PLUGIN_VALUENAME23_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[24], PSTR(PLUGIN_VALUENAME24_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[25], PSTR(PLUGIN_VALUENAME25_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[26], PSTR(PLUGIN_VALUENAME26_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[27], PSTR(PLUGIN_VALUENAME27_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[28], PSTR(PLUGIN_VALUENAME28_132));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[29], PSTR(PLUGIN_VALUENAME29_132));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
#ifdef CAS_VERSION
        addFormSubHeader(F("Yazıcı Ayarları"));
#else
        addFormSubHeader(F("EYZCL Ayarları"));
#endif
      addFormCheckBox(F("Bartender prn"), F("plugin_132_bartender"), EYZCL_Bartender);
/*
        byte choice0 = EYZCL_Model;
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
        addFormSelector(F("Yazıcı Model"), F("plugin_132_model"), 5, options0, optionValues0, choice0);

        byte choice1 = EYZCL_Mod;
        String options1[7];
        options1[0] = F("YAZICIDAN TUŞ iLE (TEK SATIRLI VERi)");
        options1[1] = F("TERAZiDEN OTOMATiK (TEK SATIRLI VERi)");
        options1[2] = F("DENGELi OTOMATiK (TEK SATIRLI VERi)");
        options1[3] = F("TERAZiDEN TUŞ iLE (ÇOK SATIRLI VERi)");
        options1[4] = F("YAZICIDAN TUŞ iLE KONTROL(ÇOK SATIRLI VERi)");
        options1[5] = F("KUMANDA VİNÇ");
        options1[6] = F("VERi PAKETi (CL5200)");
        int optionValues1[7];
        optionValues1[0] = 0;
        optionValues1[1] = 1;
        optionValues1[2] = 2;
        optionValues1[3] = 3;
        optionValues1[4] = 4;
        optionValues1[5] = 5;
        optionValues1[6] = 6;
        addFormSelector(F("Yazdırma Modu"), F("plugin_132_mod"), 7, options1, optionValues1, choice1);
#ifdef ESP8266
        fs::Dir filedata = ESPEASY_FS.openDir("rules");
        int fileno = 0;
        while (filedata.next()) {
          options2[fileno] = filedata.fileName();
          fileno++;
        }
#endif*/
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
        /*byte choice2 = ExtraTaskSettings.TaskDevicePrint[0];
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
        addFormSelector(F("Tek Etiket"), F("plugin_132_tek_prn"), 10, options2, optionValues2, choice2);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Etiket Dizayn Menüsüne Git"));

        byte choice3 = ExtraTaskSettings.TaskDevicePrint[1];
        addFormSelector(F("Artı Etiket"), F("plugin_132_art_prn"), 10, options2, optionValues2, choice3);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Etiket Dizayn Menüsüne Git"));

        byte choice4 = ExtraTaskSettings.TaskDevicePrint[2];
        addFormSelector(F("Toplam Etiket"), F("plugin_132_top_prn"), 10, options2, optionValues2, choice4);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[2]], F("Etiket Dizayn Menüsüne Git"));*/
/*#if FEATURE_SD
        byte choice5 = ExtraTaskSettings.TaskDevicePrint[3];
        addFormSelector(F("SD data"), F("plugin_132_sd_prn"), 10, options2, optionValues2, choice5);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[3]], F("SD Data Dizayn Menüsüne Git"));
#endif*/
        //addFormSubHeader(F("İndikatör Ayarları"));
        //addFormNumericBox(F("Son Byte"), F("son_byte"), ExtraTaskSettings.TaskDeviceSonByte, 0, 255);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        EYZCL_Model = getFormItemInt(F("plugin_132_model"));
        EYZCL_Mod = getFormItemInt(F("plugin_132_mod"));
        EYZCL_Bartender = isFormItemChecked(F("plugin_132_bartender"));
        for (int bit = 4; bit <= 6; bit++ ) {
          if (ExtraTaskSettings.TaskDeviceValueBit[bit] == 0)
            ExtraTaskSettings.TaskDeviceValueBit[bit] = 32;
        }
        //ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));
        //#ifdef ESP8266
        /*ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_132_tek_prn"));
        ExtraTaskSettings.TaskDevicePrint[1] = getFormItemInt(F("plugin_132_art_prn"));
        ExtraTaskSettings.TaskDevicePrint[2] = getFormItemInt(F("plugin_132_top_prn"));*/
        //ExtraTaskSettings.TaskDevicePrint[3] = getFormItemInt(F("plugin_132_sd_prn"));
        /*options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.tek_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[1]].toCharArray(Settings.art_prn, options2[ExtraTaskSettings.TaskDevicePrint[1]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[2]].toCharArray(Settings.top_prn, options2[ExtraTaskSettings.TaskDevicePrint[2]].length() + 1);*/
        //options2[ExtraTaskSettings.TaskDevicePrint[3]].toCharArray(Settings.sd_prn, options2[ExtraTaskSettings.TaskDevicePrint[3]].length() + 1);
        //#endif
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[0], PSTR("V0"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[1], PSTR("V1"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[2], PSTR("V2"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[3], PSTR("V3"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[4], PSTR("V4"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[5], PSTR("V5"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[6], PSTR("V6"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[7], PSTR("V7"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[8], PSTR("V8"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[9], PSTR("V9"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[10], PSTR("V10"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[11], PSTR("V11"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[12], PSTR("V12"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[13], PSTR("V13"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[14], PSTR("V14"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[15], PSTR("V15"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[16], PSTR("V16"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[17], PSTR("V17"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[18], PSTR("V18"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[19], PSTR("V19"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[20], PSTR("V20"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[21], PSTR("V21"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[22], PSTR("V22"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[23], PSTR("V23"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[24], PSTR("V24"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[25], PSTR("V25"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[26], PSTR("V26"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[27], PSTR("V27"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[28], PSTR("V28"));
        strcpy_P(ExtraTaskSettings.TaskDeviceFormula[29], PSTR("V29"));
        Settings.WebAPP = 132;
        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        if (((millis() - hataTimer_l) > 1000) && (yazdir_aktif)) {
          XML_V0.replace("FR", "");
          XML_V0.replace("\"", "");
          String eyzcl = "/rules/eyz";
          XML_V0.toLowerCase();
          eyzcl += XML_V0;
          eyzcl += ".prn";
          String komut = "eyztest#";
          komut += eyzcl;
          ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, komut.c_str());
          veri_sayac = 0;
          yazdir_aktif = false;
          Serial.flush();
        }
        else if ((millis() - hataTimer_l) > 1000)
          veri_sayac = 0;
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
          if (inChar == 10) {//ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (tartimString_s.length() >= 1) {
              
              /*if (tartimString_s.length() >= 1) 
                if () 
                  XML_DATA[veri_sayac] = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[veri_sayac], (tartimString_s.length() - 1));
              else
                XML_DATA[veri_sayac] = "";*/

              if (veri_sayac == 0) {
                if (tartimString_s.length() >= 1)  
                  XML_V0 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (tartimString_s.length()));
                else
                  XML_V0 = "";
              }
              else if (veri_sayac == 1) {
                if (tartimString_s.length() >= 1)  
                  XML_V1 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[1], (tartimString_s.length()));
                else
                  XML_V1 = "";
              }
              else if (veri_sayac == 2) {
                if (tartimString_s.length() >= 1)
                  XML_V2 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[2], (tartimString_s.length() - 1));
                else
                  XML_V2 = "";
              }
              else if (veri_sayac == 3) {
                if (tartimString_s.length() >= 1)
                  XML_V3 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[3], (tartimString_s.length() - 1));
                else 
                  XML_V3 = "";
              }
              else if (veri_sayac == 4) {
                if (tartimString_s.length() >= 1)
                  XML_V4 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[4], ExtraTaskSettings.TaskDeviceValueBit[4]);//(tartimString_s.length() - 1));
                else 
                  XML_V4 = "";
              }
              else if (veri_sayac == 5) {
                if (tartimString_s.length() >= 1)
                  XML_V5 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[5], ExtraTaskSettings.TaskDeviceValueBit[5]);//(tartimString_s.length() - 1));
                else 
                  XML_V5 = "";
              }
              else if (veri_sayac == 6) {
                if (tartimString_s.length() >= 1)
                  XML_V6 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[6], ExtraTaskSettings.TaskDeviceValueBit[6]);//(tartimString_s.length() - 1));
                else 
                  XML_V6 = "";
              }
              else if (veri_sayac == 7) {
                if (tartimString_s.length() >= 1)
                  XML_V7 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[7], (tartimString_s.length()));
                else 
                  XML_V7 = "";
              }
              else if (veri_sayac == 8) {
                if (tartimString_s.length() >= 1)
                  XML_V8 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[8], (tartimString_s.length()));
                else 
                  XML_V8 = "";
              }
              else if (veri_sayac == 9) {
                if (tartimString_s.length() >= 1)
                  XML_V9 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[9], (tartimString_s.length()));
                else 
                  XML_V9 = "";
              }
              else if (veri_sayac == 10) {
                if (tartimString_s.length() >= 1)
                  XML_V10 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[10], (tartimString_s.length()));
                else 
                  XML_V10 = "";
              }
              else if (veri_sayac == 11) {
                if (tartimString_s.length() >= 1)
                  XML_V11 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[11], (tartimString_s.length()));
                else 
                  XML_V11 = "";
              }
              else if (veri_sayac == 12) {
                 if (tartimString_s.length() >= 1)
                  XML_V12 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[12], (tartimString_s.length()));
                else 
                  XML_V12 = "";
              }
              else if (veri_sayac == 13) {
                if (tartimString_s.length() >= 1)
                  XML_V13 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[13], (tartimString_s.length()));
                else 
                  XML_V13 = "";
              }
              else if (veri_sayac == 14) {
                if (tartimString_s.length() >= 1)
                  XML_V14 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[14], (tartimString_s.length()));
                else 
                  XML_V14 = "";
              }
              else if (veri_sayac == 15) {
                if (tartimString_s.length() >= 1) 
                  XML_V15 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[15], (tartimString_s.length()));
                else 
                  XML_V15 = "";
              }
              else if (veri_sayac == 16) {
                if (tartimString_s.length() >= 1)
                  XML_V16 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[16], (tartimString_s.length()));
                else 
                  XML_V16 = "";
              }
              else if (veri_sayac == 17) {
                if (tartimString_s.length() >= 1)
                  XML_V17 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[17], (tartimString_s.length()));
                else 
                  XML_V17 = "";
              }
              else if (veri_sayac == 18) {
                if (tartimString_s.length() >= 1)
                  XML_V18 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[18], (tartimString_s.length()));
                else 
                  XML_V18 = "";
              }
              else if (veri_sayac == 19) {
                if (tartimString_s.length() >= 1)
                  XML_V19 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[19], (tartimString_s.length()));
                else 
                  XML_V19 = "";
              }
              else if (veri_sayac == 20) {
                if (tartimString_s.length() >= 1)
                  XML_V20 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[20], (tartimString_s.length()));
                else 
                  XML_V20 = "";
              }
              else if (veri_sayac == 21) {
                if (tartimString_s.length() >= 1)
                  XML_V21 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[21], (tartimString_s.length()));
                else 
                  XML_V21 = "";
              }
              else if (veri_sayac == 22) {
                if (tartimString_s.length() >= 1)
                  XML_V22 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[22], (tartimString_s.length()));
                else 
                  XML_V22 = "";
              }
              else if (veri_sayac == 23) {
                if (tartimString_s.length() >= 1)
                  XML_V23 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[23], (tartimString_s.length()));
                else 
                  XML_V23 = "";
              }
              else if (veri_sayac == 24) {
                if (tartimString_s.length() >= 1)
                  XML_V24 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[24], (tartimString_s.length()));
                else 
                  XML_V24 = "";
              }
              else if (veri_sayac == 25) {
                if (tartimString_s.length() >= 1)
                  XML_V25 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[25], (tartimString_s.length()));
                else 
                  XML_V25 = "";
              }
              else if (veri_sayac == 26) {
                if (tartimString_s.length() >= 1)
                  XML_V26 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[26], (tartimString_s.length()));
                else 
                  XML_V26 = "";
              }
              else if (veri_sayac == 27) {
                if (tartimString_s.length() >= 1)
                  XML_V27 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[27], (tartimString_s.length()));
                else 
                  XML_V27 = "";
              }
              else if (veri_sayac == 28) {
                if (tartimString_s.length() >= 1)
                  XML_V28 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[28], (tartimString_s.length()));
                else 
                  XML_V28 = "";
              }
              /*else if (veri_sayac == 29) {
                if (tartimString_s.length() >= 1)                  
                  XML_V29 = tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[29], (tartimString_s.length()));
                else 
                  XML_V29 = "";
              }*/
            }
            if ((tartimString_s.substring(0,1) == "P") && (veri_sayac >= 24)) {//{//(XML_V29 == "P1.1") {
              XML_V29 = tartimString_s.substring(1,tartimString_s.indexOf('.'));
              yazdir_aktif = true;
            }
            tartimString_s = "";
            Serial.flush();
            veri_sayac++;
              //XML_V29 = "";
            //}
          }
        }
        success = true;
        break;
      }
  }
  return success;
}
#endif


/*
###ÜRÜN ETİKETİ###
V0_FORM       #eyz#form1.prn
V1_?
V2_BARKOD
V3_iSiM1
V4_SNO
V5_iSiM2
V6_iSiM3
V7_NET
V8_B_FiYAT
V9_TUTAR
V10_P_TARiH
V11_P_SAAT
V12_S_TARiH
V13_S_SAAT
V14_MESAJ1
V15_MESAJ2
V16_MESAJ3
V17_MESAJ4
V18_MESAJ5
V19_MESAJ6
V20_MESAJ7
V21_MESAJ8
V22_MESAJ9
V23_MESAJ10
V24_iSiM4
V25_iSiM5
V26_BRUT
V27_DARA
V28_Z
V29_P1.1

####TOTAL ETİKET###
V0_FORM       #eyz#total.prn
V1_?
V2_BARKOD
V3_iSiM1
V4_P_TARiH
V5_P_SAAT
V6_S_TARiH
V7_S_SAAT
V8_P_TARiH
V9_P_SAAT
V10_B_FiYAT
V11_TOPLAM_ADET
V12_TUTAR
V13_0
V14_TUTAR
V15_TUTAR
V16_0
V17_1 TL = 0
V18_NET
V19_0
V20_NET
V21_NET
V22_BRUT
V23_TOPLAM_YAZISI
V24_ZB
V25_P1.1
*/
/*####DÜZ KABLO ERKEK ERKEK#####*/