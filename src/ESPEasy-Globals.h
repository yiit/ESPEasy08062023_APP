#ifndef ESPEASY_GLOBALS_H_
#define ESPEASY_GLOBALS_H_



#include "ESPEasy_common.h"


//#include <FS.h>



//enable reporting status to ESPEasy developers.
//this informs us of crashes and stability issues.
// not finished yet!
// #define FEATURE_REPORTING  1

//Select which plugin sets you want to build.
//These are normally automaticly set via the Platformio build environment.
//If you use ArduinoIDE you might need to uncomment some of them, depending on your needs
//If you dont select any, a version with a minimal number of plugins will be biult for 512k versions.
//(512k is NOT finsihed or tested yet as of v2.0.0-dev6)


//build all plugins that still are being developed and are broken or incomplete
//#define PLUGIN_BUILD_DEV

//add this if you want SD support (add 10k flash)
//#define FEATURE_SD 1







/*
// TODO TD-er: Declare global variables as extern and construct them in the .cpp.
// Move all other defines in this file to separate .h files
// This file should only have the "extern" declared global variables so it can be included where they are needed.
//
// For a very good tutorial on how C++ handles global variables, see:
//    https://www.fluentcpp.com/2019/07/23/how-to-define-a-global-constant-in-cpp/
// For more information about the discussion which lead to this big change:
//    https://github.com/letscontrolit/ESPEasy/issues/2621#issuecomment-533673956
*/





/*********************************************************************************************\
 * pinStatesStruct
\*********************************************************************************************/
/*
struct pinStatesStruct
{
  pinStatesStruct() : value(0), plugin(0), index(0), mode(0) {}
  uint16_t value;
  uint8_t plugin;
  uint8_t index;
  uint8_t mode;
} pinStates[PINSTATE_TABLE_MAX];
*/

#ifdef ESP32

/*#ifdef HAS_BLUETOOTH
#include <Arduino.h>
#include "BluetoothSerial.h"
//#include "../lib/BluetoothSerial/src/BluetoothSerial.h"
extern BluetoothSerial SerialBT;
extern uint8_t address[6];
extern bool BTconnected;
extern uint8_t bluetooth_led;
extern uint8_t bluetooth_buton;
extern const char *BTpin;
#else*/
//#endif

#if defined(HAS_BLE) or defined(HAS_BLE_CLIENT)
//#include <NimBLEAddress.h>
//#include <NimBLEDevice.h>
//extern BLERemoteCharacteristic *barcodeCharacteristic;
//extern BLEAddress *pServerAddress;



#include <NimBLEDevice.h>

extern BLEServer *pServer;
extern BLEClient *pClient;
extern BLECharacteristic *pTxCharacteristic; // Server'dan Notify
extern BLERemoteCharacteristic *pRemoteRxCharacteristic; // Client'dan Write
extern bool deviceConnected;
extern bool oldDeviceConnected;
extern bool clientConnected;
extern uint8_t txValue;


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID_UART              "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX         "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX         "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define SERVICE_UUID_PRINTER           "000018F0-0000-1000-8000-00805F9B34FB"
#define CHARACTERISTIC_UUID_PRINTER_RX "00002AF1-0000-1000-8000-00805F9B34FB"

#endif

#include "PCF8574.h"
extern TwoWire I2Cone;
extern TwoWire I2Ctwo;
// Set i2c address
extern PCF8574 pcf8574;
#endif

/*#include "HX711.h"
extern HX711 scale;*/

#include "../lib/NAU7802/src/NAU7802.h"
extern NAU7802 scale;

#include <MD5Builder.h>
extern MD5Builder _md5;

extern bool WebLisansIn;
extern int WebLisansInTimer;
extern int WebLisanseyzInTimer;

extern unsigned long randomNumber;
extern double timeunix;

extern char kfont1[4];
extern char kfont2[7];
extern char kfont3[4];
extern char kfont4[7];
extern char kfont5[4];
extern char kfont6[4];

extern char font1[7];
extern char font2[7];
extern char font3[7];
extern char font4[7];
extern char font5[7];
extern char font6[7];
extern char sol[4];
extern char orta[4];
extern char sag[4];
extern char beyaz[4];
extern char siyah[4];
extern char acik[4];
extern char koyu[4];
extern char kes[4];
extern char logo[16];
extern char logo58[14];
extern char hata_beep[4];
extern char okey_beep[4];
extern char qrkodbas[24];
extern char qrkodson[11];
extern char cekmece[6];
extern char altcizgipasif[4];
extern char altcizgiaktif[4];
extern char CR[2];
extern char LF[2];
extern char etiketcal[9];

extern String tartimString_s;
extern String XML_TARIH_S;
extern String XML_SAAT_S;
extern String XML_TARIH_V;
extern String XML_SAAT_V;
extern String XML_NET_S;
extern String XML_NET_V;
extern String XML_DARA_S;
extern String XML_DARA_V;
extern String XML_BRUT_S;
extern String XML_BRUT_V;
extern String XML_NET_S_2;
extern String XML_DARA_S_2;
extern String XML_BRUT_S_2;
extern String XML_ADET_S;
extern String XML_ADET_GRAMAJ_S;
extern String XML_PLU_NO_S;
extern String XML_PLU_ADI_S;
extern String XML_PLU_KOD_S;
extern String XML_BARKOD_S;
extern String XML_BIRIM_FIYAT_S;
extern String XML_TUTAR_S;
extern String XML_SNO_S;
extern String XML_DURUS_ZAMANI_S;

extern String XML_SAYAC_1_S;
extern String XML_SAYAC_1_SONSUZ_S;
extern String XML_SAYAC_1_GECIKME_S;
extern String XML_SAYAC_2_S;
extern String XML_SAYAC_2_SONSUZ_S;
extern String XML_SAYAC_2_GECIKME_S;
extern String XML_SAYAC_3_S;
extern String XML_SAYAC_3_SONSUZ_S;
extern String XML_SAYAC_3_GECIKME_S;
extern String XML_SAYAC_4_S;
extern String XML_SAYAC_4_SONSUZ_S;
extern String XML_SAYAC_4_GECIKME_S;

extern String XML_SERI_NO_S;
extern String XML_FIS_NO_S;
extern String XML_TOP_NET_S;
extern String XML_TOP_DARA_S;
extern String XML_TOP_BRUT_S;
extern String XML_TOP_ADET_S;
extern String XML_STABIL_S;
extern String XML_FORMUL1_S;
extern String XML_FORMUL2_S;
extern String XML_FORMUL1_KATSAYI_S;
extern String XML_FORMUL2_KATSAYI_S;
extern String XML_RFIDKOD_S;
extern String XML_FIS_BASLIK1_S;
extern String XML_FIS_BASLIK2_S;
extern String XML_FIS_BASLIK3_S;
extern String XML_FIS_BASLIK4_S;
extern String XML_EAN8_S;
extern String XML_EAN13_S;
extern String XML_QRKOD_S;

extern String XML_V0;
extern String XML_V1;
extern String XML_V2;
extern String XML_V3;
extern String XML_V4;
extern String XML_V5;
extern String XML_V6;
extern String XML_V7;
extern String XML_V8;
extern String XML_V9;
extern String XML_V10;
extern String XML_V11;
extern String XML_V12;
extern String XML_V13;
extern String XML_V14;
extern String XML_V15;
extern String XML_V16;
extern String XML_V17;
extern String XML_V18;
extern String XML_V19;
extern String XML_V20;
extern String XML_V21;
extern String XML_V22;
extern String XML_V23;
extern String XML_V24;
extern String XML_V25;
extern String XML_V26;
extern String XML_V27;
extern String XML_V28;
extern String XML_V29;

extern String XML_FIRMA_ADI_S;
extern String XML_MUSTERI_ADI_S;
extern String XML_PLAKA_NO_S;
extern String XML_OPERATOR_ADI_S;

extern String XML_MESAJ1_S;
extern String XML_MESAJ2_S;
extern String XML_MESAJ3_S;
extern String XML_MESAJ4_S;
extern String XML_MESAJ5_S;
extern String XML_MESAJ6_S;
extern String XML_MESAJ7_S;
extern String XML_MESAJ8_S;
extern String XML_MESAJ9_S;

extern char XML_MESAJ1_C[10];
extern char XML_MESAJ2_C[10];
extern char XML_MESAJ3_C[10];
extern char XML_MESAJ4_C[10];
extern char XML_MESAJ5_C[10];
extern char XML_MESAJ6_C[10];
extern char XML_MESAJ7_C[10];
extern char XML_MESAJ8_C[10];
extern char XML_MESAJ9_C[10];

extern String kopya_etiket;

//extern String XML_DATA[30];

extern char XML_NET_C[9];
extern char XML_DARA_C[9];
extern char XML_BRUT_C[9];
extern char XML_NET_C_2[9];
extern char XML_DARA_C_2[9];
extern char XML_BRUT_C_2[9];
extern char XML_ADET_C[9];
extern char XML_ADET_GRAMAJ_C[9];
extern char XML_TOP_NET_C[11];
extern char XML_TOP_DARA_C[11];
extern char XML_TOP_BRUT_C[11];
extern char XML_SNO_C[4];
extern char XML_SAYAC_1_C[7];
extern char XML_SAYAC_2_C[7];
extern char XML_SAYAC_3_C[7];
extern char XML_SAYAC_4_C[7];
extern char XML_SERI_NO_C[9];
extern char XML_FIS_NO_C[9];
extern char XML_BARKOD_C[6];
extern char XML_FORMUL1_C[7];
extern char XML_FORMUL2_C[7];
extern char XML_FORMUL1_KATSAYI_C[9];
extern char XML_FORMUL2_KATSAYI_C[9];
extern char yazdir_c[2048];
extern char yazdir_xml[2048];
extern char XML_INPUT_PIN_C[4];
extern char XML_OUTPUT_PIN_C[4];
extern char XML_webapinettartim_son_C[9];
extern char XML_webapinettartim_C[9];

extern float isaret_f;
extern float isaret_f_2;
extern float webapinettartim;
extern float webapinettartim_son;
extern float webapidaratartim;
extern float webapibruttartim;
extern float webapinettartim_2;
extern float webapidaratartim_2;
extern float webapibruttartim_2;
extern float webapiadet;
extern float webapiadetgr;
extern int webapipluno;
extern float webapibfiyat;
extern float webapitutar;
extern float StabilTartim_f;
extern float webapikatsayi1;
extern float webapikatsayi2;

extern float ind_sifir_degeri;
extern float ind_dara_degeri;

extern String IRDA_DATA_S;

extern unsigned long hataTimer_l;
extern unsigned long gosterTimer_l;
extern unsigned long stabilTimer_l;
extern unsigned long button_basildi;

extern char *plu_adi[500];

extern uint32_t sno;
extern int bluetooth_mod;
extern float top_net;
extern float top_dara;
extern float top_brut;
extern long fis_no;
extern uint32_t seri_no;

extern String message0;
extern String message1;
extern String message2;
extern String message3;
extern String message4;
extern String message5;
extern String message6;
extern String message7;
extern String message8;
extern String message9;
extern String data_s;
extern String json_net;
extern String tartimdata_s;
extern String paketVeri_s;

extern String stabilTartim_s;

extern int artyaz;
extern int topyaz;
extern int hayvan_modu;

extern bool oto_yazdir;
extern bool escpos_mod;
extern unsigned long escpos_time;
extern bool tspl_mod;
extern unsigned long tspl_time;
extern bool WifiAPMode;

extern String options2[10];

extern bool karakter_195;
extern bool karakter_196;
extern bool karakter_197;

extern boolean printToWeb;
extern String printWebString;
extern boolean printToWebJSON;


//struct RTC_cache_handler_struct;


// FIXME TD-er: Must move this to some proper class (ESPEasy_Scheduler ?)
extern unsigned long timermqtt_interval;


extern unsigned long lastSend;
extern unsigned long lastWeb;

extern unsigned long wdcounter;
extern unsigned long timerAwakeFromDeepSleep;


#if FEATURE_ADC_VCC
extern float vcc;
#endif


extern bool shouldReboot;
extern bool firstLoop;

// This is read from the settings at boot.
// Even if this setting is changed, you need to reboot to activate the changes.
extern boolean UseRTOSMultitasking;

#ifdef ESP_NOW_ACTIVE
#include <esp_now.h>
#include <Preferences.h>

extern uint8_t espnow_led;

#define espnow_hata "Eslesmis cihaz yok, veri gonderilemiyor."
#define MAX_PAIRED_DEVICES 6

// Degiskenler
extern Preferences preferences;
extern uint8_t pairedMacList[MAX_PAIRED_DEVICES][6];  // Eslesmis cihazin MAC adresi
extern int pairedDeviceCount; // Eslesmis cihaz sayısını tutar

struct PeerStatus {
  uint8_t mac[6];
  bool active;
};

extern PeerStatus peerStatusList[MAX_PAIRED_DEVICES];
extern int peerStatusCount;

// Tarama sonucu geçici cihaz listesi
extern uint8_t discoveredMacList[MAX_PAIRED_DEVICES][6];
extern int discoveredCount;

// Broadcast adresini belirle
extern uint8_t broadcastAddress[];
extern esp_now_peer_info_t peerInfo;

extern unsigned long buttonPressStartTime; // Butona basılma baslangıc zamanı
extern bool buttonPressed;

extern bool isPaired; // Eslesme durumu

void StartPairing();
void EspnowSendKomut(const char *komut);
void EspnowSendData(String data);
#endif

#endif /* ESPEASY_GLOBALS_H_ */
