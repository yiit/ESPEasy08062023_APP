// #define WEBSERVER_RULES_DEBUG

#include "../WebServer/XML.h"

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

#include <FS.h>

void handle_xml() {
#ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_advanced"));
#endif

  String XML = "<?xml version='1.0' encoding=\"UTF-8\"?>";
  XML += "<xml>";

  XML += "<webtartim>";
  XML += XML_webapinettartim_C;
  XML += "</webtartim>";

  XML += "<webtartimson>";
  XML += XML_webapinettartim_son_C;
  XML += "</webtartimson>";

  XML += "<net>";
  XML += XML_NET_S;
  XML += "</net>";

  XML += "<dara>";
  XML += XML_DARA_S;
  XML += "</dara>";

  XML += "<brut>";
  XML += XML_BRUT_S;
  XML += "</brut>";

  XML += "<net2>";
  XML += XML_NET_S_2;
  XML += "</net2>";

  XML += "<dara2>";
  XML += XML_DARA_S_2;
  XML += "</dara2>";

  XML += "<brut2>";
  XML += XML_BRUT_S_2;
  XML += "</brut2>";

  XML += "<adet>";
  XML += XML_ADET_S;
  XML += "</adet>";

  XML += "<adetgramaj>";
  XML += XML_ADET_GRAMAJ_S;
  XML += "</adetgramaj>";

  XML += "<pluadi>";
  XML += XML_PLU_ADI_S;
  XML += "</pluadi>";

  XML += "<qrkod>";
  XML += XML_QRKOD_S;
  XML += "</qrkod>";

  XML += "<pluno>";
  XML += XML_PLU_NO_S;
  XML += "</pluno>";

  XML += "<tarih>";
  XML += node_time.getDateString('-');  //XML_TARIH_S;
  XML += "</tarih>";

  XML += "<saat>";
  XML += node_time.getTimeString(':');  //XML_SAAT_S;
  XML += "</saat>";

  XML += "<plukod>";
  XML += XML_PLU_KOD_S;
  XML += "</plukod>";

  XML += "<barkod>";
  XML += XML_BARKOD_S;
  XML += "</barkod>";

  XML += "<sno>";
  XML += XML_SNO_S;
  XML += "</sno>";

  XML += "<sayac1>";
  XML += XML_SAYAC_1_S;
  XML += "</sayac1>";

  XML += "<sayac1sonsuz>";
  XML += XML_SAYAC_1_SONSUZ_S;
  XML += "</sayac1sonsuz>";

  XML += "<duruszamani>";
  XML += XML_DURUS_ZAMANI_S;
  XML += "</duruszamani>";

  XML += "<sayac1gecikme>";
  XML += XML_SAYAC_1_GECIKME_S;
  XML += "</sayac1gecikme>";

  XML += "<sayac2>";
  XML += XML_SAYAC_2_S;
  XML += "</sayac2>";

  XML += "<sayac2gecikme>";
  XML += XML_SAYAC_2_GECIKME_S;
  XML += "</sayac2gecikme>";

  XML += "<sayac3>";
  XML += XML_SAYAC_3_S;
  XML += "</sayac3>";

  XML += "<sayac3gecikme>";
  XML += XML_SAYAC_3_GECIKME_S;
  XML += "</sayac3gecikme>";

  XML += "<sayac4>";
  XML += XML_SAYAC_4_S;
  XML += "</sayac4>";

  XML += "<sayac4gecikme>";
  XML += XML_SAYAC_4_GECIKME_S;
  XML += "</sayac4gecikme>";

  XML += "<serino>";
  XML += XML_SERI_NO_S;
  XML += "</serino>";

  XML += "<fisno>";
  XML += XML_FIS_NO_S;
  XML += "</fisno>";

  XML += "<stabil>";
  XML += XML_STABIL_S;
  XML += "</stabil>";

  XML += "<topnet>";
  XML += XML_TOP_NET_S;
  XML += "</topnet>";

  XML += "<topdara>";
  XML += XML_TOP_DARA_S;
  XML += "</topdara>";

  XML += "<topbrut>";
  XML += XML_TOP_BRUT_S;
  XML += "</topbrut>";

  XML += "<topadet>";
  XML += XML_TOP_ADET_S;
  XML += "</topadet>";

  XML += "<rfidkod>";
  XML += XML_RFIDKOD_S;
  XML += "</rfidkod>";

  XML += "<input>";
  XML += String(XML_INPUT_PIN_C);
  XML += "</input>";

  XML += "<output>";
  XML += String(XML_OUTPUT_PIN_C);
  XML += "</output>";

  XML += "<msg1>";
  XML += XML_MESAJ1_S;
  XML += "</msg1>";

  XML += "<msg2>";
  XML += XML_MESAJ2_S;
  XML += "</msg2>";

  XML += "<msg3>";
  XML += XML_MESAJ3_S;
  XML += "</msg3>";

  XML += "<msg4>";
  XML += XML_MESAJ4_S;
  XML += "</msg4>";

  XML += "<msg5>";
  XML += XML_MESAJ5_S;
  XML += "</msg5>";

  XML += "<msg6>";
  XML += XML_MESAJ6_S;
  XML += "</msg6>";

  XML += "<msg7>";
  XML += XML_MESAJ7_S;
  XML += "</msg7>";

  XML += "<msg8>";
  XML += XML_MESAJ8_S;
  XML += "</msg8>";

  XML += "<msg9>";
  XML += XML_MESAJ9_S;
  XML += "</msg9>";

/*#ifdef CAS_VERSION
  XML += "<V0>";
  XML += XML_V0;
  XML += "</V0>";
  XML += "<V1>";
  XML += XML_V1;
  XML += "</V1>";
  XML += "<V2>";
  XML += XML_V2;
  XML += "</V2>";
  XML += "<V3>";
  XML += XML_V3;
  XML += "</V3>";
  XML += "<V4>";
  XML += XML_V4;
  XML += "</V4>";
  XML += "<V5>";
  XML += XML_V5;
  XML += "</V5>";
  XML += "<V6>";
  XML += XML_V6;
  XML += "</V6>";
  XML += "<V7>";
  XML += XML_V7;
  XML += "</V7>";
  XML += "<V8>";
  XML += XML_V8;
  XML += "</V8>";
  XML += "<V9>";
  XML += XML_V9;
  XML += "</V9>";
  XML += "<V10>";
  XML += XML_V10;
  XML += "</V10>";
  XML += "<V11>";
  XML += XML_V11;
  XML += "</V11>";
  XML += "<V12>";
  XML += XML_V12;
  XML += "</V12>";
  XML += "<V13>";
  XML += XML_V13;
  XML += "</V13>";
  XML += "<V14>";
  XML += XML_V14;
  XML += "</V14>";
  XML += "<V15>";
  XML += XML_V15;
  XML += "</V15>";
  XML += "<V16>";
  XML += XML_V16;
  XML += "</V16>";
  XML += "<V17>";
  XML += XML_V17;
  XML += "</V17>";
  XML += "<V18>";
  XML += XML_V18;
  XML += "</V18>";
  XML += "<V19>";
  XML += XML_V19;
  XML += "</V19>";
  XML += "<V20>";
  XML += XML_V20;
  XML += "</V20>";
  XML += "<V21>";
  XML += XML_V21;
  XML += "</V21>";
  XML += "<V22>";
  XML += XML_V22;
  XML += "</V22>";
  XML += "<V23>";
  XML += XML_V23;
  XML += "</V23>";
  XML += "<V24>";
  XML += XML_V24;
  XML += "</V24>";
  XML += "<V25>";
  XML += XML_V25;
  XML += "</V25>";
  XML += "<V26>";
  XML += XML_V26;
  XML += "</V26>";
  XML += "<V27>";
  XML += XML_V27;
  XML += "</V27>";
  XML += "<V28>";
  XML += XML_V28;
  XML += "</V28>";
  XML += "<V29>";
  XML += XML_V29;
  XML += "</V29>";
#endif*/
  XML += "</xml>";
  yazdir_xml[0] = 0;
  String XML_SON;
  strncpy(yazdir_xml, XML.c_str(), XML.length());
  for (int i = 0; i < XML.length(); i++) {
    if ((yazdir_xml[i] >= 32) && (yazdir_xml[i] < 127))
      XML_SON += yazdir_xml[i];
  }
  web_server.send(200, "text/xml", XML_SON);
}
void handle_xml_xml() {
#ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_advanced"));
#endif

  String XML = "<?xml version='1.0' encoding=\"UTF-8\"?>";
  XML += "<xml>";

  XML += "<net>";
  XML += XML_NET_S;
  XML += "</net>";

  XML += "<dara>";
  XML += XML_DARA_S;
  XML += "</dara>";

  XML += "<brut>";
  XML += XML_BRUT_S;
  XML += "</brut>";

  XML += "<net2>";
  XML += XML_NET_S_2;
  XML += "</net2>";

  XML += "<dara2>";
  XML += XML_DARA_S_2;
  XML += "</dara2>";

  XML += "<brut2>";
  XML += XML_BRUT_S_2;
  XML += "</brut2>";

  XML += "<adet>";
  XML += XML_ADET_S;
  XML += "</adet>";

  XML += "<adetgramaj>";
  XML += XML_ADET_GRAMAJ_S;
  XML += "</adetgramaj>";

  XML += "<pluadi>";
  XML += XML_PLU_ADI_S;
  XML += "</pluadi>";

  XML += "<qrkod>";
  XML += XML_QRKOD_S;
  XML += "</qrkod>";

  XML += "<pluno>";
  XML += XML_PLU_NO_S;
  XML += "</pluno>";

  XML += "<tarih>";
  XML += node_time.getDateString('-');  //XML_TARIH_S;
  XML += "</tarih>";

  XML += "<saat>";
  XML += node_time.getTimeString(':');  //XML_SAAT_S;
  XML += "</saat>";

  XML += "<plukod>";
  XML += XML_PLU_KOD_S;
  XML += "</plukod>";

  XML += "<barkod>";
  XML += XML_BARKOD_S;
  XML += "</barkod>";

  XML += "<sno>";
  XML += XML_SNO_S;
  XML += "</sno>";

  XML += "<sayac1>";
  XML += XML_SAYAC_1_S;
  XML += "</sayac1>";

  XML += "<sayac2>";
  XML += XML_SAYAC_2_S;
  XML += "</sayac2>";

  XML += "<serino>";
  XML += XML_SERI_NO_S;
  XML += "</serino>";

  XML += "<fisno>";
  XML += XML_FIS_NO_S;
  XML += "</fisno>";

  XML += "<stabil>";
  XML += XML_STABIL_S;
  XML += "</stabil>";

  XML += "<topnet>";
  XML += XML_TOP_NET_S;
  XML += "</topnet>";

  XML += "<topdara>";
  XML += XML_TOP_DARA_S;
  XML += "</topdara>";

  XML += "<topbrut>";
  XML += XML_TOP_BRUT_S;
  XML += "</topbrut>";

  XML += "<topadet>";
  XML += XML_TOP_ADET_S;
  XML += "</topadet>";

  XML += "<rfidkod>";
  XML += XML_RFIDKOD_S;
  XML += "</rfidkod>";

  XML += "<input>";
  XML += String(XML_INPUT_PIN_C);
  XML += "</input>";

  XML += "<output>";
  XML += String(XML_OUTPUT_PIN_C);
  XML += "</output>";

  XML += "</xml>";
  web_server.send(200, "text/xml", XML);
}