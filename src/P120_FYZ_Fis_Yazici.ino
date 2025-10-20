#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P120

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
//#######################################################################################################
//##################################### Plugin 120: FYZ  ################################################
//#######################################################################################################

#define PLUGIN_120
#define PLUGIN_ID_120 120
#define PLUGIN_NAME_120 "Printer - FYZ"
#define PLUGIN_VALUENAME1_120 "NET"
#define PLUGIN_VALUENAME2_120 "DARA"
#define PLUGIN_VALUENAME3_120 "BRUT"
#define PLUGIN_VALUENAME4_120 "ADET"
#define PLUGIN_VALUENAME5_120 "ADETGR"
#define PLUGIN_VALUENAME6_120 "PLUNO"
#define PLUGIN_VALUENAME7_120 "B.FIYAT"
#define PLUGIN_VALUENAME8_120 "TUTAR"
#define PLUGIN_VALUENAME9_120 "NET_2"
#define PLUGIN_VALUENAME10_120 "DARA_2"
#define MAX_SRV_CLIENTS 5

#define CUSTOMTASK2_STR_SIZE_P120 46

#define HEDEF_ADDR_SIZE_P120 8
#define FIS_BASLIK1_ADDR_SIZE_P120 44
#define FIS_BASLIK2_ADDR_SIZE_P120 44
#define FIS_BASLIK3_ADDR_SIZE_P120 44
#define FIS_BASLIK4_ADDR_SIZE_P120 44


#define MES_BUFF_SIZE_P120 19
#define HEDEF_BUFF_SIZE_P120 9
#define FIS_BASLIK1_BUFF_SIZE_P120 45
#define FIS_BASLIK2_BUFF_SIZE_P120 45
#define FIS_BASLIK3_BUFF_SIZE_P120 45
#define FIS_BASLIK4_BUFF_SIZE_P120 45

#define FIS_BASLIK1_DEF_P120 "fisbasligi 1"
#define FIS_BASLIK2_DEF_P120 "fisbasligi 2"
#define FIS_BASLIK3_DEF_P120 "fisbasligi 3"
#define FIS_BASLIK4_DEF_P120 "fisbasligi 4"

#define FYZ_Model ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define FYZ_Indikator ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define FYZ_Mod ExtraTaskSettings.TaskDevicePluginConfigLong[2]
#define FYZ_Buton1 ExtraTaskSettings.TaskDevicePluginConfigLong[3]
#define FYZ_Buton2 ExtraTaskSettings.TaskDevicePluginConfigLong[4]
#define FYZ_Gecikme ExtraTaskSettings.TaskDevicePluginConfigLong[5]
#define FYZ_Sayac ExtraTaskSettings.TaskDevicePluginConfigLong[6]

#define FYZ_Kopya ExtraTaskSettings.TaskDevicePluginConfig[0]
#define FYZ_Logo ExtraTaskSettings.TaskDevicePluginConfig[1]

#define FYZ_art_komut ExtraTaskSettings.TaskDeviceMesage[0]
#define FYZ_tek_komut ExtraTaskSettings.TaskDeviceMesage[1]
#define FYZ_top_komut ExtraTaskSettings.TaskDeviceMesage[2]

#ifdef CAS_VERSION
#define FYZ_fisbaslik1 ExtraTaskSettings.TaskDeviceBaslik[0]
#define FYZ_fisbaslik2 ExtraTaskSettings.TaskDeviceBaslik[1]
#define FYZ_fisbaslik3 ExtraTaskSettings.TaskDeviceBaslik[2]
#define FYZ_fisbaslik4 ExtraTaskSettings.TaskDeviceBaslik[3]
#endif

#define FYZ_Hedef PCONFIG_FLOAT(0)

int fyz_stabil_sayisi = 0;

//WiFiServer *FyzServer;
//WiFiClient FyzClients[MAX_SRV_CLIENTS];
/*#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

//const uint16_t kRecvPin = 13;
const uint16_t kRecvPin = 14;

IRrecv irrecv(kRecvPin);

decode_results results;
*/

#include "OneButton.h"

#ifdef ESP8266
OneButton FYZ_button1(12, true, true);
OneButton FYZ_button2(14, true, true);
#endif

#ifdef ESP32
#if FEATURE_ETHERNET
OneButton FYZ_button1(14, true, true);
OneButton FYZ_button2(15, true, true);
#endif
#ifdef HAS_WIFI
OneButton FYZ_button1(21, true, true);
OneButton FYZ_button2(22, true, true);
#endif
#endif

float sum_f_d[128];
float sum_f = 0;
uint8_t i = 0;

/*uint8_t broadcastAddress_rcv_120[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
esp_now_peer_info_t peerInfo_rcv_120;*/

void FYZ_click1() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
}
void FYZ_longPressStart1() {
  Settings.UseSerial = true;
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "settings");
}
void FYZ_longPressStop1() {
  Settings.UseSerial = false;
}
void FYZ_click2() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
  if (FYZ_Kopya)
    ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzkop");
}
void FYZ_longPressStart2() {
  //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzkop");
}
void FYZ_longPressStop2() {
  Settings.UseSerial = false;
}

/*void onReceiveData_120(const uint8_t *mac, const uint8_t *data, int len) {
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
void initESP_NOW_SRV_120() {
  if (esp_now_init() != ESP_OK) {
    //Serial.printf("Error initializing ESP-NOW\n");
    return;
  }
  memcpy(peerInfo_rcv_120.peer_addr, broadcastAddress_rcv_120, sizeof(broadcastAddress_rcv_120));
  peerInfo_rcv_120.channel = 1;  
  peerInfo_rcv_120.encrypt = false;
  if (esp_now_add_peer(&peerInfo_rcv_120) != ESP_OK){
    //Serial.printf("Failed to add peer\r\n");
    return;
  }
}*/

boolean Plugin_120(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  switch (function) {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_120;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        //Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 10;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }
    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_120);
        break;
      }
    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_120));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_120));
        break;
      }
    case PLUGIN_WEBFORM_LOAD:
      {
#ifdef CAS_VERSION
        addFormSubHeader(F("Yazıcı Ayarları"));
#else
        addFormSubHeader(F("FYZ Ayarları"));
#endif
        byte choice0 = FYZ_Model;
        String options0[4];
        options0[0] = F("FYZ58H");
        options0[1] = F("FYZ80");
        options0[2] = F("FYZ58Mobil");
        options0[3] = F("FYZ80Mobil");
        int optionValues0[4] = {0, 1, 2, 3};
        addFormSelector(F("Model"), F("plugin_120_model"), 4, options0, optionValues0, choice0);
        addFormCheckBox(F("Kopya Aktif"), F("plugin_120_kopya"), FYZ_Kopya);
        byte choice1 = FYZ_Mod;
        String options1[7];
        options1[0] = F("SÜREKLi VERi (TEK SATIRLI VERi)");
        options1[1] = F("TERAZiDEN OTOMATiK (TEK SATIRLI VERi)");
        options1[2] = F("DENGELi OTOMATiK (TEK SATIRLI VERi)");
        options1[3] = F("TERAZiDEN TUŞ iLE (ÇOK SATIRLI VERi)");
        options1[4] = F("YAZICIDAN TUŞ iLE KONTROL(ÇOK SATIRLI VERi)");
        options1[5] = F("KUMANDA");
        options1[6] = F("VERi PAKETi");
        int optionValues1[7] = {0, 1, 2, 3, 4, 5, 6};
        addFormSelector(F("Mod"), F("plugin_120_mod"), 7, options1, optionValues1, choice1);
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
            if (fname.startsWith(F("/fyz")) || fname.startsWith(F("fyz"))) {
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
        addFormSelector(F("Tek Fiş"), F("plugin_120_tek_prn"), 10, options2, optionValues2, choice2);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Fiş Dizayn Menüsüne Git"));
        byte choice3 = ExtraTaskSettings.TaskDevicePrint[1];
        addFormSelector(F("Artı Fiş"), F("plugin_120_art_prn"), 10, options2, optionValues2, choice3);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Fiş Dizayn Menüsüne Git"));
        byte choice5 = ExtraTaskSettings.TaskDevicePrint[3];
        addFormSelector(F("Ek Fiş"), F("plugin_120_ek_prn"), 10, options2, optionValues2, choice5);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[3]], F("Fiş Dizayn Menüsüne Git"));
        byte choice4 = ExtraTaskSettings.TaskDevicePrint[2];
        addFormSelector(F("Toplam Fiş"), F("plugin_120_top_prn"), 10, options2, optionValues2, choice4);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[2]], F("Fiş Dizayn Menüsüne Git"));
        if (FYZ_Mod == 2) {
          addFormTextBox(F("Hedef Kilogram"), F("plugin_120_hedef"), String(FYZ_Hedef), HEDEF_BUFF_SIZE_P120);
          addFormNumericBox(F("Gecikme Saniye"), F("plugin_120_gecikme"), FYZ_Gecikme, 0, 999);
          addFormNumericBox(F("Veri Sayaç"), F("plugin_120_sayac"), FYZ_Sayac, 2, 129);
        } else if (FYZ_Mod == 5) {
          addFormTextBox(F("Artı Komutu"), getPluginCustomArgName(0), FYZ_art_komut, MES_BUFF_SIZE_P120);
          addFormTextBox(F("Toplam Komutu"), getPluginCustomArgName(1), FYZ_top_komut, MES_BUFF_SIZE_P120);
          addFormTextBox(F("Tek Komutu"), getPluginCustomArgName(2), FYZ_tek_komut, MES_BUFF_SIZE_P120);
        }
#ifdef CAS_VERSION
        addFormTextBox(F("Fiş Başlığı 1"), getPluginCustomArgName(5), FYZ_fisbaslik1, FIS_BASLIK1_ADDR_SIZE_P120);
        addFormTextBox(F("Fiş Başlığı 2"), getPluginCustomArgName(6), FYZ_fisbaslik2, FIS_BASLIK2_ADDR_SIZE_P120);
        addFormTextBox(F("Fiş Başlığı 3"), getPluginCustomArgName(7), FYZ_fisbaslik3, FIS_BASLIK3_ADDR_SIZE_P120);
        addFormTextBox(F("Fiş Başlığı 4"), getPluginCustomArgName(8), FYZ_fisbaslik4, FIS_BASLIK4_ADDR_SIZE_P120);
#endif
        addFormSubHeader(F("İndikatör Ayarları"));
        indikator_secimi(event, FYZ_Indikator, F("plugin_120_indikator"));
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));
        success = true;
        break;
      }
    case PLUGIN_WEBFORM_SAVE:
      {
        FYZ_Model = getFormItemInt(F("plugin_120_model"));
        FYZ_Indikator = getFormItemInt(F("plugin_120_indikator"));
        FYZ_Kopya = isFormItemChecked(F("plugin_120_kopya"));
        FYZ_Mod = getFormItemInt(F("plugin_120_mod"));
        FYZ_Hedef = getFormItemFloat(F("plugin_120_hedef"));
        FYZ_Gecikme = getFormItemInt(F("plugin_120_gecikme"));
        FYZ_Sayac = getFormItemInt(F("plugin_120_sayac"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));
        if (FYZ_Mod == 5) {
          strncpy_webserver_arg(FYZ_art_komut, getPluginCustomArgName(0));
          strncpy_webserver_arg(FYZ_top_komut, getPluginCustomArgName(1));
          strncpy_webserver_arg(FYZ_tek_komut, getPluginCustomArgName(2));
        }
#ifdef CAS_VERSION
        strncpy_webserver_arg(FYZ_fisbaslik1, getPluginCustomArgName(5));
        strncpy_webserver_arg(FYZ_fisbaslik2, getPluginCustomArgName(6));
        strncpy_webserver_arg(FYZ_fisbaslik3, getPluginCustomArgName(7));
        strncpy_webserver_arg(FYZ_fisbaslik4, getPluginCustomArgName(8));
#endif
        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, FYZ_Indikator, PCONFIG(4));
        ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_120_tek_prn"));
        ExtraTaskSettings.TaskDevicePrint[1] = getFormItemInt(F("plugin_120_art_prn"));
        ExtraTaskSettings.TaskDevicePrint[3] = getFormItemInt(F("plugin_120_ek_prn"));
        ExtraTaskSettings.TaskDevicePrint[2] = getFormItemInt(F("plugin_120_top_prn"));
        options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.tek_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[1]].toCharArray(Settings.art_prn, options2[ExtraTaskSettings.TaskDevicePrint[1]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[3]].toCharArray(Settings.ek_prn, options2[ExtraTaskSettings.TaskDevicePrint[3]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[2]].toCharArray(Settings.top_prn, options2[ExtraTaskSettings.TaskDevicePrint[2]].length() + 1);
        sum_f = 0;
        i = 0;
        success = true;
        break;
      }
    case PLUGIN_INIT:
      {
        /*if (Settings.espnow_mod == 2)
          initESP_NOW_SRV_120();*/
//irrecv.enableIRIn();
//FyzServer = new WiFiServer(9100);
//FyzServer->begin();
#ifdef CAS_VERSION
        XML_FIS_BASLIK1_S = FYZ_fisbaslik1;
        XML_FIS_BASLIK2_S = FYZ_fisbaslik2;
        XML_FIS_BASLIK3_S = FYZ_fisbaslik3;
        XML_FIS_BASLIK4_S = FYZ_fisbaslik4;
#endif
        FYZ_button1.attachClick(FYZ_click1);
        FYZ_button1.attachLongPressStart(FYZ_longPressStart1);
        FYZ_button1.attachLongPressStop(FYZ_longPressStop1);
        FYZ_button2.attachClick(FYZ_click2);
        FYZ_button2.attachLongPressStart(FYZ_longPressStart2);
        FYZ_button2.attachLongPressStop(FYZ_longPressStop2);
        FYZ_button1.tick();
        FYZ_button2.tick();
        /*#ifdef ESP8266
        pinMode(12, INPUT_PULLUP);
        pinMode(14, INPUT_PULLUP);
#endif
        #ifdef ESP32
        //pinMode(12, INPUT);
        //pinMode(27, INPUT);
#endif*/
        Settings.WebAPP = 120;
        success = true;
        break;
      }
      /*case PLUGIN_TEN_PER_SECOND:
      {
        if (irrecv.decode(&results)) {
          //serialPrintUint64(results.value, HEX);
          IRDA_DATA_S = String(int(results.value));
          //Serial.println(IRDA_DATA_S);
          irrecv.resume();  // Receive the next value
        }
        delay(100);
        success = true;
        break;
      }*/

      /*case PLUGIN_TEN_PER_SECOND:
      {
        uint8_t i;
        if (WiFiConnected()) {
          if (FyzServer->hasClient())
          {
            for (i = 0; i < MAX_SRV_CLIENTS; i++) {
              if (!FyzClients[i]) {
                FyzClients[i] = FyzServer->available();
                continue;
              }
            }
          }
          for (int i = 0; i < MAX_SRV_CLIENTS; i++) {
            while (FyzClients[i].available() && Serial.availableForWrite() > 0) {
              Serial.write(FyzClients[i].read());
            }
          }
        }
        success = true;
        break;
      }*/
#ifdef CAS_VERSION
    case PLUGIN_ONE_HUNDRED_PER_SECOND:
      {
        if (FYZ_Mod == 2) {
          if (webapinettartim_son == webapinettartim) {
            fyz_stabil_sayisi++;
          } else
            fyz_stabil_sayisi = 0;
          if (fyz_stabil_sayisi >= (FYZ_Gecikme * 10)) {
            XML_STABIL_S = "ST";
            if ((webapinettartim > FYZ_Hedef) && (hayvan_modu == 0)) {
              ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
              hayvan_modu = 2;
            }
          } else {
            XML_STABIL_S = "US";
            if ((webapinettartim < FYZ_Hedef) && (hayvan_modu == 2))
              hayvan_modu = 0;
          }
        }
        success = true;
        break;
      }
#endif
    case PLUGIN_FIFTY_PER_SECOND:
      {
        /*if (Settings.espnow_mod == 2)
          esp_now_register_recv_cb(onReceiveData_120);*/
        switch (FYZ_Mod) {
          case 0:
            FYZ_button1.tick();
            FYZ_button2.tick();
            break;
          case 1:
            FYZ_button1.tick();
            FYZ_button2.tick();
            break;
          case 2:
            FYZ_button2.tick();
            dtostrf(webapinettartim, 8, ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_webapinettartim_C);
            dtostrf(webapinettartim_son, 8, ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_webapinettartim_son_C);
            if (String(webapinettartim_son) == String(webapinettartim))
              fyz_stabil_sayisi++;
            else
              fyz_stabil_sayisi = 0;
            if (fyz_stabil_sayisi >= FYZ_Gecikme) {
              XML_STABIL_S = "ST";
              if ((webapinettartim_son > FYZ_Hedef) && (hayvan_modu == 0)) {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                hayvan_modu = 2;
              }
            } else {
              XML_STABIL_S = "US";
              if ((webapinettartim_son < FYZ_Hedef) && (hayvan_modu == 2))
                hayvan_modu = 0;
            }
            break;
          case 4:
            FYZ_button1.tick();
            FYZ_button2.tick();
            break;
          case 5:
            FYZ_button1.tick();
            FYZ_button2.tick();
            break;
        }
        serial_error(event, FYZ_Mod, "fyztop");
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (Settings.UDPPort > 0) {
          string.replace("\n", "");
          string.replace("\r", "");
          string.replace(String(char(ExtraTaskSettings.TaskDeviceSonByte)), "");
          hataTimer_l = millis();
          if ((String(FYZ_art_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0]+2, (String(FYZ_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0]+2)) == String(FYZ_art_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
          else if ((String(FYZ_top_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0]+2, (String(FYZ_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0]+2)) == String(FYZ_top_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
          else if ((String(FYZ_tek_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0]+2, (String(FYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0]+2)) == String(FYZ_tek_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
          else
            udp_client(event, FYZ_Indikator, string, FYZ_Mod);
          //Serial.println(string);
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
          if (inChar == 255)  // binary data...
          {
            Serial1.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, FYZ_Indikator, tartimString_s);
            if ((FYZ_Mod == 3) || (FYZ_Mod == 4))
              formul_kontrol(event, tartimString_s, FYZ_Mod, true);
            else if (FYZ_Mod == 6) {
              paketVeri_s += tartimString_s;
              oto_yazdir = true;
            } else {
              if (FYZ_Mod == 5) {
                if ((String(FYZ_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZ_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                if ((String(FYZ_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZ_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                if ((String(FYZ_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZ_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
              }
              formul_seri(event, tartimString_s, FYZ_Indikator);
              if ((FYZ_Mod == 1) && (webapinettartim > 0.0001)) {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                String komut = "SendTo#1#fyzart#";
                komut = XML_NET_S;
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, komut.c_str());
              }
              if (FYZ_Mod == 2) {
                if (i >= FYZ_Sayac) {
                  i--;
                  sum_f = sum_f - sum_f_d[0];
                  for (uint8_t i = 0; i < FYZ_Sayac; i++)
                    sum_f_d[i] = sum_f_d[i + 1];
                } else {
                  sum_f_d[i] = webapinettartim;
                  sum_f = sum_f + sum_f_d[i];
                  webapinettartim_son = sum_f / (i + 1);
                  i++;
                }
              }
            }
            tartimString_s = "";
            Serial1.flush();
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
          if (inChar == 255)  // binary data...
          {
            Serial.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          if ((inChar == ExtraTaskSettings.TaskDeviceSonByte) && (tartimString_s.length() > 1)) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, FYZ_Indikator, tartimString_s);
            if ((FYZ_Mod == 3) || (FYZ_Mod == 4))
              formul_kontrol(event, tartimString_s, FYZ_Mod, true);
            else if (FYZ_Mod == 6) {
              paketVeri_s += tartimString_s;
              oto_yazdir = true;
            } else {
              if (FYZ_Mod == 5) {
                //Serial.println(tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])));
                addLog(LOG_LEVEL_INFO, tartimString_s);
                if ((String(FYZ_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZ_art_komut))) {
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                  tartimString_s = "";
                  break;
                }
                if ((String(FYZ_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZ_top_komut))) {
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                  tartimString_s = "";
                  break;
                }
                if ((String(FYZ_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZ_tek_komut))) {
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                  tartimString_s = "";
                  break;
                }
              }
              formul_seri(event, tartimString_s, FYZ_Indikator);
              if ((FYZ_Mod == 1) && (webapinettartim > 0.0001)) {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                String komut = "SendTo#1#fyzart#";
                komut = XML_NET_S;
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, komut.c_str());
              }
              if (FYZ_Mod == 2) {
                if (i >= FYZ_Sayac) {
                  i--;
                  sum_f = sum_f - sum_f_d[0];
                  for (uint8_t i = 0; i < FYZ_Sayac; i++)
                    sum_f_d[i] = sum_f_d[i + 1];
                } else {
                  sum_f_d[i] = webapinettartim;
                  sum_f = sum_f + sum_f_d[i];
                  webapinettartim_son = sum_f / (i + 1);
                  i++;
                }
              }
            }
            #ifdef ESP_NOW_ACTIVE
              EspnowSendData(String(XML_NET_C));
            #endif
            tartimString_s = "";
            Serial.flush();
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
          if (inChar == 255)  // binary data...
          {
            Serial.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          if (inChar == ExtraTaskSettings.TaskDeviceSonByte) {
            hataTimer_l = millis();
            if (Settings.Tersle)
              tersle(event, tartimString_s);
            isaret(event, FYZ_Indikator, tartimString_s);
            if ((FYZ_Mod == 3) || (FYZ_Mod == 4))
              formul_kontrol(event, tartimString_s, FYZ_Mod, true);
            else if (FYZ_Mod == 6) {
              paketVeri_s += tartimString_s;
              oto_yazdir = true;
            } else {
              if (FYZ_Mod == 5) {
                if ((String(FYZ_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZ_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                if ((String(FYZ_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZ_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
                if ((String(FYZ_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(FYZ_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(FYZ_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
              }
              /*formul_seri(event, tartimString_s, FYZ_Indikator);
              if ((FYZ_Mod == 1) && (webapinettartim > 0.0001)) {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyztop");
              }*/
              formul_seri(event, tartimString_s, FYZ_Indikator);
              if ((FYZ_Mod == 1) && (webapinettartim > 0.0001)) {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "fyzart");
                String komut = "SendTo#1#fyzart#";
                komut = XML_NET_S;
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, komut.c_str());
              }
              if (FYZ_Mod == 2) {
                if (i >= FYZ_Sayac) {
                  i--;
                  sum_f = sum_f - sum_f_d[0];
                  for (uint8_t i = 0; i < FYZ_Sayac; i++)
                    sum_f_d[i] = sum_f_d[i + 1];
                } else {
                  sum_f_d[i] = webapinettartim;
                  sum_f = sum_f + sum_f_d[i];
                  webapinettartim_son = sum_f / (i + 1);
                  i++;
                }
              }
            }
            tartimString_s = "";
            Serial.flush();
          }
        }
        success = true;
        break;
      }
#endif
  }
  return success;
}
#endif

//ocs-sx 58880
//invld noacc nodel
//Dikomsan
//ETXSTXadd:       01   CRLF
//n/w:      0.09 gCRLF
//u/w:       0    CRLF
//pcs:       0   CRLF

//ETXSTXadd:       01   CRLF
//n/w:      0.09 gCRLF
//u/w:       0    CRLF
//pcs:       0    CRLF
//hardware   D3 output low