#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P109
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

#include <Wire.h>

//#######################################################################################################
//#################################### Plugin 109: SERiAL WEBSERVER  #######################################
//#######################################################################################################

#define PLUGIN_109
#define PLUGIN_ID_109 109
#define PLUGIN_NAME_109 "Communication - SERiAL WEBSERVER"
#define PLUGIN_VALUENAME1_109 "NET"

boolean Plugin_109(byte function, struct EventStruct* event, String& string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_109;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        //Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_109);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_109));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        /*pcf8574.pinMode(P0, OUTPUT, HIGH);
	      pcf8574.pinMode(P1, OUTPUT, HIGH);
	      pcf8574.pinMode(P2, OUTPUT, HIGH);
	      pcf8574.pinMode(P3, OUTPUT, HIGH);
	      pcf8574.pinMode(P4, INPUT);
	      pcf8574.pinMode(P5, INPUT);
	      pcf8574.pinMode(P6, INPUT);
	      pcf8574.pinMode(P7, INPUT);*/
        pcf8574.pinMode(P0, OUTPUT, LOW);
        pcf8574.pinMode(P1, OUTPUT, LOW);
        pcf8574.pinMode(P2, OUTPUT, LOW);
        pcf8574.pinMode(P3, OUTPUT, LOW);
        pcf8574.pinMode(P4, INPUT_PULLUP);
        pcf8574.pinMode(P5, INPUT_PULLUP);
        pcf8574.pinMode(P6, INPUT_PULLUP);
        pcf8574.pinMode(P7, INPUT_PULLUP);
        pcf8574.digitalWrite(P0, LOW);
        pcf8574.digitalWrite(P1, LOW);
        pcf8574.digitalWrite(P2, LOW);
        pcf8574.digitalWrite(P3, LOW);
        //XML_INPUT_PIN_C  = {'0','0','0','0'};
        XML_OUTPUT_PIN_C[0] = '0';
        XML_OUTPUT_PIN_C[1] = '0';
        XML_OUTPUT_PIN_C[2] = '0';
        XML_OUTPUT_PIN_C[3] = '0';
        pcf8574.begin();
        Settings.WebAPP = 109;
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (pcf8574.digitalRead(P4) == LOW)
          XML_INPUT_PIN_C[0] = '0';
        else
          XML_INPUT_PIN_C[0] = '1';
        if (pcf8574.digitalRead(P5) == LOW)
          XML_INPUT_PIN_C[1] = '0';
        else
          XML_INPUT_PIN_C[1] = '1';
        if (pcf8574.digitalRead(P6) == LOW)
          XML_INPUT_PIN_C[2] = '0';
        else
          XML_INPUT_PIN_C[2] = '1';
        if (pcf8574.digitalRead(P7) == LOW)
          XML_INPUT_PIN_C[3] = '0';
        else
          XML_INPUT_PIN_C[3] = '1';
        success = true;
        break;
      }

      /*case PLUGIN_SERIAL_IN:
      {
        while (Serial.available()) {
          char inChar = Serial.read();
          //if (inChar == 255) // binary data...
          //{
            //Serial.flush();
            //break;
          //}
          tartimString_s += (String)inChar;
          if (inChar == 't') {
            //for (uint8_t i = 0; i < tartimString_s.length(); i++) {
            int index = tartimString_s.indexOf(9);
            //while (tartimString_s.substring(i, i+1).toInt() == 9) {
            XML_PLU_ADI_S = tartimString_s.substring(0,index);
            //i = tartimString_s.length() + 1;
            for (uint8_t i = XML_PLU_ADI_S.length(); i < tartimString_s.length(); i++) {
              while (tartimString_s.substring(i, i+1) == ".") {
                XML_NET_S = tartimString_s.substring(i+4, i+9);
                i = tartimString_s.length() + 1;
              }
            }
            tartimString_s = "";                       
          }
        }
        success = true;
        break;
      }*/
  }
  return success;
}
#endif  // USES_P109
        //#endif