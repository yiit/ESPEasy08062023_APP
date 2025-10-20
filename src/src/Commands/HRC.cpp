#include "../Commands/HRC.h"

#include "../../ESPEasy-Globals.h"
#include "../../ESPEasy_common.h"

#include "../Commands/Common.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/SecuritySettings.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Memory.h"
#include "../Helpers/Misc.h"
#include "../Helpers/StringConverter.h"

#include "../DataStructs/TimingStats.h"

String Command_Hrc_Mesaj(struct EventStruct *event, const char* Line)
{
  String TmpStr1 = " ";
  if (GetArgv(Line, TmpStr1, 2))
    Serial1.println(TmpStr1);
  return return_command_success();
}