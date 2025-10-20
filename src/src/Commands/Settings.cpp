#include "../Commands/Settings.h"

#include "../../ESPEasy_common.h"
#include "../../ESPEasy-Globals.h"

#include "../Commands/Common.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/SecuritySettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Memory.h"
#include "../Helpers/MDNS_Helper.h"
#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringGenerator_System.h"


String Command_Settings_Build(struct EventStruct *event, const char* Line)
{
	if (HasArgv(Line, 2)) {
	  Settings.Build = event->Par1;
	} else {
      return return_result(event, concat(F("Build:"), static_cast<int>(Settings.Build)));
	}
	return return_command_success_str();
}

String Command_Settings_Unit(struct EventStruct *event, const char* Line)
{
	if (HasArgv(Line, 2)) {
	  Settings.Unit = event->Par1;
	  update_mDNS();
	} else {
      return return_result(event, concat(F("Unit:"), static_cast<int>(Settings.Unit)));
	}
	return return_command_success_str();
}

String Command_Settings_Name(struct EventStruct *event, const char* Line)
{
	return Command_GetORSetString(event, F("Name:"),
							Line,
							Settings.Name,
							sizeof(Settings.Name),
							1);
}

String Command_Settings_Password(struct EventStruct *event, const char* Line)
{
	return Command_GetORSetString(event, F("Password:"),
				      Line,
				      SecuritySettings.Password,
				      sizeof(SecuritySettings.Password),
				      1
				      );
}

const __FlashStringHelper *  Command_Settings_Password_Clear(struct EventStruct *event, const char* Line)
{
	const String storedPassword = SecuritySettings.getPassword();
	if (storedPassword.length() > 0) {
		// There is a password set, so we must check it.
		const String password = parseStringKeepCase(Line, 2);
		if (!storedPassword.equals(password)) {
			return return_command_failed();
		}
        ZERO_FILL(SecuritySettings.Password);
	}
	return return_command_success();
}

const __FlashStringHelper * Command_Settings_Save(struct EventStruct *event, const char* Line)
{
	SaveSettings();
	return return_command_success();
}

const __FlashStringHelper * Command_Settings_Load(struct EventStruct *event, const char* Line)
{
	LoadSettings();
	return return_command_success();
}

const __FlashStringHelper * Command_Settings_Print(struct EventStruct *event, const char* Line)
{
	serialPrintln();

	serialPrintln(F("System Info"));
	serialPrint(F("  IP Address    : ")); serialPrintln(formatIP(NetworkLocalIP()));
	serialPrint(F("  Build         : ")); serialPrintln(String(get_build_nr()) + '/' + getSystemBuildString());
	serialPrint(F("  Name          : ")); serialPrintln(Settings.getName());
	serialPrint(F("  Unit          : ")); serialPrintln(String(static_cast<int>(Settings.Unit)));
	serialPrint(F("  WifiSSID      : ")); serialPrintln(SecuritySettings.WifiSSID);
	serialPrint(F("  WifiKey       : ")); serialPrintln(SecuritySettings.WifiKey);
	serialPrint(F("  WifiSSID2     : ")); serialPrintln(SecuritySettings.WifiSSID2);
	serialPrint(F("  WifiKey2      : ")); serialPrintln(SecuritySettings.WifiKey2);
	serialPrint(F("  Free mem      : ")); serialPrintln(String(FreeMem()));
	return return_see_serial(event);
}

String Command_Settings_Zero(struct EventStruct *event, const char* Line)
{
  pinMode(14,OUTPUT);
  digitalWrite(14,LOW);
  delay(300);
  digitalWrite(14,HIGH);
  return return_command_success();
}

String Command_Settings_Tare(struct EventStruct *event, const char* Line)
{
  pinMode(27,OUTPUT);
  digitalWrite(27,LOW);
  delay(300);
  digitalWrite(27,HIGH);
  return return_command_success();
}

#ifdef ESP32
String Command_Settings_Button(struct EventStruct *event, const char* Line)
{
  String TmpStr1 = "0";
  int output = 0;
  if (GetArgv(Line, TmpStr1, 2))
     output = TmpStr1.toInt();
  switch (output) {
    case 0: pcf8574.digitalWrite(P0, LOW); delay(400); pcf8574.digitalWrite(P0, HIGH); break;
    case 1: pcf8574.digitalWrite(P1, LOW); delay(400); pcf8574.digitalWrite(P1, HIGH); break;
    case 2: pcf8574.digitalWrite(P2, LOW); delay(400); pcf8574.digitalWrite(P2, HIGH); break;
    case 3: pcf8574.digitalWrite(P3, LOW); delay(400); pcf8574.digitalWrite(P3, HIGH); break;
    case 4: pcf8574.digitalWrite(P4, LOW); delay(400); pcf8574.digitalWrite(P4, HIGH); break;
    case 5: pcf8574.digitalWrite(P5, LOW); delay(400); pcf8574.digitalWrite(P5, HIGH); break;
    case 6: pcf8574.digitalWrite(P6, LOW); delay(400); pcf8574.digitalWrite(P6, HIGH); break;
    case 7: pcf8574.digitalWrite(P7, LOW); delay(400); pcf8574.digitalWrite(P7, HIGH); break;
  }
  return return_command_success();
}

String Command_Settings_Output(struct EventStruct *event, const char* Line)
{
  String TmpStr1 = "0";
  String TmpStr2 = "0";
  int output = 0;
  int mode = 1;
  if (GetArgv(Line, TmpStr1, 2))
     output = TmpStr1.toInt();
  if (GetArgv(Line, TmpStr2, 3))
     mode = TmpStr2.toInt();
  switch (output) {
    case 0: 
     if (mode == 1) {
      pcf8574.digitalWrite(P0, HIGH);
      XML_OUTPUT_PIN_C[0] = '1';
     } 
     else if (mode == 0) {
    	pcf8574.digitalWrite(P0, LOW);
    	XML_OUTPUT_PIN_C[0] = '0';
     } 
    break;
    case 1:    
     if (mode == 1) {
      pcf8574.digitalWrite(P1, HIGH);
      XML_OUTPUT_PIN_C[1] = '1';
     }
     else if (mode == 0) {
    	pcf8574.digitalWrite(P1, LOW);
    	XML_OUTPUT_PIN_C[1] = '0';
     }
    break;
    case 2:     
     if (mode == 1) {
      pcf8574.digitalWrite(P2, HIGH);
      XML_OUTPUT_PIN_C[2] = '1';
     } 
     else if (mode == 0) {
    	pcf8574.digitalWrite(P2, LOW);
    	XML_OUTPUT_PIN_C[2] = '0'; 
     }
    break;
    case 3:     
     if (mode == 1) {
      pcf8574.digitalWrite(P3, HIGH);
      XML_OUTPUT_PIN_C[3] = '1';
     } 
     else if (mode == 0) {
     	pcf8574.digitalWrite(P3, LOW);
     	XML_OUTPUT_PIN_C[3] = '0';
     } 
    break;
  }
  return return_command_success();
}
#endif

String Command_Hedef1_Settings(struct EventStruct *event, const char* Line)
{
  String TmpStr1 = "0";
  float hedef = 0;
  if (GetArgv(Line, TmpStr1, 2))
     hedef = TmpStr1.toFloat();
  Settings.hedef_f[0] = hedef;
  SaveSettings();
  SaveTaskSettings(event->TaskIndex);
	return return_command_success();
}

String Command_Srvnet(struct EventStruct *event, const char* Line)
{
  return return_result(event, concat(F("NET:"), XML_NET_C));
}

const __FlashStringHelper * Command_Settings_Reset(struct EventStruct *event, const char* Line)
{
	ResetFactory();
	reboot(ESPEasy_Scheduler::IntendedRebootReason_e::ResetFactoryCommand);
	return return_command_success();
}
