// #define WEBSERVER_RULES_DEBUG

#include "../WebServer/Lisans.h"

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/Settings.h"
#include "../Globals/SecuritySettings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/WebServer_commandHelper.h"

#include "../Static/WebStaticData.h"

#include "../../ESPEasy-Globals.h"

#include "../Commands/InternalCommands.h"

#include <FS.h>

void handle_lisans() {
#ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_lisans"));
#endif

  if (!clientIPallowed()) return;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  String webrequest = web_server.arg(F("password"));
  //  char command[80];
  //  command[0] = 0;
  //  webrequest.toCharArray(command, 80);

  addHtml(F("<form method='post'>"));
  addHtml(F("<h5>Lisans Referans No = "));
  addHtml(String(randomNumber));
  addHtml(F("</h5>"));
  addHtml(F("<table class='normal'><TR><TD>Lisans Key<TD>"));
  addHtml(F("<input class='wide' type='password' name='password' value='"));
  addHtml(webrequest);
  addHtml("'>");
  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_TR_TD();
  html_end_table();
  html_end_form();

  long lisans_sifre = (randomNumber * 2) - 1942;
  if (webrequest.length() != 0) {
    char command[80];
    command[0] = 0;
    webrequest.toCharArray(command, 80);
    // compare with stored password and set timer if there's a match
    if (String(command) == String(lisans_sifre)) {
      SecuritySettings.LisansControl = true;
      SaveSettings();
      WebLisansInTimer = 0;
      addHtml(F("<script>window.location = '.'</script>"));
    } else {
      addHtml(F("Invalid Lisans Key!"));
      if (Settings.UseRules) {
        String event = F("Login#Failed");
        rulesProcessing(event);
      }
    }
  }

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb = false;
}

void handle_lisans_SERiALSERVER() {
  appLisans(100, "SERiAL SERVER");
}
void handle_lisans_SERiALCLiENT() {
  appLisans(101, "SERiAL CLiENT");
}
void handle_lisans_SERiALTCPMODBUSSLAVE() {
  appLisans(102, "SERiAL TCP/MODBUS SLAVE");
}
void handle_lisans_TCPMODBUSHRCMESAJ() {
  appLisans(104, "TCP/MODBUS HRCMESAJ");
}
void handle_lisans_HRC() {
  appLisans(110, "HRC");
}
void handle_lisans_FYZ() {
  appLisans(120, "FYZ");
}
void handle_lisans_FYZPLU() {
  appLisans(121, "FYZPLU");
}
void handle_lisans_EYZ() {
  appLisans(130, "EYZ");
}
void handle_lisans_EYZPLU() {
  appLisans(131, "EYZPLU");
}

void handle_lisans_IND() {
  appLisans(140, "IND");
}

void handle_lisans_BLE_CLiENT() {
  appLisans(105, "BLE_CLiENT");
}

void handle_lisans_BLE_SERVER() {
  appLisans(106, "BLE_SERVER");
}

void appLisans(int key, String lisans_name) {
#ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_lisans"));
#endif

  if (!clientIPallowed()) return;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);

  String webrequest = web_server.arg(F("password"));

  addHtml(F("<form method='post'>"));
  addHtml(F("<h5>"));
  addHtml(lisans_name);
  addHtml(F(" Referans No = "));
  addHtml(String(randomNumber));
  addHtml(F("</h5>"));
  addHtml(F("<table class='normal'><TR><TD>Lisans Key<TD>"));
  addHtml(F("<input class='wide' type='password' name='password' value='"));
  addHtml(webrequest);
  addHtml("'>");
  html_TR_TD();
  html_TD();
  addSubmitButton();
  html_TR_TD();
  html_end_table();
  html_end_form();

  long lisans_sifre = (randomNumber * 2) - key;
  if (webrequest.length() != 0) {
    char command[80];
    command[0] = 0;
    webrequest.toCharArray(command, 80);
    // compare with stored password and set timer if there's a match
    if (String(command) == String(lisans_sifre)) {
      switch (key) {
        case 100: SecuritySettings.LisansControlSERiALSERVER = true; break;
        case 101: SecuritySettings.LisansControlSERiALCLiENT = true; break;
        case 102: SecuritySettings.LisansControlSERiALTCPMODBUSSLAVE = true; break;
        case 104: SecuritySettings.LisansControlTCPMODBUSHRCMESAJ = true; break;
        case 110: SecuritySettings.LisansControlHRC = true; break;
        case 120: SecuritySettings.LisansControlFYZ = true; break;
        case 121: SecuritySettings.LisansControlFYZPLU = true; break;
        case 130: SecuritySettings.LisansControlEYZ = true; break;
        case 131: SecuritySettings.LisansControlEYZPLU = true; break;
        case 140: SecuritySettings.LisansControlIND = true; break;
        case 105: SecuritySettings.LisansControlBLECLiENT = true; break;
        case 106: SecuritySettings.LisansControlBLESERVER = true; break;
      }
      SaveSettings();
      WebLisansInTimer = 0;
      addHtml(F("<script>window.location = '.'</script>"));
    } else {
      addHtml(F("Invalid Lisans Key!"));
      if (Settings.UseRules) {
        String event = F("Login#Failed");
        rulesProcessing(event);
      }
    }
  }

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
  printWebString = "";
  printToWeb = false;
}
