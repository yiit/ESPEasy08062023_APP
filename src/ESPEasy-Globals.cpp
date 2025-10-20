#include "ESPEasy-Globals.h"

#ifdef ESP32

/*#ifdef HAS_BLUETOOTH
#include "BluetoothSerial.h"
//#include "../lib/BluetoothSerial/src/BluetoothSerial.h"
BluetoothSerial SerialBT;
uint8_t address[6];
boolean BTconnected     = false;
uint8_t bluetooth_led   = 27;  //2;   //19;
uint8_t bluetooth_buton = 22;  //27;  //14; //5; //18;
const char *BTpin = "1234";
#else*/
uint8_t espnow_led = 27;
//#endif

#if defined(HAS_BLE) or defined(HAS_BLE_CLIENT)
//#include <NimBLEAddress.h>
//#include <NimBLEDevice.h>
//BLERemoteCharacteristic *barcodeCharacteristic;
//BLEAddress *pServerAddress;


BLEServer *pServer = nullptr;
BLEClient *pClient = nullptr;
BLECharacteristic *pTxCharacteristic = nullptr; // Server'dan Notify
BLERemoteCharacteristic *pRemoteRxCharacteristic = nullptr; // Client'dan Write
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool clientConnected = false;
uint8_t txValue = 0;
#endif

#include "PCF8574.h"
TwoWire I2Cone = TwoWire(0);
TwoWire I2Ctwo = TwoWire(1);
// Set i2c address
//PCF8574 pcf8574(&I2Ctwo, 0x20, 4, 2);
PCF8574 pcf8574(&I2Ctwo, 0x20, 4, 15);
//PCF8574 pcf8574(&I2Ctwo, 0x20, 2, 4);
#endif

/*#include "HX711.h"
HX711 scale;*/

#include "../lib/NAU7802/src/NAU7802.h"

NAU7802 scale;

#include <MD5Builder.h>
MD5Builder _md5;

bool WebLisansIn = false;
int WebLisansInTimer = 300;
int WebLisanseyzInTimer = 300;

unsigned long randomNumber = 0;
double timeunix = 0;

char kfont1[4]    = { 27, 87, 1 };             //32
char kfont2[7]    = { 27, 87, 1, 27, 86, 2 };  //32
char kfont3[4]    = { 27, 87, 2 };             //32
char kfont4[7]    = { 27, 87, 1, 27, 86, 3 };  //16
char kfont5[4]    = { 27, 87, 3 };             //10
char kfont6[4]    = { 27, 87, 4 };             //8

char font1[7]     = { 27, 77, 49, 29, 33, 01 };  //64
char font2[7]     = { 27, 77, 48, 29, 33, 01 };  //48
char font3[7]     = { 27, 77, 49, 29, 33, 17 };  //32
char font4[7]     = { 27, 77, 48, 29, 33, 17 };  //24
char font5[7]     = { 27, 77, 48, 29, 33, 33 };  //16
char font6[7]     = { 27, 77, 48, 29, 33, 49 };  //12
char sol[4]       = { 27, 97, 94 };
char orta[4]      = { 27, 97, 01 };
char sag[4]       = { 27, 97, 02 };
char beyaz[4]     = { 29, 66, 94 };
char siyah[4]     = { 29, 66, 01 };
char acik[4]      = { 27, 69, 94 };
char koyu[4]      = { 27, 69, 01 };
char kes[4]       = { 29, 86, 49 };
char logo[16]     = { 27, 64, 27, 97, 01, 29, 84, 28, 112, 01, 48, 27, 64, 94, 94 };
char cekmece[6]   = { 16, 20, 1, 1, 5 };
char altcizgiaktif[4]  = { 27, 45, 48};
char altcizgipasif[4]  = { 27, 45, 50};

char hata_beep[4] = { 27, 66, 03, 04 };
char okey_beep[4] = { 27, 66, 01, 02 };

char qrkodbas[24] = { 27, 64, 10, 29, 40, 107, 48, 103, 4, 29, 40, 107, 48, 105, 72, 29, 40, 107, 48, 128, 254, 94 };
//const char qrkodbas[24]  = {27, 64, 10,  29, 40, 107, 48, 103,  7, 29, 40, 107, 48, 105, 72, 29, 40, 107, 48, 128, 254, 94};
char qrkodson[11] = { 94, 29, 40, 107, 48, 129, 27, 74, 200 };
char CR[2]        = { 13 };
char LF[2]        = { 10 };
char etiketcal[9] = { 31, 27, 26, 3, 1, 94, 13, 10};

/*u char qrkodbas[36]  = {27, 64, 29, 40, 107,  4, 94, 49, 65, 50, 94, 29, 40, 107,  3, 94, 49, 67,  4, 29, 40, 107,  3, 94, 49, 69, 48, 29, 40, 107, 253, 254, 49, 80, 48};
uchar qrkodson[18]  = {29, 40, 107,  3, 94,  49, 81, 48, 29, 40, 107,  3, 94,  49, 82, 48};*/

String json_net = "";
String tartimString_s    = "";
String XML_TARIH_S       = "ERROR";
String XML_SAAT_S        = "ERROR";
String XML_TARIH_V       = "ERROR";
String XML_SAAT_V        = "ERROR";
String XML_NET_S         = "ERROR";
String XML_NET_V         = "ERROR";
String XML_DARA_S        = "ERROR";
String XML_DARA_V        = "ERROR";
String XML_BRUT_S        = "ERROR";
String XML_BRUT_V        = "ERROR";
String XML_NET_S_2       = "ERROR";
String XML_DARA_S_2      = "ERROR";
String XML_BRUT_S_2      = "ERROR";
String XML_ADET_S        = "ERROR";
String XML_ADET_GRAMAJ_S = "ERROR";
String XML_PLU_NO_S      = "ERROR";
String XML_PLU_ADI_S     = "ERROR";
String XML_PLU_KOD_S     = "ERROR";
String XML_BARKOD_S      = "ERROR";
String XML_BIRIM_FIYAT_S = "ERROR";
String XML_TUTAR_S       = "ERROR";
String XML_SNO_S         = "ERROR";
String XML_DURUS_ZAMANI_S = "ERROR";

String XML_SAYAC_1_S         = "0";
String XML_SAYAC_1_SONSUZ_S  = "0";
String XML_SAYAC_1_GECIKME_S = "0";
String XML_SAYAC_2_S         = "0";
String XML_SAYAC_2_SONSUZ_S  = "0";
String XML_SAYAC_2_GECIKME_S = "0";
String XML_SAYAC_3_S         = "0";
String XML_SAYAC_3_SONSUZ_S  = "0";
String XML_SAYAC_3_GECIKME_S = "0";
String XML_SAYAC_4_S         = "0";
String XML_SAYAC_4_SONSUZ_S  = "0";
String XML_SAYAC_4_GECIKME_S = "0";

String XML_SERI_NO_S     = "0";
String XML_FIS_NO_S      = "ERROR";
String XML_STABIL_S      = "ST";
String XML_TOP_NET_S     = "ERROR";
String XML_TOP_DARA_S    = "ERROR";
String XML_TOP_BRUT_S    = "ERROR";
String XML_TOP_ADET_S    = "ERROR";
String XML_RFIDKOD_S     = "ERROR";
String XML_EAN8_S        = "ERROR";
String XML_EAN13_S       = "ERROR";
String XML_QRKOD_S       = "ERROR";


//String XML_DATA[30];

String XML_V0  = "V0";
String XML_V1  = "V1";
String XML_V2  = "V2";
String XML_V3  = "V3";
String XML_V4  = "V4";
String XML_V5  = "V5";
String XML_V6  = "V6";
String XML_V7  = "V7";
String XML_V8  = "V8";
String XML_V9  = "V9";
String XML_V10 = "V10";
String XML_V11 = "V11";
String XML_V12 = "V12";
String XML_V13 = "V13";
String XML_V14 = "V14";
String XML_V15 = "V15";
String XML_V16 = "V16";
String XML_V17 = "V17";
String XML_V18 = "V18";
String XML_V19 = "V19";
String XML_V20 = "V20";
String XML_V21 = "V21";
String XML_V22 = "V22";
String XML_V23 = "V23";
String XML_V24 = "V24";
String XML_V25 = "V25";
String XML_V26 = "V26";
String XML_V27 = "V27";
String XML_V28 = "V28";
String XML_V29 = "V29";

String XML_FORMUL1_S         = "ERROR";
String XML_FORMUL2_S         = "ERROR";
String XML_FORMUL1_KATSAYI_S = "ERROR";
String XML_FORMUL2_KATSAYI_S = "ERROR";

String XML_FIRMA_ADI_S       = "ERROR";
String XML_MUSTERI_ADI_S     = "ERROR";
String XML_PLAKA_NO_S        = "ERROR";
String XML_OPERATOR_ADI_S    = "ERROR";

String XML_MESAJ1_S          = "ERROR";
String XML_MESAJ2_S          = "ERROR";
String XML_MESAJ3_S          = "ERROR";
String XML_MESAJ4_S          = "ERROR";
String XML_MESAJ5_S          = "ERROR";
String XML_MESAJ6_S          = "ERROR";
String XML_MESAJ7_S          = "ERROR";
String XML_MESAJ8_S          = "ERROR";
String XML_MESAJ9_S          = "ERROR";

String XML_FIS_BASLIK1_S = "";
String XML_FIS_BASLIK2_S = "";
String XML_FIS_BASLIK3_S = "";
String XML_FIS_BASLIK4_S = "";

String kopya_etiket = "";
String IRDA_DATA_S = "";

char XML_MESAJ1_C[10];
char XML_MESAJ2_C[10];
char XML_MESAJ3_C[10];
char XML_MESAJ4_C[10];
char XML_MESAJ5_C[10];
char XML_MESAJ6_C[10];
char XML_MESAJ7_C[10];
char XML_MESAJ8_C[10];
char XML_MESAJ9_C[10];

char XML_NET_C[9];
char XML_DARA_C[9];
char XML_BRUT_C[9];
char XML_NET_C_2[9];
char XML_DARA_C_2[9];
char XML_BRUT_C_2[9];
char XML_ADET_C[9];
char XML_ADET_GRAMAJ_C[9];
char XML_TOP_NET_C[11];
char XML_TOP_DARA_C[11];
char XML_TOP_BRUT_C[11];
char XML_SNO_C[4];
char XML_SAYAC_1_C[7] = { "0" };
char XML_SAYAC_2_C[7] = { "0" };
char XML_SAYAC_3_C[7] = { "0" };
char XML_SAYAC_4_C[7] = { "0" };
char XML_SERI_NO_C[9];
char XML_FIS_NO_C[9];
char XML_BARKOD_C[6];
char XML_FORMUL1_C[7];
char XML_FORMUL2_C[7];
char XML_FORMUL1_KATSAYI_C[9];
char XML_FORMUL2_KATSAYI_C[9];
char yazdir_c[2048];
char yazdir_xml[2048];
char XML_INPUT_PIN_C[4];
char XML_OUTPUT_PIN_C[4];
char XML_webapinettartim_son_C[9];
char XML_webapinettartim_C[9];


float isaret_f = 1;
float isaret_f_2 = 1;
float webapinettartim = 0;
float webapinettartim_son = 0;
float webapidaratartim = 0;
float webapibruttartim = 0;
float webapinettartim_2 = 0;
float webapidaratartim_2 = 0;
float webapibruttartim_2 = 0;
float webapiadet = 0;
float webapiadetgr = 0;
int webapipluno = 0;
float webapibfiyat = 0;
float webapitutar = 0;
float StabilTartim_f = 0;
float webapikatsayi1 = 0;
float webapikatsayi2 = 0;

float ind_sifir_degeri = 0;
float ind_dara_degeri = 0;

unsigned long hataTimer_l = 0;
unsigned long gosterTimer_l = 0;
unsigned long stabilTimer_l = 0;
unsigned long button_basildi = 0;

bool LisansControl = false;
char *plu_adi[500];

int bluetooth_mod = 1;
float top_net = 0;
float top_dara = 0;
float top_brut = 0;
long fis_no = 0;
uint32_t seri_no = 0;
uint32_t sno = 0;

String message0 = "";
String message1 = "";
String message2 = "";
String message3 = "";
String message4 = "";
String message5 = "";
String message6 = "";
String message7 = "";
String message8 = "";
String message9 = "";
String data_s = "";
String tartimdata_s = "";
String paketVeri_s = "";

String stabilTartim_s = "";

int artyaz = 0;
int topyaz = 0;

bool oto_yazdir = false;
bool escpos_mod = false;
unsigned long escpos_time = 0;
bool tspl_mod = false;
unsigned long tspl_time = 0;
bool WifiAPMode;


String options2[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

int hayvan_modu = 0;

bool karakter_195 = false;
bool karakter_196 = false;
bool karakter_197 = false;


boolean printToWeb = false;
String printWebString;
boolean printToWebJSON = false;



unsigned long timermqtt_interval = 250;
unsigned long lastSend = 0;
unsigned long lastWeb = 0;

unsigned long wdcounter = 0;
unsigned long timerAwakeFromDeepSleep = 0;


#if FEATURE_ADC_VCC
float vcc = -1.0f;
#endif

bool shouldReboot(false);
bool firstLoop(true);


boolean UseRTOSMultitasking(false);

#ifdef ESP_NOW_ACTIVE

#include <esp_now.h>
#include "src/Globals/Settings.h"
#include <Preferences.h>

// Degiskenler
Preferences preferences;
uint8_t pairedMacList[MAX_PAIRED_DEVICES][6];  // Eslesmis cihazin MAC adresi
int pairedDeviceCount; // Eslesmis cihaz sayısını tutar

PeerStatus peerStatusList[MAX_PAIRED_DEVICES];
int peerStatusCount = 0;

// Tarama sonucu geçici cihaz listesi
uint8_t discoveredMacList[MAX_PAIRED_DEVICES][6];
int discoveredCount = 0;

// Broadcast adresini belirle
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

unsigned long buttonPressStartTime = 0; // Butona basılma baslangıc zamanı
bool buttonPressed = false;

bool isPaired = false; // Eslesme durumu

void StartPairing() {
  Serial.println("Eslesme Modu Baslatildi...");
  const char *pairMessage = "PAIR_REQUEST";

  // Broadcast adresine mesaj gonder
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)pairMessage, strlen(pairMessage));
  if (result == ESP_OK) {
    Serial.println("Eslesme Istegi Basariyla Gonderildi.");
  } else {
    Serial.println("Eslesme Istegi Gonderilemedi.");
  }
}

void EspnowSendKomut(const char *komut) {
  /*if (!isPaired) {
    Serial.println("❌ Eşleşmiş cihaz yok, gönderilemiyor.");
    return;
  }*/

  esp_err_t result = esp_now_send(Settings.espnow_mac_address, (uint8_t *)komut, strlen(komut));

  if (result == ESP_OK) {
    Serial.printf("✅ '%s' komutu gönderildi.\n", komut);
  } else {
    Serial.printf("❌ '%s' komutu gönderilemedi! Hata kodu: %d (0x%X)\n", komut, result, result);
  }
}

void EspnowSendData(String data) {
	if (!isPaired) {
	  Serial.println(espnow_hata);
	  return;
	}
  
	if (pairedDeviceCount == 0) {
	  Serial.println(espnow_hata);
	  return;
	}
	
	for (int i = 0; i < pairedDeviceCount; i++) { // Sadece kayıtlı cihazlar kadar dongu
	  if (pairedMacList[i] != nullptr) { // MAC adresi gecerli mi kontrol et
		  esp_err_t result = esp_now_send(pairedMacList[i], (uint8_t *)data.c_str(), data.length());
		  if (result != ESP_OK) {
		    Serial.printf("Veri gonderme hatası: Cihaz %d\n", i);
		  }
	  } else {
		  Serial.printf("Gecersiz MAC adresi: Cihaz %d\n", i);
	  }
	}
}
  
#endif