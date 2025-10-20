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

// Modbus RTU Master için
#include <ModbusMaster.h>

// Modbus Slave için basit implementasyon
struct ModbusSlave {
  uint16_t registers[10]; // 40001-40010 registerları
  uint8_t slaveId;
  bool enabled;
  
  ModbusSlave() : slaveId(1), enabled(false) {
    memset(registers, 0, sizeof(registers));
  }
  
  uint16_t calculateCRC(uint8_t* data, uint8_t length);
  void processRequest();
  void sendResponse(uint8_t* response, uint8_t len);
};

// Modbus RTU değişkenleri
ModbusMaster modbus;
ModbusSlave modbusSlave;
unsigned long lastModbusRead = 0;
// Veri yenileme süresi (ms). Kullanıcının istediği üzere 200 ms
const unsigned long MODBUS_READ_INTERVAL = 200; // 200 ms
bool modbusEnabled = false;
bool isModbusMaster = true; // true=Master, false=Slave
uint16_t modbusNetAddr = 40001; // Net bilgisi adresi
uint16_t modbusTriggerAddr = 40003; // Tetik biti adresi
bool processComplete = false;

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

#include "OneButton.h"

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

// Modbus RTU fonksiyonları
bool readModbusHoldingRegister(uint16_t address, uint16_t &value) {
  if (!modbusEnabled) return false;
  
  uint8_t result = modbus.readHoldingRegisters(address - 40001, 1); // Modbus adresi 0-based
  if (result == modbus.ku8MBSuccess) {
    value = modbus.getResponseBuffer(0);
    return true;
  }
  return false;
}

bool writeModbusHoldingRegister(uint16_t address, uint16_t value) {
  if (!modbusEnabled) return false;
  
  uint8_t result = modbus.writeSingleRegister(address - 40001, value); // Modbus adresi 0-based
  return (result == modbus.ku8MBSuccess);
}

void processModbusData() {
  if (!modbusEnabled) {
    return;
  }
  
  if (isModbusMaster) {
    // MASTER MODU: Veri okuma ve yazma
    if (millis() - lastModbusRead < MODBUS_READ_INTERVAL) {
      return;
    }
    
    lastModbusRead = millis();
    hataTimer_l = millis();
    
    // Global değişkenlerden adres bilgilerini al
    uint16_t netAddr = modbusNetAddr; // Net bilgisi adresi
    uint16_t triggerAddr = modbusTriggerAddr; // Tetik biti adresi
    
    // Net bilgisini (kilo) oku
    uint16_t kiloRaw;
    if (readModbusHoldingRegister(netAddr, kiloRaw)) {
      // Kilo verisini global değişkene yaz
      webapinettartim = kiloRaw;
        
      XML_NET_S = String(webapinettartim, 0);
      dtostrf(webapinettartim, 8, 0, XML_NET_C);
      
      addLog(LOG_LEVEL_DEBUG, String(F("EYZ Modbus Master: ")) + String(netAddr) + 
             String(F(" adresinden Kilo = ")) + String(kiloRaw, int(ExtraTaskSettings.TaskDeviceValueDecimals[0])) + F(" kg"));
    }
    
    // Tetik bitini oku
    uint16_t triggerBit;
    if (readModbusHoldingRegister(triggerAddr, triggerBit)) {
      if (triggerBit == 1 && !processComplete) {
        // Eyztek komutunu çalıştır
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
        addLog(LOG_LEVEL_INFO, String(F("EYZ Modbus Master: ")) + String(triggerAddr) + 
               String(F(" adresinden Eyztek komutu tetiklendi")));
        processComplete = true;
        
        // İşlem tamamlandığında tetik adresini 0 yap
        if (writeModbusHoldingRegister(triggerAddr, 0)) {
          addLog(LOG_LEVEL_DEBUG, String(F("EYZ Modbus Master: ")) + String(triggerAddr) + 
                 String(F(" adresi sıfırlandı")));
        }
      } else if (triggerBit == 0) {
        processComplete = false; // Yeni tetikleme için hazır
      }
    }
  } else {
    // SLAVE MODU: İstekleri işle ve verileri sağla
    modbusSlave.processRequest();
    hataTimer_l = millis();
    
    // Global değişkenlerden dinamik adresler
    uint16_t triggerAddr = modbusTriggerAddr; // Tetik biti adresi
    
    // Register index hesaplama (40001 -> 0, 40002 -> 1, vs.)
    uint16_t triggerRegIndex = triggerAddr - 40001;
    
    // Eyztek işlemini tetiklemek için dinamik tetik adresini kontrol et
    if (triggerRegIndex < 10) {
      if (modbusSlave.registers[triggerRegIndex] == 1 && !processComplete) {
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
        addLog(LOG_LEVEL_INFO, String(F("EYZ Modbus Slave: ")) + String(triggerAddr) + 
               String(F(" adresinden Eyztek komutu tetiklendi")));
        processComplete = true;
        modbusSlave.registers[triggerRegIndex] = 0; // Tetik bitini sıfırla
      } else if (modbusSlave.registers[triggerRegIndex] == 0) {
        processComplete = false;
      }
    }
  }
}

/*void eyz_longPressStop2() {
  ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
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
        
        // Modbus RTU Ayarları
        addFormSubHeader(F("Modbus RTU Ayarları"));
        addFormCheckBox(F("Modbus RTU Etkin"), F("plugin_130_modbus_enable"), PCONFIG(5));
        
        // Sadece Modbus etkinse diğer ayarları göster
        if (PCONFIG(5)) {
          // Modbus Master/Slave seçimi
          byte choice_modbus = PCONFIG(7); // PCONFIG(7) = 0:Master, 1:Slave
          String modbus_options[2];
          modbus_options[0] = F("Master (Veri okuyucu)");
          modbus_options[1] = F("Slave (Veri sağlayıcı)");
          int modbus_optionValues[2] = {0, 1};
          addFormSelector(F("Modbus Modu"), F("plugin_130_modbus_mode"), 2, modbus_options, modbus_optionValues, choice_modbus);
          
          addFormNumericBox(F("Slave ID"), F("plugin_130_slave_id"), PCONFIG(6), 1, 247);
          addFormNumericBox(F("Baud Rate"), F("plugin_130_baud"), PCONFIG_LONG(0), 9600, 115200);
          
          // Modbus Register Adresleri
          addFormSubHeader(F("Modbus Register Adresleri"));
          addFormNumericBox(F("Net Bilgisi Adresi"), F("plugin_130_net_addr"), PCONFIG_LONG(1) ? PCONFIG_LONG(1) : 40001, 40001, 49999);
          addFormNote(F("Net bilgisi (kilo verisi) okunacak/yazılacak Modbus register adresi"));
          
          addFormNumericBox(F("Yazdır Tetik Adresi"), F("plugin_130_trigger_addr"), PCONFIG_LONG(2) ? PCONFIG_LONG(2) : 40003, 40001, 49999);
          addFormNote(F("Yazdırma işlemini tetikleyen bit'in bulunduğu Modbus register adresi"));
          
          if (PCONFIG(7) == 0) { // Master modu
            addFormNote(F("Master Modu: Belirtilen adreslerden veri okur ve tetik biti kontrol eder"));
          } else { // Slave modu  
            addFormNote(F("Slave Modu: Belirtilen adreslerde diğer cihazların okuyabileceği veriler sağlar"));
          }
        } else {
          addFormNote(F("Modbus RTU ayarları için önce 'Modbus RTU Etkin' seçeneğini işaretleyin"));
        }

        addFormSubHeader(F("Etiket Seçimi"));
        
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
        
        // Modbus RTU ayarlarını kaydet
        PCONFIG(5) = isFormItemChecked(F("plugin_130_modbus_enable"));
        PCONFIG(6) = getFormItemInt(F("plugin_130_slave_id"));
        PCONFIG(7) = getFormItemInt(F("plugin_130_modbus_mode")); // 0:Master, 1:Slave
        
        // Adresleri ayrı PCONFIG_LONG'lara kaydet
        uint16_t netAddr = getFormItemInt(F("plugin_130_net_addr"));
        uint16_t triggerAddr = getFormItemInt(F("plugin_130_trigger_addr"));
        PCONFIG_LONG(1) = netAddr;      // Net adresi
        PCONFIG_LONG(2) = triggerAddr;  // Tetik adresi
        
        PCONFIG_LONG(0) = getFormItemInt(F("plugin_130_baud"));
        
        // Debug: Kaydedilen adresleri logla
        addLog(LOG_LEVEL_INFO, String(F("EYZ: Net adresi kaydedildi: ")) + String(netAddr));
        addLog(LOG_LEVEL_INFO, String(F("EYZ: Tetik adresi kaydedildi: ")) + String(triggerAddr));
        
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
        eyz_button1.attachClick(eyz_click1);
        eyz_button1.attachDoubleClick(eyz_click1);
        eyz_button1.attachLongPressStart(eyz_click1);
        //eyz_button1.attachLongPressStop(eyz_longPressStop1);
        eyz_button2.attachClick(eyz_click2);
        eyz_button2.attachLongPressStart(eyz_longPressStart2);
        //eyz_button2.attachLongPressStop(eyz_longPressStop2);
        //eyz_button1.tick();
        eyz_button2.tick();
        
        // Modbus RTU başlatma (Master/Slave)
        if (PCONFIG(5)) { // Modbus etkinse
          modbusEnabled = true;
          isModbusMaster = (PCONFIG(7) == 0); // 0=Master, 1=Slave
          
          // Global değişkenlere adresleri kaydet - Ayrı PCONFIG_LONG'lardan oku
          modbusNetAddr = PCONFIG_LONG(1) ? PCONFIG_LONG(1) : 40001;      // Net adresi
          modbusTriggerAddr = PCONFIG_LONG(2) ? PCONFIG_LONG(2) : 40003;  // Tetik adresi

          // Debug: Yüklenen adresleri logla
          addLog(LOG_LEVEL_INFO, String(F("EYZ Init: Net adresi yüklendi: ")) + String(modbusNetAddr));
          addLog(LOG_LEVEL_INFO, String(F("EYZ Init: Tetik adresi yüklendi: ")) + String(modbusTriggerAddr));
          
          #ifdef ESP32
          #if FEATURE_ETHERNET
            Serial1.begin(PCONFIG_LONG(0) ? PCONFIG_LONG(0) : 9600, SERIAL_8N1, 14, 12); // RX=14, TX=12
            if (isModbusMaster) {
              modbus.begin(PCONFIG(6) ? PCONFIG(6) : 1, Serial1); // Slave ID
              addLog(LOG_LEVEL_INFO, F("EYZ: Modbus RTU Master başlatıldı"));
              hataTimer_l = millis(); // Serial error hatasını önle
            } else {
              modbusSlave.slaveId = PCONFIG(6) ? PCONFIG(6) : 1;
              modbusSlave.enabled = true;
              addLog(LOG_LEVEL_INFO, F("EYZ: Modbus RTU Slave başlatıldı"));
              hataTimer_l = millis(); // Serial error hatasını önle
            }
          #else
            // Pin seçenekleri:
            // 16,17 = Yazıcı için kullanılıyor (çakışma var)
            // 13,27 = Modbus için seçildi (çakışma yok)  
            // 14,12 = Eski pinler (timeout sorunu vardı)
            Serial2.begin(PCONFIG_LONG(0) ? PCONFIG_LONG(0) : 9600, SERIAL_8N1, 13, 27); // RX=13, TX=27 (Modbus için)
            if (isModbusMaster) {
              modbus.begin(PCONFIG(6) ? PCONFIG(6) : 1, Serial2); // Slave ID
              addLog(LOG_LEVEL_INFO, F("EYZ: Modbus RTU Master başlatıldı"));
            } else {
              modbusSlave.slaveId = PCONFIG(6) ? PCONFIG(6) : 1;
              modbusSlave.enabled = true;
              addLog(LOG_LEVEL_INFO, F("EYZ: Modbus RTU Slave başlatıldı"));
            }
          #endif
          #endif
        } else {
          modbusEnabled = false;
        }
        
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
        Settings.WebAPP = 130;
        success = true;
        break;
      }

    case PLUGIN_FIFTY_PER_SECOND:
      {
        // Modbus verilerini işle
        if (modbusEnabled) {
          processModbusData();
        }
        
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
  }
  return success;
}
#endif

// Modbus Slave fonksiyonları
void ModbusSlave::processRequest() {
  if (!enabled) return;
  
  #ifdef ESP32
  #if FEATURE_ETHERNET
    if (!Serial1.available()) return;
  #else
    if (!Serial2.available()) return;
  #endif
  #endif
  
  uint8_t buffer[256];
  uint8_t len = 0;
  
  // Serial veri okuma
  #ifdef ESP32
  #if FEATURE_ETHERNET
    while (Serial1.available() && len < 256) {
      buffer[len++] = Serial1.read();
    }
  #else
    while (Serial2.available() && len < 256) {
      buffer[len++] = Serial2.read();
    }
  #endif
  #endif
  
  if (len < 8) return; // Minimum Modbus frame
  
  // Slave ID kontrolü
  if (buffer[0] != slaveId) return;
  
  uint8_t function = buffer[1];
  uint16_t startAddr = (buffer[2] << 8) | buffer[3];
  uint16_t quantity = (buffer[4] << 8) | buffer[5];
  
  // Function 03: Read Holding Registers
  if (function == 0x03) {
    addLog(LOG_LEVEL_INFO, String(F("Modbus Slave: Okuma isteği - Başlangıç: ")) + String(startAddr) + 
           String(F(", Adet: ")) + String(quantity));
           
    uint8_t response[256];
    response[0] = slaveId;
    response[1] = 0x03;
    response[2] = quantity * 2; // Byte count
    
    int idx = 3;
    for (int i = 0; i < quantity; i++) {
      uint16_t regAddr = startAddr + i; // startAddr zaten 0-based (40001 -> 0)
      if (regAddr < 10) {
        response[idx++] = (registers[regAddr] >> 8) & 0xFF;
        response[idx++] = registers[regAddr] & 0xFF;
        addLog(LOG_LEVEL_DEBUG, String(F("Modbus Slave: Register[")) + String(regAddr) + 
               String(F("] = ")) + String(registers[regAddr]));
      } else {
        response[idx++] = 0;
        response[idx++] = 0;
      }
    }
    
    // CRC hesaplama
    uint16_t crc = calculateCRC(response, idx);
    response[idx++] = crc & 0xFF;        // CRC Low
    response[idx++] = (crc >> 8) & 0xFF; // CRC High
    
    sendResponse(response, idx);
  }
  
  // Function 06: Write Single Register
  else if (function == 0x06) {
    uint16_t regAddr = startAddr; // startAddr already 0-based
    uint16_t value = (buffer[4] << 8) | buffer[5];

    if (regAddr < 10) {
      registers[regAddr] = value;

      // Eğer yazılan register net register ise, XML ve web değişkenlerini güncelle
      if (regAddr == (modbusNetAddr - 40001)) {
        // Değeri XML_NET_S ile webapinettartim'e yaz
        webapinettartim = (float)value;
        
        XML_NET_S = String(webapinettartim, 0);
        dtostrf(webapinettartim, 8, 0, XML_NET_C);
        addLog(LOG_LEVEL_INFO, String(F("Modbus Slave: Net register yazildi, XML_NET_S guncellendi: ")) + XML_NET_S);
      }

      // Echo response with proper CRC
      uint16_t crc = calculateCRC(buffer, 6);
      buffer[6] = crc & 0xFF;
      buffer[7] = (crc >> 8) & 0xFF;
      sendResponse(buffer, 8);
    }
  }
}

void ModbusSlave::sendResponse(uint8_t* response, uint8_t len) {
  #ifdef ESP32
  #if FEATURE_ETHERNET
    Serial1.write(response, len);
  #else
    Serial2.write(response, len);
  #endif
  #endif
}

// Modbus CRC16 hesaplama fonksiyonu
uint16_t ModbusSlave::calculateCRC(uint8_t* data, uint8_t length) {
  uint16_t crc = 0xFFFF;
  
  for (uint8_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

//Dikomsan
//ETXSTXadd:       01   CRLF
//n/w:      0.09 gCRLF
//u/w:       0    CRLF
//pcs:       0   CRLF