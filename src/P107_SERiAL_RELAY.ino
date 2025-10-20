#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P107

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
//#################################### Plugin 107: SERiAL SERVER  #######################################
//#######################################################################################################

#define PLUGIN_107
#define PLUGIN_ID_107 107
#define PLUGIN_NAME_107 "Communication - SERiAL RELAY"
#define PLUGIN_VALUENAME1_107  "NET"
#define PLUGIN_VALUENAME2_107  "DARA"
#define PLUGIN_VALUENAME3_107  "BRUT"
#define PLUGIN_VALUENAME4_107  "ADET"
#define PLUGIN_VALUENAME5_107  "ADETGR"
#define PLUGIN_VALUENAME6_107  "PLUNO"
#define PLUGIN_VALUENAME7_107  "B.FIYAT"
#define PLUGIN_VALUENAME8_107  "TUTAR"
#define PLUGIN_VALUENAME9_107  "NET_2"
#define PLUGIN_VALUENAME10_107 "DARA_2"

#define REL_ASCII      ExtraTaskSettings.TaskDevicePluginConfig[0]

#define REL_INDIKATOR  ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define REL_MOD        ExtraTaskSettings.TaskDevicePluginConfigLong[1]

int8_t hedef1_pin = 12;
int8_t hedef2_pin = 14;
int8_t hedef3_pin = 13;
int8_t hedef4_pin = 27;

boolean Plugin_107_init = false;

boolean Plugin_107(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  static byte connectionState = 0;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_107;
        //Device[deviceCount].Type = DEVICE_TYPE_SERIAL;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        //Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_STRING;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 10;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_107);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_107));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_107));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_107));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_107));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_107));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_107));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_107));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_107));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_107));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_107));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormCheckBox(F("ASCII"), F("plugin_107_ascii"), REL_ASCII);
        
        byte choice1 = REL_MOD;
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
        int optionValues1[7];
        optionValues1[0] = 0;
        optionValues1[1] = 1;
        optionValues1[2] = 2;
        optionValues1[3] = 3;
        optionValues1[4] = 4;
        optionValues1[5] = 5;
        optionValues1[6] = 6;
        addFormSelector(F("Mod"), F("plugin_107_mod"), 7, options1, optionValues1, choice1);

        addFormSubHeader(F("İndikatör Ayarları"));
        indikator_secimi(event, REL_INDIKATOR, F("plugin_107_indikator"));
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        REL_INDIKATOR  = getFormItemInt(F("plugin_107_indikator"));
        REL_MOD        = getFormItemInt(F("plugin_107_mod"));
        REL_ASCII      = isFormItemChecked(F("plugin_107_ascii"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));

        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, REL_INDIKATOR, PCONFIG(4));

        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        pinMode(hedef1_pin, OUTPUT);
        pinMode(hedef2_pin, OUTPUT);
        pinMode(hedef3_pin, OUTPUT);
        pinMode(hedef4_pin, OUTPUT);
        digitalWrite(hedef1_pin, HIGH);
        digitalWrite(hedef2_pin, HIGH);
        digitalWrite(hedef3_pin, HIGH);
        digitalWrite(hedef4_pin, HIGH);
        Settings.WebAPP = 107;
        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        if (XML_NET_S.toFloat() > Settings.hedef1_f) {
          digitalWrite(hedef1_pin, LOW);
          digitalWrite(hedef2_pin, HIGH);
          digitalWrite(hedef3_pin, HIGH);
          digitalWrite(hedef4_pin, LOW);
        }
        else {
          digitalWrite(hedef1_pin, HIGH);
          digitalWrite(hedef2_pin, HIGH);
          digitalWrite(hedef3_pin, LOW);
          digitalWrite(hedef4_pin, HIGH);
        }
        if (XML_NET_S.toFloat() < Settings.hedef2_f) {
          digitalWrite(hedef1_pin, HIGH);
          digitalWrite(hedef2_pin, LOW);
          digitalWrite(hedef3_pin, HIGH);
          digitalWrite(hedef4_pin, LOW);
        }
        else {
          digitalWrite(hedef1_pin, HIGH);
          digitalWrite(hedef2_pin, HIGH);
          digitalWrite(hedef3_pin, LOW);
          digitalWrite(hedef4_pin, HIGH);
        }
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        if ((REL_MOD == 0) || (REL_MOD == 2) || (REL_MOD == 4))
          serial_error(event, PCONFIG_LONG(2), "");
        success = true;
        break;
      }

#ifdef HAS_WIFI
    case PLUGIN_SERIAL_IN:
      {
          while (Serial.available()) {
            char inChar = Serial.read();
            if (inChar == 255) {
              Serial.flush();
              break;
            }
            if (REL_ASCII) {
              if (isprint(inChar))
                tartimString_s += (String)inChar;
            } else
              tartimString_s += (String)inChar;
            if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
              hataTimer_l = millis();
              if (Settings.Tersle)
                tersle(event, tartimString_s);
              isaret(event, REL_INDIKATOR, tartimString_s);
              if ((REL_MOD == 3) || (REL_MOD == 4))
                formul_kontrol(event, tartimString_s, REL_MOD, false);
              else {
                formul_seri(event, tartimString_s, REL_MOD);
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
#endif  // USES_P107