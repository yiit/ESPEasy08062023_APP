// #define WEBSERVER_RULES_DEBUG

#include "../WebServer/json_net.h"

#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/StringConverter.h"
#include "../Static/WebStaticData.h"
#include "../../ESPEasy-Globals.h"
#include "../Globals/ESPEasy_time.h"

#include <ArduinoJson.h>


#include <FS.h>

void handle_json_net() {
#ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_advanced"));
#endif

  StaticJsonDocument<128> doc;
  doc["data"] = json_net;

  String response;
  serializeJson(doc, response);

  json_server.sendHeader("Access-Control-Allow-Origin", "*");  // 👈 bunu mutlaka ekle
  json_server.send(200, "application/json", response);
}