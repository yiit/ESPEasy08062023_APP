/*#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
#define i2c_Address 0x3c  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
*/
#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P130

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
#include "src/Globals/Plugins.h"

#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/Memory.h"
#include "src/Helpers/StringConverter.h"
#include "src/Helpers/StringParser.h"
#include "src/Helpers/Networking.h"

#include "ESPEasy_common.h"
#include "ESPEasy-Globals.h"

//#include "ESP32Ping.h"
//#######################################################################################################
//##################################### Plugin 130: EYZ #################################################
//#######################################################################################################

#define PLUGIN_130
#define PLUGIN_ID_130 130
#define PLUGIN_NAME_130 "Printer - EYZ"
#define PLUGIN_VALUENAME1_130 "NET"
#define PLUGIN_VALUENAME2_130 "DARA"
#define PLUGIN_VALUENAME3_130 "BRUT"
#define PLUGIN_VALUENAME4_130 "ADET"
#define PLUGIN_VALUENAME5_130 "ADETGR"
#define PLUGIN_VALUENAME6_130 "QRKOD"
#define PLUGIN_VALUENAME7_130 "PLUNO"//B.FIYAT"
#define PLUGIN_VALUENAME8_130 "PLUADI"
#define PLUGIN_VALUENAME9_130 "NET_2"
#define PLUGIN_VALUENAME10_130 "DARA_2"
#define MAX_SRV_CLIENTS 5

#define CUSTOMTASK_STR_SIZE_P130 20

#define HEDEF_ADDR_SIZE_P130 8

#define MES_BUFF_SIZE_P130 19
#define HEDEF_BUFF_SIZE_P130 9

#define EYZ_Model ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define EYZ_Indikator ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define EYZ_Mod ExtraTaskSettings.TaskDevicePluginConfigLong[2]
#define EYZ_Gecikme ExtraTaskSettings.TaskDevicePluginConfigLong[3]

#define EYZ_Bartender ExtraTaskSettings.TaskPrintBartender
#define EYZ_ASCII ExtraTaskSettings.TaskDevicePluginConfig[0]

#define EYZ_art_komut ExtraTaskSettings.TaskDeviceMesage[0]
#define EYZ_tek_komut ExtraTaskSettings.TaskDeviceMesage[1]
#define EYZ_top_komut ExtraTaskSettings.TaskDeviceMesage[2]

#define EYZ_Hedef PCONFIG_FLOAT(0)

//WiFiServer *EyzServer;
//WiFiClient EyzClients[MAX_SRV_CLIENTS];

#include "OneButton.h"

//OneButton eyz_button2(12, false, false); //denedik
#ifdef ESP8266
//OneButton eyz_button2(14, false, false);  //RONGTA
OneButton eyz_button2(12, false, false);  //RONGTA
//OneButton eyz_button2(12, true, true); //HPRT
#endif
#ifdef ESP32
#if FEATURE_ETHERNET
OneButton eyz_button1(14, false, false);  //RONGTA
OneButton eyz_button2(15, false, false);  //RONGTA
//OneButton eyz_button2(22, true, true);  //HPRT
#endif
#ifdef HAS_WIFI
OneButton eyz_button1(21, false, false);  //RONGTA
OneButton eyz_button2(22, false, false);  //RONGTA

//OneButton eyz_button1(21, true, true);  //SK330
//OneButton eyz_button2(22, true, true);  //SK330
#endif
#endif
//OneButton eyz_button1(12, false, false);  //SAYAC

//OneButton eyz_button2(12, true, true); //HPRT
//OneButton eyz_button2(13, true, true); //HPRT
//OneButton eyz_button2(15, true, true); //HPRT
//OneButton eyz_button2(2, true, true);  //SD KARD AKTİF

bool internet_p130 = false;

/*uint8_t broadcastAddress_rcv[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
esp_now_peer_info_t peerInfo_rcv;*/

void eyz_click1() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");
  /*//ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
  int sayac = XML_SAYAC_S.toInt();
  sayac = sayac + 1;
  XML_SAYAC_S = String(sayac);
  dtostrf(XML_SAYAC_S.toInt(), 5, 0, XML_SAYAC_C);*/
}
/*void eyz_longPressStart1() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzsensor");
  //Settings.UseSerial = true;
}
void eyz_longPressStop1() {
  //Settings.UseSerial = false;
  int sayac = XML_SAYAC_S.toInt();
  sayac = sayac + 1;
  XML_SAYAC_S = String(sayac);
  dtostrf(XML_SAYAC_S.toInt(), 5, 0, XML_SAYAC_C);
}*/
void eyz_click2() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
  //XML_SAYAC_S = "0";
  //dtostrf(XML_SAYAC_S.toInt(), 5, 0, XML_SAYAC_C);
}
void eyz_longPressStart2() {
  //Settings.UseSerial = true;
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzsensor");
}
/*
void eyz_longPressStop2() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
}*/  
/*void onReceiveData(const uint8_t *mac, const uint8_t *data, int len) {
  //Serial.print("** Data Received **\n\n");
  //Serial.printf("Received from MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  //Serial.printf("Length: %d byte(s)\n", len);
  //Serial.printf("Data: %d\r\n", data);
  //memcpy(&myData, data, sizeof(myData));
  //Serial.write((char*)data, len);
  tartimdata_s = (char*)data;
  //Serial.print(tartimdata_s);
  PluginCall(PLUGIN_WRITE, 0, tartimdata_s);
}
void initESP_NOW_SRV() {
  if (esp_now_init() != ESP_OK) {
    //Serial.printf("Error initializing ESP-NOW\n");
    return;
  }
  memcpy(peerInfo_rcv.peer_addr, broadcastAddress_rcv, sizeof(broadcastAddress_rcv));
  peerInfo_rcv.channel = 1;  
  peerInfo_rcv.encrypt = false;
  if (esp_now_add_peer(&peerInfo_rcv) != ESP_OK){
    //Serial.printf("Failed to add peer\r\n");
    return;
  }
}*/
boolean Plugin_130(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  switch (function) {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_130;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 10;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }
    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_130);
        break;
      }
    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_130));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_130));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_130));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_130));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_130));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_130));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_130));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_130));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_130));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_130));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
#ifdef CAS_VERSION
        addFormSubHeader(F("Yazıcı Ayarları"));
#else
        addFormSubHeader(F("EYZ Ayarları"));
#endif
        addFormCheckBox(F("ASCII"), F("plugin_130_ascii"), EYZ_ASCII);
        byte choice0 = EYZ_Model;
        String options0[5];
        options0[0] = F("EYZ72R");
        options0[1] = F("EYZ100");
        options0[2] = F("EYZ100R");
        options0[3] = F("EYZ72Mobil");
        options0[4] = F("EYZ100Mobil");
        int optionValues0[5] = {0, 1, 2, 3, 4};
        addFormSelector(F("Yazıcı Model"), F("plugin_130_model"), 5, options0, optionValues0, choice0);
        byte choice1 = EYZ_Mod;
        String options1[7];
        options1[0] = F("SÜREKLi VERi (TEK SATIRLI VERi)");
        options1[1] = F("TERAZiDEN OTOMATiK (TEK SATIRLI VERi)");
        options1[2] = F("DENGELi OTOMATiK (TEK SATIRLI VERi)");
        options1[3] = F("TERAZiDEN TUŞ iLE (ÇOK SATIRLI VERi)");
        options1[4] = F("YAZICIDAN TUŞ iLE KONTROL(ÇOK SATIRLI VERi)");
        options1[5] = F("KUMANDA");
        options1[6] = F("VERi PAKETi");
        int optionValues1[7] = {0, 1, 2, 3, 4, 5, 6};
        addFormSelector(F("Yazdırma Modu"), F("plugin_130_mod"), 7, options1, optionValues1, choice1);
        addFormCheckBox(F("Bartender prn"), F("plugin_130_bartender"), EYZ_Bartender);
#ifdef ESP8266
        fs::Dir filedata = ESPEASY_FS.openDir("rules");
        int fileno = 0;
        while (filedata.next()) {
          options2[fileno] = filedata.fileName();
          fileno++;
        }
#endif
#ifdef ESP32
        fs::File root = ESPEASY_FS.open(F("/rules"));
        fs::File file = root.openNextFile();
        int fileno = 0;
        while (file) {
          if (!file.isDirectory()) {
            const String fname(file.name());
            if (fname.startsWith(F("/eyz")) || fname.startsWith(F("eyz"))) {
              //int count = getCacheFileCountFromFilename(fname);
              options2[fileno] = "/rules/";
              options2[fileno] += file.name();
              fileno++;
            }
          }
          file = root.openNextFile();
        }
#endif
        byte choice2 = ExtraTaskSettings.TaskDevicePrint[0];
        int optionValues2[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        addFormSelector(F("Tek Etiket"), F("plugin_130_tek_prn"), 10, options2, optionValues2, choice2);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Etiket Dizayn Menüsüne Git"));
        byte choice3 = ExtraTaskSettings.TaskDevicePrint[1];
        addFormSelector(F("Artı Etiket"), F("plugin_130_art_prn"), 10, options2, optionValues2, choice3);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Etiket Dizayn Menüsüne Git"));
        byte choice4 = ExtraTaskSettings.TaskDevicePrint[2];
        addFormSelector(F("Toplam Etiket"), F("plugin_130_top_prn"), 10, options2, optionValues2, choice4);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[2]], F("Etiket Dizayn Menüsüne Git"));
#if FEATURE_SD
        byte choice5 = ExtraTaskSettings.TaskDevicePrint[3];
        addFormSelector(F("SD data"), F("plugin_130_sd_prn"), 10, options2, optionValues2, choice5);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[3]], F("SD Data Dizayn Menüsüne Git"));
#endif
        if (EYZ_Mod == 2) {
          addFormTextBox(F("Hedef Kilogram"), F("plugin_130_hedef"), String(EYZ_Hedef, 3), HEDEF_BUFF_SIZE_P130);
          addFormNumericBox(F("Gecikme Saniyesi"), F("plugin_130_gecikme"), EYZ_Gecikme, 0, 999999);
        } else if (EYZ_Mod == 5) {
          addFormTextBox(F("Artı Komutu"), getPluginCustomArgName(0), EYZ_art_komut, MES_BUFF_SIZE_P130);
          addFormTextBox(F("Toplam Komutu"), getPluginCustomArgName(1), EYZ_top_komut, MES_BUFF_SIZE_P130);
          addFormTextBox(F("Tek Komutu"), getPluginCustomArgName(2), EYZ_tek_komut, MES_BUFF_SIZE_P130);
        }
        addFormSubHeader(F("İndikatör Ayarları"));
        indikator_secimi(event, EYZ_Indikator, F("plugin_130_indikator"));
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        EYZ_Model = getFormItemInt(F("plugin_130_model"));
        EYZ_Indikator = getFormItemInt(F("plugin_130_indikator"));
        EYZ_Mod = getFormItemInt(F("plugin_130_mod"));
        EYZ_Gecikme = getFormItemInt(F("plugin_130_gecikme"));
        EYZ_Bartender = isFormItemChecked(F("plugin_130_bartender"));
        EYZ_ASCII = isFormItemChecked(F("plugin_130_ascii"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));
        PCONFIG_FLOAT(0) = getFormItemFloat(F("plugin_130_hedef"));
        if (EYZ_Mod == 5) {
          strncpy_webserver_arg(EYZ_art_komut, getPluginCustomArgName(0));
          strncpy_webserver_arg(EYZ_top_komut, getPluginCustomArgName(1));
          strncpy_webserver_arg(EYZ_tek_komut, getPluginCustomArgName(2));
        }
        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, EYZ_Indikator, PCONFIG(4));
        ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_130_tek_prn"));
        ExtraTaskSettings.TaskDevicePrint[1] = getFormItemInt(F("plugin_130_art_prn"));
        ExtraTaskSettings.TaskDevicePrint[2] = getFormItemInt(F("plugin_130_top_prn"));
        ExtraTaskSettings.TaskDevicePrint[3] = getFormItemInt(F("plugin_130_sd_prn"));
        options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.tek_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[1]].toCharArray(Settings.art_prn, options2[ExtraTaskSettings.TaskDevicePrint[1]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[2]].toCharArray(Settings.top_prn, options2[ExtraTaskSettings.TaskDevicePrint[2]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[3]].toCharArray(Settings.sd_prn, options2[ExtraTaskSettings.TaskDevicePrint[3]].length() + 1);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        /*if (Settings.espnow_mod == 2)
          initESP_NOW_SRV();*/
        //eyz_button1 = new OneButton();
        //eyz_button2 = new OneButton();
        //switch (EYZ_Model) {
        // case 1: eyz_button1 = new OneButton(12, true, true);
        //         eyz_button2 = new OneButton(14, true, true);
        //         break;
        // case 2: eyz_button1 = new OneButton(12, false, false);
        //         eyz_button2 = new OneButton(14, false, false);
        //         break;
        //}
        eyz_button1.attachClick(eyz_click1);
        eyz_button1.attachDoubleClick(eyz_click1);
        eyz_button1.attachLongPressStart(eyz_click1);
        //eyz_button1.attachLongPressStop(eyz_longPressStop1);
        eyz_button2.attachClick(eyz_click2);
        eyz_button2.attachLongPressStart(eyz_longPressStart2);
        //eyz_button2.attachLongPressStop(eyz_longPressStop2);
        //eyz_button1.tick();
        eyz_button2.tick();
        //EyzServer = new WiFiServer(9100);
        //EyzServer->begin();
        if (!PCONFIG(4)) {
          switch (EYZ_Indikator) {
            case 26:
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[0], PSTR("NW"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[1], PSTR("TW"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[2], PSTR("GW"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[3], PSTR("QTY"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[4], PSTR("APW"));
              break;
            case 27:
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[0], PSTR("NET"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[1], PSTR("Tare"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[2], PSTR("Gross"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[3], PSTR("PCS"));
              strcpy_P(ExtraTaskSettings.TaskDeviceFormula[4], PSTR("U/W"));
              break;
          }
        }
/*#ifdef ESP8266
        display.begin(i2c_Address, true);
        display.display();
        display.clearDisplay();
#endif*/
        Settings.WebAPP = 130;
        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        /*if (Settings.espnow_mod == 2)
          esp_now_register_recv_cb(onReceiveData);*/
        //esp_err_t result = esp_now_send(broadcastAddress_rcv, (uint8_t *)&m, sizeof(int));
        //if (result == ESP_OK)
          //Serial.println("Data sent successfully\r\n");
        //else
          //Serial.println("Error sending the data\r\n");        
        //vTaskDelay(1000 / portTICK_PERIOD_MS);*/
/*#ifdef ESP8266
        display.clearDisplay();
        display.setTextSize(2);  // Normal 1:1 pixel scale
        display.setTextColor(SH110X_WHITE);
        display.setCursor(0, 0);  // Start at top-left corner
        display.println(F("  ADET :"));
        display.setTextSize(4);
        display.setCursor(0, 30);
        display.println(String(XML_SAYAC_C));
        display.display();
#endif*/
        switch (EYZ_Mod) {
          case 0:
            eyz_button1.tick();
            eyz_button2.tick();
            break;
          case 4:
            //eyz_button1.tick();
            eyz_button2.tick();
            break;
          case 2:
            if ((webapinettartim > EYZ_Hedef) && (hayvan_modu == 0)) {
              stabilTimer_l = millis() + (EYZ_Gecikme * 1000);
              StabilTartim_f = webapinettartim;
              hayvan_modu = 1;
            }
            if ((millis() > stabilTimer_l) && (hayvan_modu == 1)) {
              if (StabilTartim_f == webapinettartim) {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
                hayvan_modu = 2;
              } else
                hayvan_modu = 0;
            }
            if (((webapinettartim < EYZ_Hedef) && (webapinettartim > 0.001)) && (hayvan_modu == 2))
              hayvan_modu = 0;
            break;
        }
        success = true;
        break;
      }


    case PLUGIN_ONCE_A_SECOND:
      {
        //IPAddress host(172,217,20,67);
        /*IPAddress host(Settings.Cli_PrinterIP);
        if (Ping.ping(host,3))
          internet_p130 = true;
        else
          internet_p130 = false;
        Serial.println(internet_p130);*/
        serial_error(event, EYZ_Mod, "eyztek");
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (Settings.UDPPort > 0) {
          //string.replace("\n", "");
          //string.replace("\r", "");
          //string.replace(String(char(ExtraTaskSettings.TaskDeviceSonByte)), "");
          hataTimer_l = millis();
          Serial.print(string);
          if ((String(EYZ_art_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0]+2, (String(EYZ_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0]+2)) == String(EYZ_art_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");
          else if ((String(EYZ_top_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0]+2, (String(EYZ_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0]+2)) == String(EYZ_top_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
          else if ((String(EYZ_tek_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0]+2, (String(EYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0]+2)) == String(EYZ_tek_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
          else {
            if (Settings.Tersle)
              tersle(event, string);
            isaret(event, EYZ_Indikator, string);
            if ((EYZ_Mod == 3) || (EYZ_Mod == 4))
              formul_kontrol(event, string, EYZ_Mod, true);
            else
              udp_client(event, EYZ_Indikator, string, EYZ_Mod);
          }
          string = "";
        }
        success = true;
        break;
      }
#ifdef ESP32
#if FEATURE_ETHERNET
    case PLUGIN_SERIAL_IN:
      {
        while (Serial1.available()) {
          char inChar = Serial1.read();
          if (inChar == 255) {
            Serial1.flush();
            break;
          }
          if (EYZ_ASCII) {
            if (isprint(inChar))
              tartimString_s += (String)inChar;
          } else
            tartimString_s += (String)inChar;
          if ((inChar == ExtraTaskSettings.TaskDeviceSonByte) && (tartimString_s.length() > 1)) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, EYZ_Indikator, tartimString_s);
            if ((EYZ_Mod == 3) || (EYZ_Mod == 4))
              formul_kontrol(event, tartimString_s, EYZ_Mod, true);
            else {
              if (EYZ_Mod == 5) {
                if ((String(EYZ_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");
                if ((String(EYZ_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
                if ((String(EYZ_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
              }
              formul_seri(event, tartimString_s, EYZ_Indikator);
              if ((EYZ_Mod == 1) && ((webapinettartim > 0.001) || (Settings.UseNegatifYaz)))
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
            }
            tartimString_s = "";
          }
        }
        success = true;
        break;
      }
#endif
#if defined(HAS_WIFI) || defined(HAS_BLE)
    case PLUGIN_SERIAL_IN:
      {
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255) {
            Serial.flush();
            break;
          }
          if (EYZ_ASCII) {
            if (isprint(inChar))
              tartimString_s += (String)inChar;
          } else
            tartimString_s += (String)inChar;
          if ((inChar == ExtraTaskSettings.TaskDeviceSonByte) && (tartimString_s.length() > 1)) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, EYZ_Indikator, tartimString_s);
            if ((EYZ_Mod == 3) || (EYZ_Mod == 4))
              formul_kontrol(event, tartimString_s, EYZ_Mod, true);
            else {
              if (EYZ_Mod == 5) {
                if ((String(EYZ_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");
                if ((String(EYZ_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
                if ((String(EYZ_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
              }
              formul_seri(event, tartimString_s, EYZ_Indikator);
              if ((EYZ_Mod == 1) && ((webapinettartim > 0.001) || (Settings.UseNegatifYaz)))
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
            }
            #ifdef ESP_NOW_ACTIVE
              EspnowSendData(String(XML_NET_C));
            #endif
            tartimString_s = "";
          }
        }
        success = true;
        break;
      }
#endif
#endif
#ifdef ESP8266
    case PLUGIN_SERIAL_IN:
      {
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255) {
            Serial.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, EYZ_Indikator, tartimString_s);
            if ((EYZ_Mod == 3) || (EYZ_Mod == 4))
              formul_kontrol(event, tartimString_s, EYZ_Mod, true);
            else {
              if (EYZ_Mod == 5) {
                if ((String(EYZ_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");
                if ((String(EYZ_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
                if ((String(EYZ_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
              }
              formul_seri(event, tartimString_s, EYZ_Indikator);
              if ((EYZ_Mod == 1) && ((webapinettartim > 0.001) || (Settings.UseNegatifYaz)))
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
            }
            tartimString_s = "";
          }
        }
        success = true;
        break;
      }
#endif

/*#ifdef ESP32
#ifdef HAS_BLUETOOTH
    case PLUGIN_SERIALBT_IN:
      {
        while (SerialBT.available()) {
          char inChar = SerialBT.read();
          if (inChar == 255) {
            SerialBT.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          Serial1.print(tartimString_s);
          if ((inChar == ExtraTaskSettings.TaskDeviceSonByte) && (tartimString_s.length() > 1)) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, EYZ_Indikator, tartimString_s);
            if ((EYZ_Mod == 3) || (EYZ_Mod == 4))
              formul_kontrol(event, tartimString_s, EYZ_Mod, true);
            else {
              if (EYZ_Mod == 5) {
                if ((String(EYZ_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");
                if ((String(EYZ_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
                if ((String(EYZ_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZ_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
              }
              formul_seri(event, tartimString_s, EYZ_Indikator);
              if ((EYZ_Mod == 1) && ((webapinettartim > 0.001) || (Settings.UseNegatifYaz)))
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
            }
            tartimString_s = "";
          }
        }
        success = true;
        break;
      }
      
      
      {
        while (SerialBT.available()) {
          char inChar = Serial.read();
          if (inChar == 255) {
            Serial.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, EYZ_Indikator, tartimString_s);
            if ((EYZ_Mod == 3) || (EYZ_Mod == 4))
              formul_kontrol(event, tartimString_s, EYZ_Mod, true);
            else {
              if (EYZ_Mod == 5) {
                if ((String(eyzartKomut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(eyzartKomut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(eyzartKomut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzart");
                if ((String(eyztopKomut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(eyztopKomut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(eyztopKomut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
                if ((String(eyztekKomut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(eyztekKomut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(eyztekKomut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
              }
              formul_seri(event, tartimString_s, EYZ_Indikator);
              if ((EYZ_Mod == 1) && (webapinettartim > 0.001) || (Settings.UseNegatifYaz))
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
              Serial.flush();
            }
            tartimString_s = "";
          }
        }
        success = true;
        break;
      }
#endif
#endif*/
  }
  return success;
}
#endif

//Dikomsan
//ETXSTXadd:       01   CRLF
//n/w:      0.09 gCRLF
//u/w:       0    CRLF
//pcs:       0   CRLF