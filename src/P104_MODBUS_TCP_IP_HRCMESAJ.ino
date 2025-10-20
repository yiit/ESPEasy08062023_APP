
#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P104

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

#include "src/ESPEasyCore/ESPEasyRules.h"
//#######################################################################################################
//#################################### Plugin 104: TCP/MODBUS HRCMESAJ ###########################
//#######################################################################################################

#define PLUGIN_104
#define PLUGIN_ID_104 104
#define PLUGIN_NAME_104 "Communication - TCP/MODBUS HRCMESAJ"
#define PLUGIN_VALUENAME1_104 "NET"
#define PLUGIN_VALUENAME2_104 "DARA"
#define PLUGIN_VALUENAME3_104 "BRUT"
#define PLUGIN_VALUENAME4_104 "ADET"
#define PLUGIN_VALUENAME5_104 "ADETGR"
#define PLUGIN_VALUENAME6_104 "PLUNO"
#define PLUGIN_VALUENAME7_104 "B.FIYAT"
#define PLUGIN_VALUENAME8_104 "TUTAR"
#define PLUGIN_VALUENAME9_104 "NET_2"
#define PLUGIN_VALUENAME10_104 "DARA_2"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#include <ETH.h>
#endif

#define MODBUS_HRCMESAJ_Gecikme ExtraTaskSettings.TaskDevicePluginConfigLong[9]
#define MODBUS_HRCMESAJ_msg_sayisi ExtraTaskSettings.TaskDevicePluginConfigLong[10]

#include "ModbusTCPSlave.h"
ModbusTCPSlave Mb_104;

uint8_t aktif_mesaj = 1;

String XML_MESAJX_S[8];

boolean Plugin_104_init = false;

boolean Plugin_104(byte function, struct EventStruct* event, String& string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_104;
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
        string = F(PLUGIN_NAME_104);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_104));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_104));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_104));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_104));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_104));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_104));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_104));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_104));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_104));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_104));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormNumericBox(F("Gösterim Süresi"), F("plugin_104_gecikme"), MODBUS_HRCMESAJ_Gecikme, 1, 255);
        addFormNote(F("<font color='red'>Saniye cinsinden.</font>"));
        addFormNumericBox(F("Mesaj Sayısı"), F("plugin_104_msg_sayisi"), MODBUS_HRCMESAJ_msg_sayisi, 1, 8);
        /*byte choice1 = MODBUS_HRCMESAJ_Mod;
        String options1[2];
        options1[0] = F("INT");
        options1[1] = F("FLOAT");
        int optionValues1[2];
        optionValues1[0] = 0;
        optionValues1[1] = 1;
        addFormSelector(F("Veri Tipi"), F("plugin_104_mod"), 2, options1, optionValues1, choice1);*/
        addFormNote(F("<font color='red'>Adres Datası 40001 ile 40255 arası girilmelidir.</font>"));

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
        int optionValues2[10];
        for (int val = 0;val < 10;val++) {
         optionValues2[val] = val;
        }
        byte choice2[8];
        for (int msg=0 ;msg < MODBUS_HRCMESAJ_msg_sayisi; msg++) {
          String adress = "INT ADRES ";
          adress += String(msg+1);
          String adress_html = "plugin_104_hi";
          adress_html += String(msg+1);
          adress_html += "_adres";
          addFormNumericBox(adress, adress_html, ExtraTaskSettings.TaskDevicePluginConfigLong[msg], 40001, 40255);
          choice2[msg] = ExtraTaskSettings.TaskDevicePrint[msg];
          String mesaj_adi = "Mesaj ";
          mesaj_adi += String(msg+1);
          String mesaj_html = "plugin_104_mesaj";
          mesaj_html += String(msg+1);
          mesaj_html += "_prn";
          addFormSelector(mesaj_adi, mesaj_html, 10 , options2, optionValues2, choice2[msg]);
          addButton(options2[ExtraTaskSettings.TaskDevicePrint[msg]], F("Mesaj Dizayn Menüsüne Git"));
        }            
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        MODBUS_HRCMESAJ_Gecikme = getFormItemInt(F("plugin_104_gecikme"));
        MODBUS_HRCMESAJ_msg_sayisi = getFormItemInt(F("plugin_104_msg_sayisi"));
        for (int msg = 0; msg < MODBUS_HRCMESAJ_msg_sayisi; msg++) {
          String adress = "plugin_104_hi";
          adress += String(msg+1);
          adress += "_adres";
          ExtraTaskSettings.TaskDevicePluginConfigLong[msg] = getFormItemInt(adress);
          String mesaj = "plugin_104_mesaj";
          mesaj += String(msg+1);
          mesaj += "_prn";
          ExtraTaskSettings.TaskDevicePrint[msg] = getFormItemInt(mesaj);
          options2[ExtraTaskSettings.TaskDevicePrint[msg]].toCharArray(Settings.mesaj[msg+1], options2[ExtraTaskSettings.TaskDevicePrint[msg]].length() + 1);
        }
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        Serial1.println("\nsil");
#ifdef ESP32
#if FEATURE_ETHERNET
        if (!Plugin_104_init) {
          delay(2000);
          Serial1.println("mesHRCMESAJ ");
          delay(2000);
          Serial1.println("sil");
          delay(200);
          Serial1.println("mesVER: 4.2 ");
          delay(2000);
          Serial1.println("sil");
          delay(200);
          Serial1.print("mes");
          Serial1.print(" ");
          Serial1.print(String(static_cast<int>(NetworkLocalIP()[0])));
          Serial1.print(".");
          Serial1.print(String(static_cast<int>(NetworkLocalIP()[1])));
          Serial1.println();
          delay(2000);
          Serial1.println("sil");
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
        gosterTimer_l = millis() + long(MODBUS_HRCMESAJ_Gecikme * 2000);
        hataTimer_l = millis();
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if (!Plugin_104_init) {
          Mb_104.begin();
          Plugin_104_init = true;
        }
        if ((millis() - hataTimer_l) > 10000){
          Serial1.println("mes ERROR  ");
        } else {
          for (int msg = 1;msg <= MODBUS_HRCMESAJ_msg_sayisi;msg++) {
            if ((millis() > gosterTimer_l) && (aktif_mesaj == msg)) {
              gosterTimer_l = millis() + long(MODBUS_HRCMESAJ_Gecikme * 1000);
              if (MODBUS_HRCMESAJ_msg_sayisi == msg)
                aktif_mesaj = 1;  
              else
                aktif_mesaj = msg+1;
              if (fileExists(Settings.mesaj[msg])) {
                fs::File form = tryOpenFile(Settings.mesaj[msg], "r");
                String file_data = "";
                while (form.position() < form.size()) {
                  file_data = form.readStringUntil('\n');
                  parseTemplate(file_data);
                  parse_string_commands(file_data);
                  Serial1.println(file_data);
                  Serial1.flush();
                  form.close();
                }
              }
            }  
          }
        }
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_104_init) {       
          Mb_104.Run();
          for (int msg = 0; msg < MODBUS_HRCMESAJ_msg_sayisi;msg++) {
            float bol = 1;
            switch(ExtraTaskSettings.TaskDeviceValueDecimals[msg]) {
              case 0: bol = 1; break;
              case 1: bol = 10; break;
              case 2: bol = 100; break;
              case 3: bol = 1000; break;
              case 4: bol = 10000; break; 
            }          
            if (int(Mb_104.MBHoldingRegister[int(ExtraTaskSettings.TaskDevicePluginConfigLong[msg]) - 40001]) > 32767)
              XML_MESAJX_S[msg] = String(int(Mb_104.MBHoldingRegister[int(ExtraTaskSettings.TaskDevicePluginConfigLong[msg]) - 40001])-65536);
            else
              XML_MESAJX_S[msg] = String(Mb_104.MBHoldingRegister[int(ExtraTaskSettings.TaskDevicePluginConfigLong[msg]) - 40001]);
            switch (msg) {
              case 0: XML_MESAJ1_S = dtostrf(XML_MESAJX_S[0].toFloat()/bol,ExtraTaskSettings.TaskDeviceValueBit[0],ExtraTaskSettings.TaskDeviceValueDecimals[0],XML_MESAJ1_C); break;
              case 1: XML_MESAJ2_S = dtostrf(XML_MESAJX_S[1].toFloat()/bol,ExtraTaskSettings.TaskDeviceValueBit[1],ExtraTaskSettings.TaskDeviceValueDecimals[1],XML_MESAJ2_C); break;
              case 2: XML_MESAJ3_S = XML_MESAJX_S[2]; break;
              case 3: XML_MESAJ4_S = XML_MESAJX_S[3]; break;
              case 4: XML_MESAJ5_S = XML_MESAJX_S[4]; break;
              case 5: XML_MESAJ6_S = XML_MESAJX_S[5]; break;
              case 6: XML_MESAJ7_S = XML_MESAJX_S[6]; break;
              case 7: XML_MESAJ8_S = XML_MESAJX_S[7]; break;
              }
          }
        /*} else if (MODBUS_HRCMESAJ_Mod == 1)
          XML_MESAJ1_S = String(f_2uint_float(Mb_104.MBHoldingRegister[MODBUS_HRCMESAJ_ADRES1_HI - 40001], Mb_104.MBHoldingRegister[MODBUS_HRCMESAJ_ADRES1_LOW - 40000]), int(ExtraTaskSettings.TaskDeviceValueDecimals[0]));*/
        }
        success = true;
        break;
      }
  }
  return success;
}
#endif  // USES_P104