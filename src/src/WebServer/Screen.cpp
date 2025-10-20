#include "../WebServer/Screen.h"

#include "../WebServer/ESPEasy_WebServer.h"

#include "../../PLU_Hafizasi.h"
#include "../Commands/InternalCommands.h"

#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../WebServer/Markup_Forms.h"

#include "../ESPEasyCore/ESPEasyRules.h"

#include "../Globals/ExtraTaskSettings.h"
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Networking.h"
#include "../Helpers/StringConverter.h"
#include "../Static/WebStaticData.h"
#include "../../ESPEasy-Globals.h"
#include "../Helpers/StringParser.h"

#include <FS.h>

#ifdef WEBSERVER_SCREEN

//int sirano = 1;
int sayac1 = 0;
int sayac2 = 0;
int sayac3 = 0;
int sayac4 = 0;
/*int sonsuz_sayac1 = 0;
int sonsuz_sayac2 = 0;
int sonsuz_sayac3 = 0;
int sonsuz_sayac4 = 0;*/

void handle_screen() {
#ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_screen"));
#endif

  if (!isLoggedIn()) { return; }
  if (!isLisansIn()) { return; }

  navMenuIndex = MENU_INDEX_SCREEN;
  TXBuffer.startStream();

  html_add_form();
  html_table_class_normal();
  html_TR();
  //html_table_class_normal();
  //html_table_header({{logo}}, 100);
  html_table_header(ExtraTaskSettings.TaskDeviceName, 100);
  html_end_table();
  html_table_class_normal();

  if (Settings.WebAPP == 109) {
    html_table_header("MESAJ", 80);
    html_table_header("AGIRLIK", 80);

    html_table_header(F("<h5><A1><A1 ID='mesaj1'></A1></h5>"), 500);

    html_table_header(F("<h5><A1><A1 ID='mesaj2'></A1></h5>"), 500);
    html_table_header(F("<h5>kg</h5>"));
  } else {
    html_table_header(ExtraTaskSettings.TaskDeviceValueNames[0], 80);
    html_table_header(ExtraTaskSettings.TaskDeviceValueNames[1], 80);
    html_table_header(ExtraTaskSettings.TaskDeviceValueNames[2], 80);
    html_table_class_white();

    html_table_header(F(" "), 50);
    html_table_header(F("<h5><A1><A1 ID='mesaj1'></A1></h5>"), 500);
    //html_table_header(F("<h5>kg</h5>"));

    html_table_header(F("<h5><A1><A1 ID='mesaj2'></A1></h5>"), 500);
    //html_table_header(F("<h5>kg</h5>"));

    html_table_header(F("<h5><A1><A1 ID='mesaj3'></A1></h5>"), 500);
    //html_table_header(F("<h5>kg</h5>"));

    //html_table_header(ExtraTaskSettings.TaskDeviceName, 100);
    html_end_table();
    html_table_class_normal();
    html_table_header(ExtraTaskSettings.TaskDeviceValueNames[3], 80);
#ifdef ESP32
    html_table_header(ExtraTaskSettings.TaskDeviceValueNames[4], 80);
    html_table_header(ExtraTaskSettings.TaskDeviceValueNames[5], 80);
#endif
    html_table_class_white();

    html_table_header(F(" "), 50);
#ifdef ESP32
    html_table_header(F("<h5><A1><A1 ID='mesaj4'></A1></h5>"), 500);
    html_table_header(F("<h5></h5>"));
    html_table_header(F("<h5><A1><A1 ID='mesaj5'></A1></h5>"), 500);
    html_table_header(F("<h5></h5>"));

    html_table_header(F("<h5><A1><A1 ID='mesaj6'></A1></h5>"), 500);
    html_table_header(F("<h5></h5>"));

    //html_table_header(ExtraTaskSettings.TaskDeviceName, 100);
    html_end_table();
    html_table_class_normal();
    html_table_header(ExtraTaskSettings.TaskDeviceValueNames[6], 80);
    html_table_header(ExtraTaskSettings.TaskDeviceValueNames[7], 80);
    html_table_class_white();

    html_table_header(F(" "), 50);
    html_table_header(F("<h5><A1><A1 ID='mesaj6'></A1></h5>"), 500);
    html_table_header(F("<h5></h5>"));

    html_table_header(F("<h5><A1><A1 ID='mesaj7'></A1></h5>"), 500);
    html_table_header(F("<h5></h5>"));
#endif
  }

  String webrequest = web_server.arg(F("cmd"));
  if (webrequest.length() > 0) {
    struct EventStruct TempEvent;
    parseCommandString(&TempEvent, webrequest);
    TempEvent.Source = EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND;
    ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, webrequest.c_str());
  }

  if (Settings.WebAPP == 109) {
    html_table_class_white();
    if (web_server.hasArg(F("menu")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#5");
    addSubmitButton(F("MENU"), F("menu"));

    if (web_server.hasArg(F("yuklebosalt")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#4");
    addSubmitButton(F("YUKLE BOSALT"), F("yuklebosalt"));

    if (web_server.hasArg(F("yukariok")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#3");
    addSubmitButton(F("YUKARI OK"), F("yukariok"));

    if (web_server.hasArg(F("asagiok")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#2");
    addSubmitButton(F("ASAGI OK"), F("asagiok"));

    if (web_server.hasArg(F("sil")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#1");
    addSubmitButton(F("SiL"), F("sil"));

    if (web_server.hasArg(F("ok")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#0");
    addSubmitButton(F("OK"), F("ok"));
  }

  if ((Settings.WebAPP == 110) || (Settings.WebAPP == 112)) {
    if (web_server.arg(F("message1")).length() > 0) {
      message1 = web_server.arg(F("message1"));
    } else
      message1 = sayac1;
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("SAYAC 1");
    TXBuffer += F("<input class='wide' type='number' name='message1' value='");
    TXBuffer += message1;
    TXBuffer += F("'><TD>");
    if (web_server.hasArg(F("sayac1"))) {
      sayac1 = message1.toInt();
      XML_SAYAC_1_S = sayac1;
      dtostrf(XML_SAYAC_1_S.toInt(), 4, 0, XML_SAYAC_1_C);
      EEPROM.writeLong(100, uint32_t(XML_SAYAC_1_S.toInt()));
      EEPROM.commit();
    }
    addSubmitButton(F("SAYAC 1 GUNCELLE"), F("sayac1"));
    html_table_class_white();

    if (web_server.arg(F("message2")).length() > 0) {
      message2 = web_server.arg(F("message2"));
    } else
      message2 = sayac2;
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("SAYAC 2");
    TXBuffer += F("<input class='wide' type='number' name='message2' value='");
    TXBuffer += message2;
    TXBuffer += F("'><TD>");
    if (web_server.hasArg(F("sayac2"))) {
      sayac2 = message2.toInt();
      XML_SAYAC_2_S = sayac2;
      dtostrf(XML_SAYAC_2_S.toInt(), 4, 0, XML_SAYAC_2_C);
      EEPROM.writeLong(104, uint32_t(XML_SAYAC_2_S.toInt()));
      EEPROM.commit();
    }
    addSubmitButton(F("SAYAC 2 GUNCELLE"), F("sayac2"));
    html_table_class_white();

    if (web_server.arg(F("message3")).length() > 0) {
      message3 = web_server.arg(F("message3"));
    } else
      message3 = sayac3;
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("SAYAC 3");
    TXBuffer += F("<input class='wide' type='number' name='message3' value='");
    TXBuffer += message3;
    TXBuffer += F("'><TD>");
    if (web_server.hasArg(F("sayac3"))) {
      sayac3 = message3.toInt();
      XML_SAYAC_3_S = sayac3;
      dtostrf(XML_SAYAC_3_S.toInt(), 4, 0, XML_SAYAC_3_C);
      EEPROM.writeLong(118, uint32_t(XML_SAYAC_3_S.toInt()));
      EEPROM.commit();
    }
    addSubmitButton(F("SAYAC 3 GUNCELLE"), F("sayac3"));
    html_table_class_white();

    if (web_server.arg(F("message4")).length() > 0) {
      message4 = web_server.arg(F("message4"));
    } else
      message4 = sayac4;
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("SAYAC 4");
    TXBuffer += F("<input class='wide' type='number' name='message4' value='");
    TXBuffer += message4;
    TXBuffer += F("'><TD>");
    if (web_server.hasArg(F("sayac4"))) {
      sayac4 = message4.toInt();
      XML_SAYAC_4_S = sayac4;
      dtostrf(XML_SAYAC_4_S.toInt(), 4, 0, XML_SAYAC_4_C);
      EEPROM.writeLong(126, uint32_t(XML_SAYAC_4_S.toInt()));
      EEPROM.commit();
    }
    addSubmitButton(F("SAYAC 4 GUNCELLE"), F("sayac4"));
    html_table_class_white();

    /*if (web_server.arg(F("message5")).length() > 0) {
      message5 = web_server.arg(F("message5"));
    } else
      message5 = sonsuz_sayac1;
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("SONSUZ SAYAC 1");
    TXBuffer += F("<input class='wide' type='number' name='message5' value='");
    TXBuffer += message5;
    TXBuffer += F("'><TD>");
    if (web_server.hasArg(F("sayac3"))) {
      sonsuz_sayac1 = message3.toInt();
      XML_SAYAC_1_SONSUZ_S = sonsuz_sayac1;
      EEPROM.writeLong(110, uint32_t(XML_SAYAC_1_SONSUZ_S.toInt()));
      EEPROM.commit();
    }
    addSubmitButton(F("SONSUZ SAYAC 1 GUNCELLE"), F("sonsuzsayac1"));
    html_table_class_white();*/
  }

  if (Settings.WebAPP == 120) {
    /*if (web_server.arg(F("message1")).length() > 0)
      message1 = web_server.arg(F("message1"));
      else
      message1 = "1";
      TXBuffer += F("<form><table class='normal'>");
      TXBuffer += F("URUN ADI ");
      TXBuffer += F("<input class='wide' type='text' name='message1' value='");
      TXBuffer += message1;
      TXBuffer += F("'><TD>");*/
    html_table_class_white();
    if (web_server.hasArg(F("arti")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
    addSubmitButton(F("ARTI"), F("arti"));
    if (web_server.hasArg(F("topla")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
    addSubmitButton(F("TOPLAM"), F("topla"));
  }

  if (Settings.WebAPP == 121) {
    if (web_server.arg(F("message1")).length() > 0)
      message1 = web_server.arg(F("message1"));
    else
      message1 = "1";
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("URUN KODU ");
    TXBuffer += F("<input class='wide' type='text' name='message1' value='");
    TXBuffer += message1;
    TXBuffer += F("'>");
    addSubmitButton(F("URUN"), F("plu"));
    TXBuffer += F("<TD>");
    if (web_server.hasArg(F("arti")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
    addSubmitButton(F("ARTI"), F("arti"));
    if (web_server.hasArg(F("topla")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
    addSubmitButton(F("TOPLAM"), F("topla"));
    String button1 = "fyzpluart#";
    button1 += message1;
    button1 += "#0";
    if (web_server.hasArg(F("plu")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, button1.c_str());
    plu_select("fyz", "#0");
    addFormSeparator(2);
    addTableSeparator(F("URUN HAFIZASI"), 2, 3);
    plu_load(F("plu"));
  }

  if (Settings.WebAPP == 130) {
    if (web_server.arg(F("message1")).length() > 0) {
      message1 = web_server.arg(F("message1"));
    } else
      message1 = XML_SERI_NO_S;
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("SIRA NO ");
    TXBuffer += F("<input class='wide' type='number' name='message1' value='");
    TXBuffer += message1;
    TXBuffer += F("'><TD>");
    if (web_server.hasArg(F("sirano"))) {
      String button1 = "eyzserino#";
      button1 += message1;
      //sirano = message1.toInt();
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, button1.c_str());     
    }
    addSubmitButton(F("SIRA NO GUNCELLE"), F("sirano"));
    html_table_class_white();
    //String button1 = "eyzart ";
    //button1 += message1;
    if (web_server.hasArg(F("arti")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");  //button1.c_str());
    addSubmitButton(F("ARTI YAZDIR"), F("arti"));
    if (web_server.hasArg(F("toplam")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");  //button1.c_str());
    addSubmitButton(F("TOPLAM YAZDIR"), F("toplam"));
    if (web_server.hasArg(F("tek"))) {
      message1 = XML_SERI_NO_S;
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
      //sirano++;
    }
    addSubmitButton(F("TEK YAZDIR"), F("tek"));
  }

  if (Settings.WebAPP == 131) {
    if (web_server.arg(F("message1")).length() > 0)
      message1 = web_server.arg(F("message1"));
    else
      message1 = "1";
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("ETiKET SAYISI ");
    TXBuffer += F("<input class='wide' type='text' name='message1' value='");
    TXBuffer += message1;
    TXBuffer += F("'><TD>");

    if (web_server.arg(F("message2")).length() > 0)
      message2 = web_server.arg(F("message2"));
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("URUN  KODU");
    TXBuffer += F("<input class='wide' type='number' name='message2' value='");
    TXBuffer += message2;
    TXBuffer += F("'>");
    addSubmitButton(F("URUN"), F("plu"));
    TXBuffer += F("<TD>");

    /*if (web_server.arg(F("message3")).length() > 0)
      message3 = web_server.arg(F("message3"));
    else
      message3 = "";
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("CiNSi");
    TXBuffer += F("<input class='wide' type='text' name='message3' value='");
    TXBuffer += message3;
    TXBuffer += F("'>");
    TXBuffer += F("<TD>");

    if (web_server.arg(F("message4")).length() > 0)
      message4 = web_server.arg(F("message4"));
    else
      message4 = "";
    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("BOY");
    TXBuffer += F("<input class='wide' type='text' name='message4' value='");
    TXBuffer += message4;
    TXBuffer += F("'>");
    TXBuffer += F("<TD>");*/

    html_table_class_white();
    String button1 = "eyzart#";
    button1 += message1;
    if (web_server.hasArg(F("arti")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, button1.c_str());
    addSubmitButton(F("ARTI"), F("arti"));
    if (web_server.hasArg(F("topla")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
    addSubmitButton(F("TOPLAM"), F("topla"));
    String button2 = "eyzpluart#";
    button2 += message2;
    button2 += "#";
    button2 += message1;
    if (web_server.hasArg(F("plu")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, button2.c_str());
    plu_select("eyz", "#1#0");
    addFormSeparator(2);
    TXBuffer += "<input type=\"text\" id=\"myInput\" onkeyup=\"myFunction()\" placeholder=\"URUN ISMI ARAMA CUBUGU..\" title=\"Type in a name\" style='width:450px;' align='center'>";
    TXBuffer += "<table id=\"myTable\" class='multirow' border=2px frame='box' rules='all'><TR>";
    TXBuffer += "<TH style='width:50px;'>NO<TH>URUN ISMI<TH>URUN KODU<TH>DARA";
    //addTableSeparator(F("URUN HAFIZASI"), 2, 3); 
    plu_load(F("plu"));
  }

  if (Settings.WebAPP == 140) {
    if (web_server.arg(F("message1")).length() > 0)
      message1 = web_server.arg(F("message1"));

    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("KALiBRASYON KiLOSU ");
    TXBuffer += F("<input class='wide' type='number' name='message1' value='");
    TXBuffer += message1;
    TXBuffer += F("'><TD>");

    html_table_class_white();
    if (web_server.hasArg(F("sifirkal")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indsifirkal");
    addSubmitButton(F("SIFIR KALiBRASYONU"), F("sifirkal"));

    String button1 = "indyukkal#";
    button1 += message1;
    if (web_server.hasArg(F("yukkal")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, button1.c_str());
    addSubmitButton(F("YUK KALiBRASYONU"), F("yukkal"));
    addFormSeparator(2);
  }

  if (Settings.WebAPP == 107) {
    if (web_server.arg(F("message1")).length() > 0)
      message1 = web_server.arg(F("message1"));
    if (web_server.arg(F("message2")).length() > 0)
      message2 = web_server.arg(F("message2"));

    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("ALT LiMiT");
    TXBuffer += F("<input class='wide' type='number' name='message1' value='");
    TXBuffer += message1;
    TXBuffer += F("'><TD>");

    html_table_class_white();
    String button1 = "hedef1#";
    button1 += message1;
    if (web_server.hasArg(F("hedef1")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, button1.c_str());
    addSubmitButton(F("KAYDET"), F("hedef1"));
    addFormSeparator(2);

    TXBuffer += F("<form><table class='normal'>");
    TXBuffer += F("UST LiMiT");
    TXBuffer += F("<input class='wide' type='number' name='message2' value='");
    TXBuffer += message2;
    TXBuffer += F("'><TD>");

    html_table_class_white();
    String button2 = "hedef2#";
    button2 += message2;
    if (web_server.hasArg(F("hedef2")))
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, button2.c_str());
    addSubmitButton(F("KAYDET"), F("hedef2"));
  }

  if (Settings.WebAPP == 150) {
    TXBuffer += F("<TR><TD HEIGHT=\"30\">");
    addButton(F("/screen?cmd=zeropoint"), F("ZERO"));
    TXBuffer += F("<TD>");
    TXBuffer += F("<TR><TD HEIGHT=\"30\">");
    addButton(F("/screen?cmd=tarein"), F("TAREiN"));
    TXBuffer += F("<TD>");
    TXBuffer += F("<TR><TD HEIGHT=\"30\">");
    addButton(F("/screen?cmd=tareout"), F("TAREOUT"));
    TXBuffer += F("<TD>");
  }

  html_end_table();
  html_end_form();
  sendHeadandTail(F("TmplDsh"), _HEAD);
  TXBuffer.endStream();
}
#endif