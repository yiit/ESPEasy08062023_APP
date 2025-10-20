#include "PLU_Hafizasi.h"

#include "src/Commands/InternalCommands.h"
#include "src/Helpers/WebServer_commandHelper.h"

#include "src/WebServer/ESPEasy_WebServer.h"

#include "src/ESPEasyCore/ESPEasyRules.h"

#include "src/WebServer/AccessControl.h"
#include "src/WebServer/HTML_wrappers.h"
#include "src/WebServer/Markup.h"
#include "src/WebServer/Markup_Buttons.h"
#include "src/WebServer/Markup_Forms.h"

#include "src/Globals/Settings.h"
#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/StringConverter.h"
#include "src/Static/WebStaticData.h"
#include "ESPEasy-Globals.h"

void plu_load(String button) {
  int pluno = 0;
  if (ESPEASY_FS.exists(FILE_PLU)) {
    fs::File form = ESPEASY_FS.open(FILE_PLU, "r");
    String s;
    while (form.position() < form.size()) {
      s = form.readStringUntil('\n');
      if (s.indexOf(",") > 0) {
        pluno++;
        int say = s.indexOf(",");
        String plu_adi_s = s.substring(0, say);
        plu_adi[pluno] = (char*)plu_adi_s.c_str();
        String plu = button;  //"/screen?cmd=eyzplu+";
        plu += pluno;
        //addButton(plu, plu_adi[pluno]);
        html_TR_TD_height(30);
        addRowLabel(String(pluno));
        addSubmitButton(plu_adi[pluno], plu);
        //addWideButtonPlusDescription(String(plu), plu_adi[pluno], String(pluno));
        s = "";
      } else {
        s.trim();
      }
    }
    form.close();
  }
}

void plu_select(String yazici, String carpi) {
  for (int i = 1; i < 500; i++) {
    if (web_server.hasArg("plu" + String(i)))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, (yazici + "pluart#" + String(i) + carpi).c_str());
  }
}