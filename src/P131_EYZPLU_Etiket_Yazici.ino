#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
//#include "ESP32Ping.h"

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
#define i2c_Address 0x3c  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P131

#include "src/Commands/InternalCommands.h"
#include "src/ESPEasyCore/ESPEasyNetwork.h"
#include "src/ESPEasyCore/Controller.h"

#include "src/CustomBuild/CompiletimeDefines.h"

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
#include "src/Globals/SecuritySettings.h"

#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/Memory.h"
#include "src/Helpers/StringConverter.h"
#include "src/Helpers/StringParser.h"
#include "src/Helpers/Networking.h"
#include "src/Helpers/StringGenerator_System.h"

#include "ESPEasy_common.h"
#include "ESPEasy-Globals.h"

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16
static const unsigned char PROGMEM logo_bmp[] = {
  B00000000, B00001110,
  B00000000, B00001110,
  B00000000, B11101110,
  B00000000, B11101110,
  B00001110, B11101110,
  B00001110, B11101110,
  B11101110, B11101110,
  B11101110, B11101110
};

static const unsigned char PROGMEM logo_bmp1[] = {
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B11100000,
  B00000000, B11100000,
  B00001110, B11100000,
  B00001110, B11100000,
  B11101110, B11100000,
  B11101110, B11100000
};

static const unsigned char PROGMEM logo_bmp2[] = {
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00001110, B00000000,
  B00001110, B00000000,
  B11101110, B00000000,
  B11101110, B00000000
};

static const unsigned char PROGMEM logo_bmp3[] = {
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B11100000, B00000000,
  B11100000, B00000000
};

static const unsigned char PROGMEM warning_icon16x16[] =
{
	B00000000, B10000000, //         #       
	B00000001, B11000000, //        ###      
	B00000001, B11000000, //        ###      
	B00000011, B11100000, //       #####     
	B00000011, B01100000, //       ## ##     
	B00000111, B01110000, //      ### ###    
	B00000110, B00110000, //      ##   ##    
	B00001110, B10111000, //     ### # ###   
	B00001100, B10011000, //     ##  #  ##   
	B00011100, B10011100, //    ###  #  ###  
	B00011000, B10001100, //    ##   #   ##  
	B00111000, B00001110, //   ###       ### 
	B00110000, B10000110, //   ##    #    ## 
	B01111111, B11111111, //  ###############
	B01111111, B11111111, //  ###############
	B00000000, B00000000  //                 
};

static const unsigned char PROGMEM okey_icon16x16[] =
{
	B00000000, B00000000, //  
	B00000000, B00000000, //       
	B00000000, B00000111, //      
	B00000000, B00001111, //      
	B00000000, B00011110, //                 ##      
	B11000000, B00111100, //  #            ####
	B01100000, B01111000, //  ##         ####
	B01110000, B11110000, //  ###      ####
	B00111001, B11100000, //   ###   ####
	B00111111, B11000000, //   #########
	B00011111, B10000000, //   #####
	B00011111, B00000000, // 
	B00000000, B00000000, //  
	B00000000, B00000000, // 
	B00000000, B00000000, // 
	B00000000, B00000000  //                 
};

const unsigned char FIS_YAZICI[] = {
  0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x10, 0x04,
  0xd0, 0x05, 0x10, 0x04, 0xd0, 0x05, 0x10, 0x04,
  0xfc, 0x1f, 0x04, 0x10, 0xe4, 0x11, 0x24, 0x10,
  0xe4, 0x10, 0x24, 0x10, 0x24, 0x10, 0x04, 0x10,
  0xf8, 0x0f, 0x00, 0x00, 0x00, 0x00
};

const unsigned char ETIKET_YAZICI[] = {
  0x00, 0x00, 0xf0, 0x07, 0x08, 0x08, 0xe8, 0x0b,
  0x08, 0x08, 0xe8, 0x0b, 0x08, 0x08, 0xf0, 0x07,
  0x00, 0x00, 0xfc, 0x1f, 0x04, 0x10, 0xe4, 0x11,
  0x24, 0x10, 0xe4, 0x10, 0x24, 0x10, 0xe4, 0x11,
  0x04, 0x10, 0xf8, 0x0f, 0x00, 0x00
};

//#######################################################################################################
//##################################### Plugin 131: EYZPLU ##############################################
//#######################################################################################################

#define PLUGIN_131
#define PLUGIN_ID_131 131
#define PLUGIN_NAME_131 "Printer - EYZPLU"
#define PLUGIN_VALUENAME1_131 "NET"
#define PLUGIN_VALUENAME2_131 "DARA"
#define PLUGIN_VALUENAME3_131 "BRUT"
#define PLUGIN_VALUENAME4_131 "ADET"
#define PLUGIN_VALUENAME5_131 "ADETGR"
#define PLUGIN_VALUENAME6_131 "PLUNO"
#define PLUGIN_VALUENAME7_131 "B.FIYAT"
#define PLUGIN_VALUENAME8_131 "TUTAR"
#define PLUGIN_VALUENAME9_131 "NET_2"
#define PLUGIN_VALUENAME10_131 "DARA_2"
#define MAX_SRV_CLIENTS 5

#define CUSTOMTASK_STR_SIZE_P131 20

#define HEDEF_ADDR_SIZE_P131 8

#define MES_BUFF_SIZE_P131 19
#define HEDEF_BUFF_SIZE_P131 9

#define EYZPLU_Model     ExtraTaskSettings.TaskDevicePluginConfigLong[0]
#define EYZPLU_Indikator ExtraTaskSettings.TaskDevicePluginConfigLong[1]
#define EYZPLU_Mod       ExtraTaskSettings.TaskDevicePluginConfigLong[2]
#define EYZPLU_Gecikme   ExtraTaskSettings.TaskDevicePluginConfigLong[3]

#define EYZPLU_Bartender ExtraTaskSettings.TaskPrintBartender

#define EYZPLU_art_komut ExtraTaskSettings.TaskDeviceMesage[0]
#define EYZPLU_tek_komut ExtraTaskSettings.TaskDeviceMesage[1]
#define EYZPLU_top_komut ExtraTaskSettings.TaskDeviceMesage[2]

#define EYZPLU_Hedef PCONFIG_FLOAT(0)

bool internet = false;

#include "OneButton.h"

#ifdef ESP8266
OneButton EYZPLU_button1(12, false, false);
#endif

#ifdef ESP32
#ifdef ESP32_CLASSIC
#if FEATURE_ETHERNET
//OneButton EYZPLU_button1(15, true, true);  //
//OneButton EYZPLU_button1(14, false, false);  //RONGTA
OneButton EYZPLU_button1(15, false, false);  //RONGTA
//OneButton eyz_button2(22, true, true);  //HPRT
#endif
#ifdef HAS_WIFI
OneButton EYZPLU_button1(21, true, true);  //RONGTA
OneButton EYZPLU_button2(22, true, true);  //RONGTA
OneButton EYZPLU_button3(13, true, true);  //RONGTA
OneButton EYZPLU_button4(14, true, true);  //RONGTA
OneButton EYZPLU_button5(27, true, true);  //RONGTA
#endif
#endif
#endif

/*OneButton EYZPLU_button1(12, true, true);
OneButton EYZPLU_button2(14, true, true);
OneButton EYZPLU_button3(13, true, true);*/

#ifdef ESP32_CLASSIC
void EYZPLU_click1() {
  display.clearDisplay();
  display.setTextSize(1,2);
  display.println("KAYIT BASLADI.");
  /*ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart#1#1#0");
  digitalWrite(5, HIGH);
  delay(2000);
  digitalWrite(5, LOW);*/
}
void EYZPLU_longPressStart1() {
  /*Serial1.println("SIZE 55 mm, 40 mm");
  Serial1.println("DIRECTION 0,0");
  Serial1.println("REFERENCE 0,0");
  Serial1.println("OFFSET 0 mm");
  Serial1.println("SET PEEL OFF");
  Serial1.println("SET CUTTER OFF");
  Serial1.println("SET TEAR ON");
  Serial1.println("CLS");
  Serial1.println("CODEPAGE 857");
  Serial1.println("TEXT 400,240,\"2\",180,1,2,\"VERSIYON 1.1\"");
  Serial1.print("TEXT 400,180,\"2\",180,1,1,\"");
  Serial1.print(F("IP Address : ")); Serial1.print(formatIP(NetworkLocalIP()));
  Serial1.println("\"");
  Serial1.print("TEXT 400,140,\"2\",180,1,1,\"");
  Serial1.print(F("  Build         : ")); Serial1.print(String(get_build_nr()) + '/' + getSystemBuildString());
  Serial1.println("\"");
  Serial1.print("TEXT 400,100,\"2\",180,1,1,\"");
  Serial1.print(F("WifiSSID : ")); Serial1.print(SecuritySettings.WifiSSID);
  Serial1.println("\"");  
  Serial1.println("PRINT 1,1");*/
}
void EYZPLU_click2() {
  /*ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart#2#1#0");
  digitalWrite(5, HIGH);
  delay(2000);
  digitalWrite(5, LOW);*/
}
void EYZPLU_click3() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart#3#1#0");
  digitalWrite(5, HIGH);
  delay(2000);
  digitalWrite(5, LOW);
}
void EYZPLU_click4() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart#4#1#0");
  digitalWrite(5, HIGH);
  delay(2000);
  digitalWrite(5, LOW);
}
void EYZPLU_click5() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart#0#1#0");
  digitalWrite(5, HIGH);
  delay(2000);
  digitalWrite(5, LOW);
}
#endif

boolean Plugin_131(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_131;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 10;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_131);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_131));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_131));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_131));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_131));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[4], PSTR(PLUGIN_VALUENAME5_131));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[5], PSTR(PLUGIN_VALUENAME6_131));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[6], PSTR(PLUGIN_VALUENAME7_131));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[7], PSTR(PLUGIN_VALUENAME8_131));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[8], PSTR(PLUGIN_VALUENAME9_131));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[9], PSTR(PLUGIN_VALUENAME10_131));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
#ifdef CAS_VERSION
        addFormSubHeader(F("Yazıcı Ayarları"));
#else
        addFormSubHeader(F("EYZPLU Ayarları"));
#endif
        byte choice0 = EYZPLU_Model;
        String options0[5];
        options0[0] = F("EYZ72R");
        options0[1] = F("EYZ100");
        options0[2] = F("EYZ100R");
        options0[3] = F("EYZ72Mobil");
        options0[4] = F("EYZ100Mobil");
        int optionValues0[5] = {0, 1, 2, 3, 4};
        addFormSelector(F("Yazıcı Model"), F("plugin_131_model"), 5, options0, optionValues0, choice0);

        byte choice1 = EYZPLU_Mod;
        String options1[7];
        options1[0] = F("SÜREKLi VERi (TEK SATIRLI VERi)");
        options1[1] = F("TERAZiDEN OTOMATiK (TEK SATIRLI VERi)");
        options1[2] = F("DENGELi OTOMATiK (TEK SATIRLI VERi)");
        options1[3] = F("TERAZiDEN TUŞ iLE (ÇOK SATIRLI VERi)");
        options1[4] = F("YAZICIDAN TUŞ iLE KONTROL(ÇOK SATIRLI VERi)");
        options1[5] = F("KUMANDA");
        options1[6] = F("VERi PAKETi");
        int optionValues1[7] = {0, 1, 2, 3, 4, 5, 6};
        addFormSelector(F("Yazdırma Modu"), F("plugin_131_mod"), 7, options1, optionValues1, choice1);
        
        addFormCheckBox(F("Bartender prn"), F("plugin_131_bartender"), EYZPLU_Bartender);
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
        addFormSelector(F("Tek Etiket"), F("plugin_131_tek_prn"), 10, options2, optionValues2, choice2);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Etiket Dizayn Menüsüne Git"));

        byte choice3 = ExtraTaskSettings.TaskDevicePrint[1];

        addFormSelector(F("Artı Etiket"), F("plugin_131_art_prn"), 10, options2, optionValues2, choice3);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Etiket Dizayn Menüsüne Git"));

        byte choice4 = ExtraTaskSettings.TaskDevicePrint[2];

        addFormSelector(F("Toplam Etiket"), F("plugin_131_top_prn"), 10, options2, optionValues2, choice4);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Etiket Dizayn Menüsüne Git"));

#if FEATURE_SD
        byte choice5 = ExtraTaskSettings.TaskDevicePrint[3];
        addFormSelector(F("SD Data"), F("plugin_131_sd_prn"), 10, options2, optionValues2, choice5);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[3]], F("SD Data Dizayn Menüsüne Git"));
#endif
        byte choice6 = ExtraTaskSettings.TaskDevicePrint[4];

        addFormSelector(F("Server Data"), F("plugin_131_srv_prn"), 10, options2, optionValues2, choice6);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[4]], F("Server Data Dizayn Menüsüne Git"));

        if (EYZPLU_Mod == 2) {
          addFormTextBox(F("Hedef Kilogram"), F("plugin_131_hedef"), String(EYZPLU_Hedef, 3), HEDEF_BUFF_SIZE_P131);
          addFormNumericBox(F("Gecikme Saniye"), F("plugin_131_gecikme"), EYZPLU_Gecikme, 0, 999999);
        } else if (EYZPLU_Mod == 5) {
          addFormTextBox(F("Artı Komutu"), getPluginCustomArgName(0), EYZPLU_art_komut, MES_BUFF_SIZE_P131);
          addFormTextBox(F("Toplam Komutu"), getPluginCustomArgName(1), EYZPLU_top_komut, MES_BUFF_SIZE_P131);
          addFormTextBox(F("Tek Komutu"), getPluginCustomArgName(2), EYZPLU_tek_komut, MES_BUFF_SIZE_P131);
        }
        addFormSubHeader(F("İndikatör Ayarları"));
        indikator_secimi(event, EYZPLU_Indikator, F("plugin_131_indikator"));
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        EYZPLU_Model = getFormItemInt(F("plugin_131_model"));
        EYZPLU_Indikator = getFormItemInt(F("plugin_131_indikator"));
        EYZPLU_Mod = getFormItemInt(F("plugin_131_mod"));
        EYZPLU_Gecikme = getFormItemInt(F("plugin_131_gecikme"));
        EYZPLU_Bartender = isFormItemChecked(F("plugin_131_bartender"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));

        PCONFIG_FLOAT(0) = getFormItemFloat(F("plugin_131_hedef"));
        if (EYZPLU_Indikator == 5) {
          strncpy_webserver_arg(EYZPLU_art_komut, getPluginCustomArgName(0));
          strncpy_webserver_arg(EYZPLU_top_komut, getPluginCustomArgName(1));
          strncpy_webserver_arg(EYZPLU_tek_komut, getPluginCustomArgName(2));
        }

        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, EYZPLU_Indikator, PCONFIG(4));

        ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_131_tek_prn"));
        ExtraTaskSettings.TaskDevicePrint[1] = getFormItemInt(F("plugin_131_art_prn"));
        ExtraTaskSettings.TaskDevicePrint[2] = getFormItemInt(F("plugin_131_top_prn"));
        ExtraTaskSettings.TaskDevicePrint[3] = getFormItemInt(F("plugin_131_sd_prn"));
        ExtraTaskSettings.TaskDevicePrint[4] = getFormItemInt(F("plugin_131_srv_prn"));
        options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.tek_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[1]].toCharArray(Settings.art_prn, options2[ExtraTaskSettings.TaskDevicePrint[1]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[2]].toCharArray(Settings.top_prn, options2[ExtraTaskSettings.TaskDevicePrint[2]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[3]].toCharArray(Settings.sd_prn,  options2[ExtraTaskSettings.TaskDevicePrint[3]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[4]].toCharArray(Settings.srv_prn, options2[ExtraTaskSettings.TaskDevicePrint[4]].length() + 1);
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        #ifdef ESP32_CLASSIC
        EYZPLU_button1.attachClick(EYZPLU_click1);
        EYZPLU_button1.attachLongPressStart(EYZPLU_longPressStart1);
        //EYZPLU_button2.attachClick(EYZPLU_click2);
        #ifdef HAS_WIFI
        EYZPLU_button3.attachClick(EYZPLU_click3);
        EYZPLU_button4.attachClick(EYZPLU_click4);
        EYZPLU_button5.attachClick(EYZPLU_click5);
        EYZPLU_button1.tick();
        EYZPLU_button2.tick();
        EYZPLU_button3.tick();
        EYZPLU_button4.tick();
        EYZPLU_button5.tick();
        #endif
        //pinMode(5, OUTPUT);
        //digitalWrite(5, LOW);
        #endif
        /*if (!PCONFIG(4)) {
          switch (EYZPLU_Indikator) {
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
        }*/        
        display.begin(i2c_Address, true);
        display.display();
        display.clearDisplay();
        Settings.WebAPP = 131;
        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        display.clearDisplay();
        if(!internet)
          display.drawBitmap(0, 0, warning_icon16x16, 16, 16, 1);
        else
          display.drawBitmap(0, 0, okey_icon16x16, 16, 16, 1);
        long rssi = WiFi.RSSI();
        if (rssi >= -50)
          display.drawBitmap(112, 0, logo_bmp, 16, 8, 1);
        else if ((rssi < -50) & (rssi > -65))
          display.drawBitmap(112, 0, logo_bmp1, 16, 8, 1);
        else if (rssi< -65 & rssi > -80)
          display.drawBitmap(112, 0, logo_bmp2, 16, 8, 1);
        else if (rssi< -80 & rssi > -95) 
          display.drawBitmap(112, 0, logo_bmp3, 16, 8, 1);
        display.setTextSize(1);  // Normal 1:1 pixel scale
        display.setTextColor(SH110X_WHITE);
        display.setCursor(88, 0);
        display.println(rssi);
        display.setCursor(20, 0);  // Start at top-left corner
        display.println(SecuritySettings.WifiSSID);
        display.setCursor(0, 20);
        String pluadi = "%pluadi%";
        parseSystemVariables(pluadi, false);
        display.setTextSize(1,2);
        display.println(pluadi);
        display.setTextSize(2);
        display.setCursor(0, 50);
        display.println(String(XML_NET_C));
        display.display();
        success = true;
        break;
      }

      case PLUGIN_TEN_PER_SECOND:
      {
        switch (EYZPLU_Mod) {
          case 0:
            #ifdef ESP32_CLASSIC
            EYZPLU_button1.tick();
            //EYZPLU_button2.tick();
            #ifdef HAS_WiFi
            EYZPLU_button3.tick();
            EYZPLU_button4.tick();
            EYZPLU_button5.tick();
            #endif
            #endif
            break;
          case 2:
            if ((webapinettartim > EYZPLU_Hedef) && (hayvan_modu == 0)) {
              stabilTimer_l = millis() + (EYZPLU_Gecikme * 1000);
              StabilTartim_f = webapinettartim;
              hayvan_modu = 1;
            }
            if ((millis() > stabilTimer_l) && (hayvan_modu == 1)) {
              if (StabilTartim_f == webapinettartim) {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
                hayvan_modu = 2;
              } else {
                hayvan_modu = 0;
              }
            }
            if (((webapinettartim < EYZPLU_Hedef) && (webapinettartim > 0.001)) && (hayvan_modu == 2)) {
              hayvan_modu = 0;
            }
            break;
        }
        success = true;
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        /*IPAddress host(172,217,20,67);
        if (Ping.ping(host,3))
          internet = true;
        else
          internet = false;*/
        serial_error(event, EYZPLU_Mod, "eyzplutek");
        success = true;
        break;
      }

    case PLUGIN_WRITE:
      {
        if (Settings.UDPPort > 0) {
          string.replace("\n", "");
          string.replace("\r", "");
          string.replace(String(char(ExtraTaskSettings.TaskDeviceSonByte)), "");
          if ((String(EYZPLU_art_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_art_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart");
          else if ((String(EYZPLU_art_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_top_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
          else if ((String(EYZPLU_tek_komut).length() > 0) && (string.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_tek_komut)))
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
          else
            udp_client(event, EYZPLU_Indikator, string, EYZPLU_Mod);
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
            isaret(event, EYZPLU_Indikator, tartimString_s);
            if ((EYZPLU_Mod == 3) || (EYZPLU_Mod == 4))
              formul_kontrol(event, tartimString_s, EYZPLU_Mod, true);
            else {
              if (EYZPLU_Mod == 5) {
                if ((String(EYZPLU_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart");
                if ((String(EYZPLU_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
                if ((String(EYZPLU_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
              }
              formul_seri(event, tartimString_s, EYZPLU_Indikator);
              if ((EYZPLU_Mod == 1) && (webapinettartim > 0.001)) {
                sendData(event);
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
              }
              Serial1.flush();
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
            isaret(event, EYZPLU_Indikator, tartimString_s);
            if ((EYZPLU_Mod == 3) || (EYZPLU_Mod == 4))
              formul_kontrol(event, tartimString_s, EYZPLU_Mod, true);
            else {
              if (EYZPLU_Mod == 5) {
                if ((String(EYZPLU_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart");
                if ((String(EYZPLU_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
                if ((String(EYZPLU_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
              }
              formul_seri(event, tartimString_s, EYZPLU_Indikator);
              if ((EYZPLU_Mod == 1) && (webapinettartim > 0.001)) {
                sendData(event);
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
              }
              Serial.flush();
            }
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
            isaret(event, EYZPLU_Indikator, tartimString_s);
            if ((EYZPLU_Mod == 3) || (EYZPLU_Mod == 4))
              formul_kontrol(event, tartimString_s, EYZPLU_Mod, true);
            else {
              if (EYZPLU_Mod == 5) {
                if ((String(EYZPLU_art_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_art_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_art_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzpluart");
                if ((String(EYZPLU_top_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_top_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_top_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
                if ((String(EYZPLU_tek_komut).length() > 0) && (tartimString_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], (String(EYZPLU_tek_komut).length() + ExtraTaskSettings.TaskDeviceValueBas[0])) == String(EYZPLU_tek_komut)))
                  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyzplutek");
              }
              formul_seri(event, tartimString_s, EYZPLU_Indikator);
              if ((EYZPLU_Mod == 1) && (webapinettartim > 0.001)) {
                sendData(event);
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
              }
              Serial.flush();
            }
            tartimString_s = "";
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