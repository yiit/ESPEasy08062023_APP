#include "../Commands/IND.h"

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

String Command_Ind_Sifir_Kal(struct EventStruct *event, const char* Line)
{
  //Settings.ind_sifir_kal_degeri = scale.read();
  Settings.ind_sifir_kal_degeri = scale.readADCValue();
  return return_command_success();
}

String Command_Ind_Yuk_Kal(struct EventStruct *event, const char* Line)
{
  String TmpStr1 = "0";
  float kal_yuk_degeri = 0;
  if (GetArgv(Line, TmpStr1, 2))
    kal_yuk_degeri = TmpStr1.toFloat();
  /*if (HasArgv(Line, 2)) {
    kal_yuk_degeri = event->Par1;
  } else  {
    String result = F("Ind:");
    result += kal_yuk_degeri;
    return return_result(event, result);
  }*/
  //float katsayi = float(kal_yuk_degeri) / (float(scale.read()) - Settings.ind_sifir_kal_degeri);
  float katsayi = float(kal_yuk_degeri) / (float(scale.readADCValue()) - Settings.ind_sifir_kal_degeri);
  Settings.ind_yuk_kal_degeri = katsayi;
  Serial.println(Settings.ind_yuk_kal_degeri);
  SaveSettings();
  return return_command_success();
}

String Command_Ind_Sifir(struct EventStruct *event, const char* Line)
{
  //ind_sifir_degeri = scale.read() - Settings.ind_sifir_kal_degeri;
  ind_sifir_degeri = scale.readADCValue() - Settings.ind_sifir_kal_degeri;
  return return_command_success();
}

String Command_Ind_Elle_Dara(struct EventStruct *event, const char* Line)
{
  String TmpStr1 = "0";
  float ind_elle_dara_degeri = 0;
  if (GetArgv(Line, TmpStr1, 2))
    ind_elle_dara_degeri = TmpStr1.toFloat();
  /*if (HasArgv(Line, 2)) {
    ind_elle_dara_degeri = event->Par1;
  } else  {
    String result = F("Ind:");
    result += ind_elle_dara_degeri;
    return return_result(event, result);
  }*/
  ind_dara_degeri = ind_elle_dara_degeri * -1;
  return return_command_success();
}

String Command_Ind_Dara_Ekle(struct EventStruct *event, const char* Line)
{ 
  if ((XML_NET_S.toFloat() > 0.0001) && (ind_dara_degeri >= 0.0001))
    ind_dara_degeri = float(XML_NET_S.toFloat() - float(ind_dara_degeri)) * -1;
  else if ((XML_NET_S.toFloat() > 0.0001) && (ind_dara_degeri < 0.0001))
    ind_dara_degeri = float(float(ind_dara_degeri) + (XML_NET_S.toFloat()*-1));
  //else
    //ind_dara_degeri = long(XML_NET_S.toFloat() - float(ind_dara_degeri));
  return return_command_success();
}

String Command_Ind_Dara_Sil(struct EventStruct *event, const char* Line)
{
  ind_dara_degeri = 0;
  return return_command_success();
}

String Command_Ind_Seri_Data(struct EventStruct *event, const char* Line)
{
  if (fileExists(Settings.seridata_prn)) {
    fs::File form = tryOpenFile(Settings.seridata_prn, "r");
    String s;
    while (form.position() < form.size())
    {
      s = form.readStringUntil('\r');
      s.replace("%stabil%",     XML_STABIL_S);
      s.replace("%net%",        XML_NET_S);
      s.replace("%dara%",       XML_DARA_S);
      s.replace("%brut%",       XML_BRUT_S);
      s.trim();
      Serial.println(s);
    }
    form.close();
  }
  return return_see_serial(event);
}