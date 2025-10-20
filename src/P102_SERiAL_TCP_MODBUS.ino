#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P102

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
//#################################### Plugin 102: SERiAL TCP/MODBUS  #######################################
//#######################################################################################################

#define PLUGIN_102
#define PLUGIN_ID_102 102
#define PLUGIN_NAME_102 "Communication - SERiAL TCP/MODBUS SLAVE"
#define PLUGIN_VALUENAME1_102 "NET"
#define PLUGIN_VALUENAME2_102 "DARA"
#define PLUGIN_VALUENAME3_102 "BRUT"
#define PLUGIN_VALUENAME4_102 "ADET"
#define PLUGIN_VALUENAME5_102 "ADETGR"
#define PLUGIN_VALUENAME6_102 "PLUNO"
#define PLUGIN_VALUENAME7_102 "B.FIYAT"
#define PLUGIN_VALUENAME8_102 "TUTAR"
#define PLUGIN_VALUENAME9_102 "NET_2"
#define PLUGIN_VALUENAME10_102 "DARA_2"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#include <ETH.h>
#endif

#define MODBUS_SLAVE_Indikator ExtraTaskSettings.TaskDevicePluginConfigLong[0]

#define MODBUS_SLAVE_Mod ExtraTaskSettings.TaskDevicePluginConfigLong[1]

#include "ModbusTCPSlave.h"
ModbusTCPSlave Mb_102;
boolean Plugin_102_init = false;

boolean Plugin_102(byte function, struct EventStruct* event, String& string) {
  boolean success = false;
  switch (function) {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_102;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        //Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 10;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }
    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_102);
        break;
      }
    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_102));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_102));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_102));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_102));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_102));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_102));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_102));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_102));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_102));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_102));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
        indikator_secimi(event, MODBUS_SLAVE_Indikator, F("plugin_102_indikator"));
        byte choice1 = MODBUS_SLAVE_Mod;
        String options1[3];
        options1[0] = F("OTOMATiK");
        options1[1] = F("TERAZiDEN TUŞ iLE");
        options1[2] = F("KONTROL");
        int optionValues1[3];
        optionValues1[0] = 0;
        optionValues1[1] = 1;
        optionValues1[2] = 2;
        addFormSelector(F("MOD"), F("plugin_102_mod"), 3, options1, optionValues1, choice1);
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("Baslangıç-Bitiş Datasının Değişimine İzin Veriri"));
        success = true;
        break;
      }
    case PLUGIN_WEBFORM_SAVE:
      {
        MODBUS_SLAVE_Indikator = getFormItemInt(F("plugin_102_indikator"));
        MODBUS_SLAVE_Mod = getFormItemInt(F("plugin_102_mod"));        
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));        
        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, MODBUS_SLAVE_Indikator, PCONFIG(4));
        success = true;
        break;
      }
    case PLUGIN_INIT:
      {
        //Settings.WebAPP = 102;
        success = true;
        break;
      }
    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_102_init) {
        Mb_102.Run();        
        unsigned int reg0 = f_2uint_int1(XML_NET_S.toFloat());
        unsigned int reg1 = f_2uint_int2(XML_NET_S.toFloat());
        Mb_102.MBHoldingRegister[0] = reg0;
        Mb_102.MBHoldingRegister[1] = reg1;
        unsigned int reg2 = f_2uint_int1(XML_DARA_S.toFloat());
        unsigned int reg3 = f_2uint_int2(XML_DARA_S.toFloat());
        Mb_102.MBHoldingRegister[2] = reg2;
        Mb_102.MBHoldingRegister[3] = reg3;
        unsigned int reg4 = f_2uint_int1(XML_BRUT_S.toFloat());
        unsigned int reg5 = f_2uint_int2(XML_BRUT_S.toFloat());
        Mb_102.MBHoldingRegister[4] = reg4;
        Mb_102.MBHoldingRegister[5] = reg5;
        
        //unsigned int reg6 = f_2uint_int1(XML_NET_S_2.toFloat());
        //unsigned int reg7 = f_2uint_int2(XML_NET_S_2.toFloat());
        //Mb_102.MBHoldingRegister[6] = reg6;
        //Mb_102.MBHoldingRegister[7] = reg7;
        //unsigned int reg8 = f_2uint_int1(XML_DARA_S_2.toFloat());
        //unsigned int reg9 = f_2uint_int2(XML_DARA_S_2.toFloat());
        //Mb_102.MBHoldingRegister[8] = reg8;
        //Mb_102.MBHoldingRegister[9] = reg9;
        //unsigned int reg10 = f_2uint_int1(XML_BRUT_S_2.toFloat());
        //unsigned int reg11 = f_2uint_int2(XML_BRUT_S_2.toFloat());
        //Mb_102.MBHoldingRegister[10] = reg10;
        //Mb_102.MBHoldingRegister[11] = reg11;
      
        unsigned int reg6 = f_2uint_int1(XML_ADET_S.toFloat());
        unsigned int reg7 = f_2uint_int2(XML_ADET_S.toFloat());
        Mb_102.MBHoldingRegister[6] = reg6;
        Mb_102.MBHoldingRegister[7] = reg7;
        unsigned int reg8 = f_2uint_int1(XML_ADET_GRAMAJ_S.toFloat());
        unsigned int reg9 = f_2uint_int2(XML_ADET_GRAMAJ_S.toFloat());
        Mb_102.MBHoldingRegister[8] = reg8;
        Mb_102.MBHoldingRegister[9] = reg9;
        } 
        success = true;
        break;
      }
    case PLUGIN_ONCE_A_SECOND: 
      {
        if (!Plugin_102_init) {
          Mb_102.begin();
          Plugin_102_init = true;
        }
        serial_error(event, MODBUS_SLAVE_Mod, "");
        success = true;
        break;
      }
#ifdef ESP32
#if FEATURE_ETHERNET
    case PLUGIN_SERIAL_IN:
     {
      if (Plugin_102_init) {
        while (Serial1.available()) {
          char inChar = Serial1.read();
          if (inChar == 255)  // binary data...
          {
            Serial1.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, MODBUS_SLAVE_Indikator, tartimString_s);
            if ((MODBUS_SLAVE_Mod == 1) || (MODBUS_SLAVE_Mod == 2))
              formul_kontrol(event, tartimString_s, MODBUS_SLAVE_Mod, false);
            else {
              formul_seri(event, tartimString_s, MODBUS_SLAVE_Indikator);
              Serial1.flush();
            }
            tartimString_s = "";
          }
        }
      }      
      success = true;
      break;
     }
#endif
#ifdef HAS_WIFI
    case PLUGIN_SERIAL_IN:
     {
      if (Plugin_102_init) {
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
          isaret(event, MODBUS_SLAVE_Indikator, tartimString_s);
          if ((MODBUS_SLAVE_Mod == 1) || (MODBUS_SLAVE_Mod == 2))
            formul_kontrol(event, tartimString_s, MODBUS_SLAVE_Mod, false);
          else {
            formul_seri(event, tartimString_s, MODBUS_SLAVE_Indikator);
            Serial.flush();
          }
          tartimString_s = "";
         }
       }
     }  
     success = true;
     break;
    }
#endif
#endif
  }
return success;
}
#endif  // USES_P102