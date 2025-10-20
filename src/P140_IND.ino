#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#include "ESPEasy_common.h"
#include "ESPEasy-Globals.h"

#ifdef USES_P140

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
#include "src/Globals/Settings.h"

#include "src/Helpers/ESPEasy_Storage.h"
#include "src/Helpers/Memory.h"
#include "src/Helpers/StringConverter.h"
#include "src/Helpers/StringParser.h"
#include "src/Helpers/Networking.h"

#include "src/ESPEasyCore/ESPEasy_Log.h"

#include <Wire.h>

#define RS232Baud 115200
#define RS232Format SERIAL_8N1

//#######################################################################################################
//#################################### Plugin 140: SCALE IND  #######################################
//#######################################################################################################



#define PLUGIN_140
#define PLUGIN_ID_140 140
#define PLUGIN_NAME_140 "Scale - IND"
#define PLUGIN_VALUENAME1_140 "NET"
#define PLUGIN_VALUENAME2_140 "DARA"
#define PLUGIN_VALUENAME3_140 "BRUT"

const uint8_t LOADCELL_DOUT_PIN = 11;
const uint8_t LOADCELL_SCK_PIN = 10;

bool indkaydet = true;

#define CUSTOMTASK_STR_SIZE_P140 40

#define IND_SIFIR_KAL_DEGERI_ADDR_SIZE_P140 9
#define IND_YUK_KAL_DEGERI_ADDR_SIZE_P140 9
#define IND_KAPASITE_DEGERI_ADDR_SIZE_P140 9

#define IND_SIFIR_KAL_DEGERI_BUFF_SIZE_P140 9
#define IND_YUK_KAL_DEGERI_BUFF_SIZE_P140 9
#define IND_KAPASITE_DEGERI_BUFF_SIZE_P140 9

#define IND_SIFIR_KAL_DEGERI PCONFIG_FLOAT(0)
#define IND_YUK_KAL_DEGERI PCONFIG_FLOAT(1)
#define IND_KAPASITE PCONFIG_FLOAT(2)

#define IND_EKRAN_MOD PCONFIG(0)
#define IND_TAKSIMAT PCONFIG(1)
#define IND_NOKTA PCONFIG(2)
#define IND_FILTRE PCONFIG(3)
#define IND_HIZ PCONFIG(4)
#define IND_UDP_AKTiF PCONFIG(5)
#define IND_TCP_AKTiF PCONFIG(6)
#define IND_TCP_PORT PCONFIG(7)
#define IND_TCPMODBUS_AKTiF PCONFIG(8)
#define IND_KAYIT_TIMER PCONFIG(9)

#define IND_Bartender ExtraTaskSettings.TaskPrintBartender

char goster_net_c[10];
char goster_dara_c[10];
char goster_brut_c[10];

int    ind_stabil_sayisi = 0;
float  bol = 0;
float  ind_tartim_degeri_son = 0;
float  ind_tartim_degeri_son1 = 0;
String file_data = "";

bool output_1_aktif = true;
bool output_2_aktif = true;
bool output_3_aktif = true;
bool output_4_aktif = true;

WiFiServer *sernetServer_140;
WiFiClient sernetClients_140;  //[MAX_SRV_CLIENTS];

const int fifoSize = 255;        // FIFO boyutu
float fifoBuffer[fifoSize];     // FIFO tampon
int fifoIndex = 0;              // FIFO tamponundaki mevcut indeks
int numValues = 0;              // FIFO içindeki toplam değer sayısı
float fifoSum = 0;              // FIFO'daki değerlerin toplamı

const int trendBufferSize = 5; // Kısa süreli tampon boyutu
float trendBuffer[trendBufferSize] = {0}; // Trend verileri için tampon
int trendIndex = 0; // Trend tamponu için indeks
int trendCount = 0; // Trend tamponundaki toplam değer sayısı


float filterFactor = 0.7;       // Dinamik olarak değişecek başlangıç filtre katsayısı

// Önceki değerler
long lastWeight = 0;            // Bir önceki ölçülen ağırlık
unsigned long lastTime = 0;     // Bir önceki ölçüm zamanı

long kayit_millis = 0;

#include "ModbusTCPSlave.h"
ModbusTCPSlave Mb_140;

// Function to determine stable or unstable state
String weightState(float currentWeight, int stable) {
  static float lastWeight = 0;
  static unsigned long stableCounter = 0;
  if (currentWeight == lastWeight) {
    stableCounter++;
    if (stableCounter >= stable) {
      if (indkaydet) {
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indkaydet");
        indkaydet = false;
      }
      return "ST"; // Weight is stable
    }
  } else {
    stableCounter = 0; // Reset stability counter on change
    indkaydet = true;
  }
  lastWeight = currentWeight;
  return "US"; // Weight is changing
}

float adjustDynamicFilterFactor(float currentWeight, float lastWeight, unsigned long currentTime, unsigned long lastTime, int taksimatStep) {
    float weightChange = abs(currentWeight - lastWeight); // Ağırlık değişimi
    unsigned long timeElapsed = currentTime - lastTime; // Geçen süre
    float changeRate = (timeElapsed > 0) ? float(weightChange) / timeElapsed : 0;

    if (changeRate > (2 * taksimatStep)) { // Çok hızlı değişim
        return 0.3; }  // Düşük filtreleme
    else if (weightChange <= taksimatStep) { // Çok az değişim
        return 0.95; }  // Stabil sonuçlar için yüksek filtreleme
    else if (timeElapsed > 1000) {// Uzun süre sabit ölçüm
        return 0.85; }
    else { // Normal değişim
        return 0.7; }
}

float applyTaksimat(float rawWeight, int taksimatStep, int decimalPlaces) {
    // 1. Ağırlığı tam sayıya çevirmek için ölçekleme
    float scaleFactor = pow(10, decimalPlaces);
    long scaledWeight = round(rawWeight * scaleFactor);

    // 2. Taksimat adımını ölçeklemek
    long scaledTaksimat = round(taksimatStep * scaleFactor);

    // 3. Taksimat adımına yuvarlama
    long roundedScaledWeight = (scaledWeight / scaledTaksimat) * scaledTaksimat;

    // 4. Sonucu tekrar orijinal ölçeğe geri dönüştürmek
    return float(roundedScaledWeight) / scaleFactor;
}

bool isStable(float currentWeight, float average, float stabilityThreshold) {
  // Ağırlığın stabil olup olmadığını kontrol et
  return abs(currentWeight - average) <= (float)stabilityThreshold;
}

float updateMovingAverageWithStability(
  float newValue, float* buffer, int& index, int& numValues, 
  int fifoSize, float& sum, float deviationThreshold, 
  int stabilityThreshold, int taksimatStep, int consecutiveThreshold
) {
  // Mevcut ortalamayı hesapla
  float average = (numValues > 0) ? sum / numValues : 0;

  // Aykırı değer kontrolü: Değerler trend kontrolü için kullanılacak
  static int outlierCount = 0;
  static bool lastWasHigh = false;

  // Yeni değer ve ortalama farkını kontrol et
  float deviation = abs(newValue - average);

  if (deviation > deviationThreshold && deviation < taksimatStep * 3) {
      // Eğer iki ardışık yüksek/düşük değer varsa, yok say
      if ((newValue > average && lastWasHigh) || (newValue < average && !lastWasHigh)) {
          outlierCount++;
          if (outlierCount >= consecutiveThreshold) {
              Serial.println("Sürekli yüksek/düşük trend algılandı, dikkate alınıyor.");
              outlierCount = 0; // Reset
              lastWasHigh = newValue > average;
              //XML_STABIL_S = "US"; // Unstabil durum
          } else {
              Serial.println("Yüksek/düşük aykırı değer yok sayılıyor.");
              //XML_STABIL_S = "US"; // Unstabil durum
              return average;
          }
      } else {
          outlierCount = 1; // Yeni bir trend başlıyor
          lastWasHigh = newValue > average;
          //XML_STABIL_S = "US"; // Unstabil durum
          return average;
      }
  } else {
      outlierCount = 0; // Aykırı değilse sıfırla
  }

  // FIFO tam dolduysa en eski değeri çıkar
  if (numValues == fifoSize) {
      sum -= buffer[index];
  } else {
      numValues++;
  }

  // Yeni değeri FIFO'ya ekle
  buffer[index] = newValue;
  sum += newValue;

  // Döngüsel tampon için indeksi güncelle
  index = (index + 1) % fifoSize;

  // Stabilite kontrolü
  if (isStable(newValue, average, stabilityThreshold)) {
      XML_STABIL_S = "ST"; // Stabil durum
      return sum / numValues;
  } else {
      XML_STABIL_S = "US"; // Unstabil durum
      return average;  // Stabil değilse mevcut ortalamayı döndür
  }
}

float performWeighing(float rawWeight, int taksimatStep, float* buffer, int& index, int& numValues, int fifoSize, float& sum, float deviationThreshold, int stabilityThreshold, int decimalPlaces, int consecutiveThreshold) {
  // Ağırlığı FIFO'ya ekle ve filtrelenmiş ortalama hesapla
  float filteredWeight = updateMovingAverageWithStability(
      rawWeight, buffer, index, numValues, fifoSize, sum, deviationThreshold, stabilityThreshold, taksimatStep, consecutiveThreshold
  );

  // Ham ağırlığı taksimata yuvarla
  float roundedWeight = applyTaksimat(filteredWeight, taksimatStep, decimalPlaces);
  // Stabilize edilmiş ağırlığı döndür
  return roundedWeight;
}

boolean Plugin_140(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  //static byte connectionState = 0;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_140;

        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        //Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_140);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_140));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_140));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_140));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        //String strings[3];
        //LoadCustomTaskSettings(event->TaskIndex, strings, 3, CUSTOMTASK_STR_SIZE_P140);

        byte choice1 = IND_EKRAN_MOD;
        String options1[2];
        options1[0] = F("TARTIM");
        options1[1] = F("URUN HAFIZASI");
        int optionValues1[2];
        optionValues1[0] = 0;
        optionValues1[1] = 1;
        addFormSelector(F("EKRAN MODU"), F("plugin_140_ekran_mod"), 2, options1, optionValues1, choice1);
        IND_SIFIR_KAL_DEGERI = Settings.ind_sifir_kal_degeri;
        addFormTextBox(F("SIFIR KALIBRASYON DEGERI"), F("plugin_140_sifir_kal_deg"), String(IND_SIFIR_KAL_DEGERI, 0), IND_SIFIR_KAL_DEGERI_BUFF_SIZE_P140);
        IND_YUK_KAL_DEGERI = float(Settings.ind_yuk_kal_degeri);
        addFormTextBox(F("YUK KALIBRASYON DEGERI"), F("plugin_140_yuk_kal_deg"), String(IND_YUK_KAL_DEGERI, 8), IND_YUK_KAL_DEGERI_BUFF_SIZE_P140);

        addFormTextBox(F("YUK KAPASITE DEGERI"), F("plugin_140_kapasite_deg"), String(IND_KAPASITE, 8), IND_KAPASITE_DEGERI_BUFF_SIZE_P140);

        byte choice6 = IND_TAKSIMAT;
        String options6[6];
        options6[0] = F("1");
        options6[1] = F("2");
        options6[2] = F("5");
        options6[3] = F("10");
        options6[4] = F("20");
        options6[5] = F("50");
        int optionValues6[6];
        optionValues6[0] = 1;
        optionValues6[1] = 2;
        optionValues6[2] = 5;
        optionValues6[3] = 10;
        optionValues6[4] = 20;
        optionValues6[5] = 50;
        addFormSelector(F("TAKSiMAT"), F("plugin_140_taksimat_deg"), 6, options6, optionValues6, choice6);

        byte choice7 = IND_NOKTA;
        String options7[5];
        options7[0] = F("0");
        options7[1] = F("0.0");
        options7[2] = F("0.00");
        options7[3] = F("0.000");
        options7[4] = F("0.0000");
        int optionValues7[5];
        optionValues7[0] = 0;
        optionValues7[1] = 1;
        optionValues7[2] = 2;
        optionValues7[3] = 3;
        optionValues7[4] = 4;
        addFormSelector(F("NOKTA"), F("plugin_140_nokta_deg"), 5, options7, optionValues7, choice7);

        addFormNumericBox(F("FILTRE"), F("plugin_140_filtre_deg"), IND_FILTRE, 1, 65535);
        addFormNumericBox(F("HIZ"), F("plugin_140_hiz_deg"), IND_HIZ, 1, 65535);
        addFormNumericBox(F("KAYIT ARALIĞI"), F("plugin_140_kayit_timer"), IND_KAYIT_TIMER, 0, 65535);

        addFormCheckBox(F("Bartender prn"), F("plugin_140_bartender"), IND_Bartender);

#ifdef ESP32
        fs::File root = ESPEASY_FS.open(F("/"));
        fs::File file = root.openNextFile();
        int fileno = 0;
        while (file && fileno < 10) {
          if (!file.isDirectory()) {
            const String fname(file.name());
            if (fname.startsWith(F("/ind")) || fname.startsWith(F("ind"))) {
              //int count = getCacheFileCountFromFilename(fname);
              options2[fileno] += file.name();
              fileno++;
            }
          }
          file = root.openNextFile();
        }
#endif

        addFormCheckBox(F("TCP/MODBUS AKTİF"), F("plugin_140_tcpmodbus_aktif"), IND_TCPMODBUS_AKTiF);
        addFormCheckBox(F("UDP AKTiF"), F("plugin_140_udp_aktif"), IND_UDP_AKTiF);
        addFormCheckBox(F("TCP AKTiF"), F("plugin_140_tcp_aktif"), IND_TCP_AKTiF);
        addFormNumericBox(F("TCP PORT"), F("plugin_140_tcp_port"), IND_TCP_PORT, 1, 65535);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //char deviceTemplate[0][CUSTOMTASK_STR_SIZE_P140];

        //LoadTaskSettings(event->TaskIndex);
        IND_EKRAN_MOD = getFormItemInt(F("plugin_140_ekran_mod"));
        IND_SIFIR_KAL_DEGERI = getFormItemFloat(F("plugin_140_sifir_kal_deg"));
        Settings.ind_sifir_kal_degeri = IND_SIFIR_KAL_DEGERI;
        IND_YUK_KAL_DEGERI = getFormItemFloat(F("plugin_140_yuk_kal_deg"));
        Settings.ind_yuk_kal_degeri = IND_YUK_KAL_DEGERI;

        IND_KAPASITE = getFormItemFloat(F("plugin_140_kapasite_deg"));
        IND_TAKSIMAT = getFormItemInt(F("plugin_140_taksimat_deg"));
        IND_NOKTA = getFormItemInt(F("plugin_140_nokta_deg"));
        IND_FILTRE = getFormItemInt(F("plugin_140_filtre_deg"));
        IND_HIZ = getFormItemInt(F("plugin_140_hiz_deg"));
        IND_KAYIT_TIMER = getFormItemInt(F("plugin_140_kayit_timer"));
        IND_TCPMODBUS_AKTiF = isFormItemChecked(F("plugin_140_tcpmodbus_aktif"));
        IND_UDP_AKTiF = isFormItemChecked(F("plugin_140_udp_aktif"));
        IND_TCP_AKTiF = isFormItemChecked(F("plugin_140_tcp_aktif"));
        IND_TCP_PORT = getFormItemInt(F("plugin_140_tcp_port"));

        IND_Bartender = isFormItemChecked(F("plugin_140_bartender"));

        ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_140_tek_prn"));
        ExtraTaskSettings.TaskDevicePrint[1] = getFormItemInt(F("plugin_140_art_prn"));
        ExtraTaskSettings.TaskDevicePrint[2] = getFormItemInt(F("plugin_140_top_prn"));
        ExtraTaskSettings.TaskDevicePrint[3] = getFormItemInt(F("plugin_140_sd_prn"));
        options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.tek_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[1]].toCharArray(Settings.art_prn, options2[ExtraTaskSettings.TaskDevicePrint[1]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[2]].toCharArray(Settings.top_prn, options2[ExtraTaskSettings.TaskDevicePrint[2]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[3]].toCharArray(Settings.sd_prn, options2[ExtraTaskSettings.TaskDevicePrint[3]].length() + 1);

        //SaveCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemplate, sizeof(deviceTemplate));
        //SaveSettings();
        success = true;        
        break;
      }

    case PLUGIN_INIT:
      {
        //scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
        scale.reset();
        scale.begin();
        scale.setChannel(CHANNEL1);
        scale.setCalibration();
        switch (IND_NOKTA) {
          case 0: bol = 1; break;
          case 1: bol = 10; break;
          case 2: bol = 100; break;
          case 3: bol = 1000; break;
          case 4: bol = 10000; break;
        }
        /*if (fileExists(SERIDATA_IND)) {
          fs::File form = tryOpenFile(SERIDATA_IND, "r");
          tartimString_s = "";
          while (form.position() < form.size()) {
            tartimString_s += form.readStringUntil('\r');
            tartimString_s.trim();
          }
          form.close();
        }
        if (IND_TCP_PORT != 0) {
          sernetServer_140 = new WiFiServer(IND_TCP_PORT);
          sernetServer_140->begin();
          sernetServer_140->setNoDelay(true);
        }
        if (IND_TCPMODBUS_AKTiF)
          Mb_140.begin();*/

        for (int i = 0; i < fifoSize; i++) fifoBuffer[i] = 0.0f;

        //fifoBuffer[fifoSize] = {};     // FIFO tampon
        fifoIndex = 0;              // FIFO tamponundaki mevcut indeks
        numValues = 0;              // FIFO içindeki toplam değer sayısı
        fifoSum = 0;              // FIFO'daki değerlerin toplamı
        filterFactor = 0.7;       // Dinamik olarak değişecek başlangıç filtre katsayısı
        lastWeight = 0;            // Bir önceki ölçülen ağırlık
        lastTime = 0;     // Bir önceki ölçüm zamanı
        Settings.WebAPP = 140;
        success = true;
        break;
      }

      case PLUGIN_FIFTY_PER_SECOND:
      {      
        // Ham ağırlık değerini al
        //float rawWeight = float(float((scale.read() - (Settings.ind_sifir_kal_degeri + ind_sifir_degeri)) * Settings.ind_yuk_kal_degeri) + (XML_DARA_S.toFloat() * bol));
        float rawWeight = float(float((scale.readADCValue() - (Settings.ind_sifir_kal_degeri + ind_sifir_degeri)) * Settings.ind_yuk_kal_degeri) + (XML_DARA_S.toFloat() * bol));

        // Ağırlığı stabilize et
        float stabilizedWeight = performWeighing(
           rawWeight, int(IND_TAKSIMAT), fifoBuffer, fifoIndex, numValues, float(IND_FILTRE), fifoSum, float(IND_FILTRE), int(IND_HIZ), int(IND_NOKTA), int(IND_HIZ)
        );

        dtostrf((float(stabilizedWeight) / float(bol)), 8, IND_NOKTA, goster_net_c);
        dtostrf((float(ind_dara_degeri)), 8, IND_NOKTA, goster_dara_c);
        addLog(LOG_LEVEL_INFO, String(F("Taksimat: ")) + IND_TAKSIMAT + F(" rawData: ") + rawWeight + F(" filteredWeight: ") + stabilizedWeight);
        if (stabilizedWeight < 0)
          dtostrf(((stabilizedWeight / bol) + (-1 * ind_dara_degeri)), 8, IND_NOKTA, goster_brut_c);
        else
          dtostrf(((stabilizedWeight / bol) + ind_dara_degeri), 8, IND_NOKTA, goster_brut_c);
        XML_NET_S  = String(goster_net_c);
        dtostrf(XML_NET_S.toFloat(), 7, IND_NOKTA, XML_NET_C);
        XML_DARA_S = String(goster_dara_c);
        XML_BRUT_S = String(goster_brut_c);
        if ((indkaydet) && (XML_STABIL_S == "ST")) {
          if (IND_KAYIT_TIMER == 0)
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indkaydet");
          indkaydet = false;
        }
        else if ((!indkaydet) && (XML_STABIL_S == "US")) {
          indkaydet = true;
        }
        if ((IND_KAYIT_TIMER > 0) && (millis() > kayit_millis)) {
          ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indkaydet");
          kayit_millis = kayit_millis + IND_KAYIT_TIMER*1000;
        }
        success = true;
        break;
      }

      case PLUGIN_ONCE_A_SECOND:
      {
        serialPrintln(String(F("ST,GS,")) + XML_NET_S + F(" kg"));
        success = true;
        break;
      }

      case PLUGIN_WRITE:
      {
        if (Settings.UDPPort > 0) {
          //Serial1.println("BARKOD OKUNDU");
          //Serial.println("BARKOD OKUNDU");
          string.replace("\r","");
          string.replace("\n","");
          XML_QRKOD_S = string;
          String barkod = "scan";
          barkod += XML_QRKOD_S;
          barkod +="\r\n";
          //Serial1.print(barkod);
          //ExecuteCommand_all({EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek"});
          string = "";
        }
        success = true;
        break;
      }
  }
  return success;
}

/*void addSelector(const String& label, const String& name, const String options[], const int optionValues[], int choice, int count) {
    addFormSelector(label, name, count, options, optionValues, choice);
}*/

#endif  // USES_P140

#ifdef USES_P141

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

#include <Wire.h>

/*
#include <modbus.h>
#include <modbusDevice.h>
#include <modbusRegBank.h>
#include <modbusSlave.h>

modbusDevice regBank;
modbusSlave slave;
*/

#define RS232Baud 115200
#define RS232Format SERIAL_8N1

//#######################################################################################################
//#################################### Plugin 140: SCALE IND  #######################################
//#######################################################################################################

#define PLUGIN_140
#define PLUGIN_ID_140 140
#define PLUGIN_NAME_140 "Scale - IND"
#define PLUGIN_VALUENAME1_140 "NET"
#define PLUGIN_VALUENAME2_140 "DARA"
#define PLUGIN_VALUENAME3_140 "BRUT"

#ifdef ESP8266
const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 14;
#endif

#ifdef ESP32
#ifdef IND_PANO
const uint8_t LOADCELL_DOUT_PIN = 5;
const uint8_t LOADCELL_SCK_PIN = 17;
#endif

#ifdef IND
const int LOADCELL_DOUT_PIN = 25;
const int LOADCELL_SCK_PIN = 26;
#endif

#ifdef IND_GRAFIK
#include "Wire.h"  // Wire kütüphanesini bağlıyoruz.
#include "I2CKeyPad.h"

const uint8_t KEYPAD_ADDRESS = 0x27;

I2CKeyPad keyPad(KEYPAD_ADDRESS);

//char keymap[19] = "A7410852D963ECSWNF";  // N = NoKey, F = Fail
char keymap[19] = "SD0AW987C654E321NF";  // N = NoKey, F = Fail

String key_data = "";
#include <Arduino.h>
#include <U8g2lib.h>

#include <SPI.h>

U8G2_ST7920_192X32_F_SW_SPI u8g2(U8G2_R0, /* E1=*/16, /* rw=*/2, /* rs=*/15, /* reset=*/U8X8_PIN_NONE);
U8G2_ST7920_192X32_F_SW_SPI u8g2_1(U8G2_R0, /* E2=*/17, /* rw=*/2, /* rs=*/15, /* reset=*/U8X8_PIN_NONE);

const int LOADCELL_DOUT_PIN = 25;
const int LOADCELL_SCK_PIN = 26;

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

const unsigned char ENDUTEK_LOGO[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00,
  0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00,
  0x00, 0xfc, 0xff, 0x07, 0xfc, 0xff, 0x07, 0x00, 0xfe, 0xbf, 0x1f, 0x3e,
  0x00, 0xf8, 0xf1, 0xff, 0xff, 0x80, 0xff, 0x1f, 0xe0, 0x03, 0xf0, 0x1f,
  0x00, 0xfe, 0xff, 0x0f, 0xfe, 0xff, 0x1f, 0x80, 0xff, 0xbf, 0x1f, 0x3e,
  0x00, 0xf8, 0xf1, 0xff, 0xff, 0xe0, 0xff, 0x7f, 0xe0, 0x03, 0xf8, 0x0f,
  0x00, 0xff, 0xff, 0x1f, 0xfe, 0xff, 0x7f, 0xc0, 0xff, 0xbf, 0x1f, 0x3e,
  0x00, 0xf8, 0xf1, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xe0, 0x03, 0xfe, 0x03,
  0x80, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0xe0, 0xff, 0xbf, 0x1f, 0x3e,
  0x00, 0xf8, 0xf1, 0xff, 0xff, 0xf0, 0xff, 0xff, 0xe1, 0x03, 0xff, 0x01,
  0x80, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0xe0, 0xff, 0xbf, 0x1f, 0x3e,
  0x00, 0xf8, 0xf1, 0xff, 0xff, 0xf8, 0xff, 0xff, 0xe3, 0x83, 0xff, 0x00,
  0x80, 0x0f, 0x00, 0x3e, 0x7e, 0x00, 0xfe, 0xe1, 0x07, 0x80, 0x1f, 0x3e,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0x00, 0xf0, 0xe3, 0xe3, 0x3f, 0x00,
  0x80, 0x0f, 0x00, 0x3e, 0x7e, 0x00, 0xfc, 0xe1, 0x07, 0x80, 0x1f, 0x3e,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0x00, 0xe0, 0xe3, 0xe3, 0x1f, 0x00,
  0x80, 0x0f, 0x00, 0x3e, 0x7e, 0x00, 0xf8, 0xe1, 0x03, 0x80, 0x1f, 0x3e,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0x00, 0xf0, 0xe3, 0xf3, 0x0f, 0x00,
  0x80, 0x0f, 0x00, 0x3f, 0x7e, 0x00, 0xf8, 0xe1, 0x03, 0x80, 0x1f, 0x3e,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0x00, 0xf0, 0xe3, 0xfb, 0x07, 0x00,
  0x80, 0xef, 0xff, 0x3f, 0x7e, 0x00, 0xf8, 0xe1, 0x03, 0x80, 0x1f, 0x3e,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0xfc, 0xff, 0xe3, 0xfb, 0x03, 0x00,
  0x80, 0xef, 0xff, 0x3f, 0x7e, 0x00, 0xf0, 0xe1, 0x03, 0x80, 0x1f, 0x3e,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0xfc, 0xff, 0xe1, 0xfb, 0x07, 0x00,
  0x80, 0xef, 0xff, 0x1f, 0x7e, 0x00, 0xf0, 0xe1, 0x03, 0x80, 0x1f, 0x3e,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0xfc, 0xff, 0xe0, 0xf3, 0x0f, 0x00,
  0x80, 0xef, 0xff, 0x0f, 0x7e, 0x00, 0xf0, 0xe1, 0x03, 0x80, 0x1f, 0x7e,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0xfc, 0x7f, 0xe0, 0xe3, 0x1f, 0x00,
  0x80, 0xef, 0xff, 0x07, 0x7e, 0x00, 0xf0, 0xe1, 0x03, 0x80, 0x1f, 0x7e,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0xfc, 0x3f, 0xe0, 0xc3, 0x3f, 0x00,
  0x80, 0x0f, 0x00, 0x00, 0x7e, 0x00, 0xf0, 0xe1, 0x07, 0x80, 0x1f, 0xfe,
  0x00, 0xf8, 0x01, 0xfc, 0x00, 0xf8, 0x00, 0x00, 0xe0, 0x83, 0x7f, 0x00,
  0x80, 0x1f, 0x00, 0x00, 0x7e, 0x00, 0xf0, 0xe1, 0xff, 0xff, 0x1f, 0xfe,
  0xff, 0xff, 0x01, 0xfc, 0x00, 0xf8, 0xff, 0x03, 0xe0, 0x03, 0xff, 0x00,
  0x80, 0xff, 0x7f, 0x00, 0x7e, 0x00, 0xf0, 0xe1, 0xff, 0xff, 0x0f, 0xfc,
  0xff, 0xff, 0x01, 0xfc, 0x00, 0xf8, 0xff, 0x07, 0xe0, 0x03, 0xfe, 0x01,
  0x00, 0xff, 0x7f, 0x00, 0x7e, 0x00, 0xf0, 0xe1, 0xff, 0xff, 0x0f, 0xfc,
  0xff, 0xff, 0x01, 0xfc, 0x00, 0xf0, 0xff, 0x07, 0xe0, 0x03, 0xf8, 0x03,
  0x00, 0xff, 0x7f, 0x00, 0x7e, 0x00, 0xf0, 0xc1, 0xff, 0xff, 0x0f, 0xf8,
  0xff, 0xff, 0x01, 0xfc, 0x00, 0xf0, 0xff, 0x07, 0xe0, 0x03, 0xf8, 0x07,
  0x00, 0xfe, 0x7f, 0x00, 0x7e, 0x00, 0xf0, 0x81, 0xff, 0xff, 0x07, 0xf0,
  0xff, 0xff, 0x01, 0xfc, 0x00, 0xe0, 0xff, 0x07, 0xe0, 0x03, 0xf0, 0x0f,
  0x00, 0xfc, 0x7f, 0x00, 0x7e, 0x00, 0xf0, 0x01, 0xff, 0xff, 0x01, 0xc0,
  0xff, 0xff, 0x00, 0xfc, 0x00, 0x80, 0xff, 0x07, 0xe0, 0x03, 0xe0, 0x1f,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03,
  0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x80, 0x00, 0x10, 0x00, 0x08, 0x18, 0x30, 0x00, 0x00, 0x06, 0x0c, 0x03,
  0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x80, 0x00, 0x10, 0x00, 0x08, 0x10, 0x30, 0x00, 0x00, 0x06, 0x0c, 0x02,
  0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x10, 0x00, 0x08, 0x00, 0x30, 0x00, 0x00, 0x00, 0x0c, 0x00,
  0x00, 0x00, 0x00, 0x08, 0x03, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00, 0x00,
  0x80, 0x3e, 0x9f, 0xec, 0xbf, 0x97, 0x37, 0x26, 0x7a, 0xf6, 0x7d, 0xf2,
  0xfd, 0x7c, 0xbe, 0x8f, 0xef, 0x79, 0x9f, 0xef, 0x93, 0xef, 0x9f, 0x01,
  0x80, 0x66, 0x93, 0x6c, 0x88, 0x11, 0x36, 0x76, 0xcf, 0xb6, 0xcd, 0xb2,
  0x6d, 0x48, 0xb2, 0x09, 0x33, 0x0d, 0x9b, 0x6d, 0xd6, 0x6c, 0xdb, 0x00,
  0x80, 0x66, 0x91, 0xec, 0x89, 0x90, 0x37, 0xf4, 0xff, 0xf6, 0xcd, 0x12,
  0x6d, 0x78, 0xf2, 0x08, 0xf3, 0x0d, 0x93, 0x2c, 0xd6, 0x68, 0xd3, 0x00,
  0x80, 0x66, 0x93, 0x8c, 0x89, 0xd0, 0x35, 0xdc, 0x0d, 0xf6, 0xcc, 0x12,
  0x3d, 0x5c, 0xf2, 0x08, 0x33, 0x0c, 0x93, 0x2c, 0xd6, 0xe8, 0x71, 0x00,
  0x80, 0x66, 0x9b, 0x2c, 0x99, 0xd0, 0x36, 0xdc, 0x09, 0x36, 0xcc, 0x12,
  0x0d, 0x6c, 0xb2, 0x0c, 0x33, 0x5c, 0x93, 0x6c, 0x96, 0x6c, 0x70, 0x00,
  0x80, 0x66, 0x9f, 0xef, 0xb9, 0xd0, 0x37, 0x98, 0xf9, 0xf6, 0xcd, 0x12,
  0x7d, 0xfc, 0xb2, 0x0f, 0xef, 0x79, 0x93, 0xec, 0x93, 0xef, 0x63, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00,
  0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x36, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x00,
  0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x33, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif
#endif

#define CUSTOMTASK_STR_SIZE_P140 40

#define IND_SIFIR_KAL_DEGERI_ADDR_SIZE_P140 9
#define IND_YUK_KAL_DEGERI_ADDR_SIZE_P140 9
#define IND_KAPASITE_DEGERI_ADDR_SIZE_P140 9

#define IND_SIFIR_KAL_DEGERI_BUFF_SIZE_P140 9
#define IND_YUK_KAL_DEGERI_BUFF_SIZE_P140 9
#define IND_KAPASITE_DEGERI_BUFF_SIZE_P140 9

#define IND_SIFIR_KAL_DEGERI PCONFIG_FLOAT(0)
#define IND_YUK_KAL_DEGERI PCONFIG_FLOAT(1)
#define IND_KAPASITE PCONFIG_FLOAT(2)

#define IND_EKRAN_MOD PCONFIG(0)
#define IND_TAKSIMAT PCONFIG(1)
#define IND_NOKTA PCONFIG(2)
#define IND_FILTRE PCONFIG(3)
#define IND_HIZ PCONFIG(4)
#define IND_UDP_AKTiF PCONFIG(5)
#define IND_TCP_AKTiF PCONFIG(6)
#define IND_TCP_PORT PCONFIG(7)
#define IND_TCPMODBUS_AKTiF PCONFIG(8)

#define IND_Bartender ExtraTaskSettings.TaskPrintBartender

char goster_net_c[10];
char goster_dara_c[10];
char goster_brut_c[10];

int ind_stabil_sayisi = 0;
float bol = 0;
float ind_tartim_degeri_son = 0;
float ind_tartim_degeri_son1 = 0;
String file_data = "";

bool output_1_aktif = true;
bool output_2_aktif = true;
bool output_3_aktif = true;
bool output_4_aktif = true;

WiFiServer *sernetServer_140;
WiFiClient sernetClients_140;  //[MAX_SRV_CLIENTS];

float ind_sum_f_d[254];
float ind_sum_f = 0;
uint8_t ind_i = 0;

#include "ModbusTCPSlave.h"
ModbusTCPSlave Mb_140;

boolean Plugin_140(byte function, struct EventStruct *event, String &string) {
  boolean success = false;
  static byte connectionState = 0;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_140;

        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        //Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_SINGLE;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 3;
        Device[deviceCount].SendDataOption = false;
        Device[deviceCount].TimerOption = false;
        Device[deviceCount].GlobalSyncOption = false;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_140);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_140));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_140));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_140));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        //String strings[3];
        //LoadCustomTaskSettings(event->TaskIndex, strings, 3, CUSTOMTASK_STR_SIZE_P140);

        byte choice1 = IND_EKRAN_MOD;
        String options1[2];
        options1[0] = F("TARTIM");
        options1[1] = F("URUN HAFIZASI");
        int optionValues1[2];
        optionValues1[0] = 0;
        optionValues1[1] = 1;
        addFormSelector(F("EKRAN MODU"), F("plugin_140_ekran_mod"), 2, options1, optionValues1, choice1);
        IND_SIFIR_KAL_DEGERI = long(Settings.ind_sifir_kal_degeri);
        addFormTextBox(F("SIFIR KALIBRASYON DEGERI"), F("plugin_140_sifir_kal_deg"), String(IND_SIFIR_KAL_DEGERI, 0), IND_SIFIR_KAL_DEGERI_BUFF_SIZE_P140);
        IND_YUK_KAL_DEGERI = float(Settings.ind_yuk_kal_degeri);
        addFormTextBox(F("YUK KALIBRASYON DEGERI"), F("plugin_140_yuk_kal_deg"), String(IND_YUK_KAL_DEGERI, 8), IND_YUK_KAL_DEGERI_BUFF_SIZE_P140);

        addFormTextBox(F("YUK KAPASITE DEGERI"), F("plugin_140_kapasite_deg"), String(IND_KAPASITE, 8), IND_KAPASITE_DEGERI_BUFF_SIZE_P140);

        byte choice6 = IND_TAKSIMAT;
        String options6[6];
        options6[0] = F("1");
        options6[1] = F("2");
        options6[2] = F("5");
        options6[3] = F("10");
        options6[4] = F("20");
        options6[5] = F("50");
        int optionValues6[6];
        optionValues6[0] = 1;
        optionValues6[1] = 2;
        optionValues6[2] = 5;
        optionValues6[3] = 10;
        optionValues6[4] = 20;
        optionValues6[5] = 50;
        addFormSelector(F("TAKSiMAT"), F("plugin_140_taksimat_deg"), 6, options6, optionValues6, choice6);

        byte choice7 = IND_NOKTA;
        String options7[5];
        options7[0] = F("0");
        options7[1] = F("0.0");
        options7[2] = F("0.00");
        options7[3] = F("0.000");
        options7[4] = F("0.0000");
        int optionValues7[5];
        optionValues7[0] = 0;
        optionValues7[1] = 1;
        optionValues7[2] = 2;
        optionValues7[3] = 3;
        optionValues7[4] = 4;
        addFormSelector(F("NOKTA"), F("plugin_140_nokta_deg"), 5, options7, optionValues7, choice7);

        addFormNumericBox(F("FILTRE"), F("plugin_140_filtre_deg"), IND_FILTRE, 1, 65535);
        addFormNumericBox(F("HIZ"), F("plugin_140_hiz_deg"), IND_HIZ, 1, 65535);

        addFormCheckBox(F("Bartender prn"), F("plugin_130_bartender"), IND_Bartender);

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
        int optionValues2[10];
        optionValues2[0] = 0;
        optionValues2[1] = 1;
        optionValues2[2] = 2;
        optionValues2[3] = 3;
        optionValues2[4] = 4;
        optionValues2[5] = 5;
        optionValues2[6] = 6;
        optionValues2[7] = 7;
        optionValues2[8] = 8;
        optionValues2[9] = 9;
        addFormSelector(F("Tek Etiket"), F("plugin_140_tek_prn"), 10, options2, optionValues2, choice2);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[0]], F("Etiket Dizayn Menüsüne Git"));

        byte choice3 = ExtraTaskSettings.TaskDevicePrint[1];
        addFormSelector(F("Artı Etiket"), F("plugin_140_art_prn"), 10, options2, optionValues2, choice3);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[1]], F("Etiket Dizayn Menüsüne Git"));

        byte choice4 = ExtraTaskSettings.TaskDevicePrint[2];
        addFormSelector(F("Toplam Etiket"), F("plugin_140_top_prn"), 10, options2, optionValues2, choice4);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[2]], F("Etiket Dizayn Menüsüne Git"));

#if FEATURE_SD
        byte choice5 = ExtraTaskSettings.TaskDevicePrint[3];
        addFormSelector(F("SD data"), F("plugin_140_sd_prn"), 10, options2, optionValues2, choice5);
        addButton(options2[ExtraTaskSettings.TaskDevicePrint[3]], F("SD Data Dizayn Menüsüne Git"));
#endif

        addFormCheckBox(F("TCP/MODBUS AKTİF"), F("plugin_140_tcpmodbus_aktif"), IND_TCPMODBUS_AKTiF);
        addFormCheckBox(F("UDP AKTiF"), F("plugin_140_udp_aktif"), IND_UDP_AKTiF);
        addFormCheckBox(F("TCP AKTiF"), F("plugin_140_tcp_aktif"), IND_TCP_AKTiF);
        addFormNumericBox(F("TCP PORT"), F("plugin_140_tcp_port"), IND_TCP_PORT, 1, 65535);

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        //char deviceTemplate[0][CUSTOMTASK_STR_SIZE_P140];

        //LoadTaskSettings(event->TaskIndex);
        IND_EKRAN_MOD = getFormItemInt(F("plugin_140_ekran_mod"));
        PCONFIG_FLOAT(0) = getFormItemFloat(F("plugin_140_sifir_kal_deg"));
        Settings.ind_sifir_kal_degeri = PCONFIG_FLOAT(0);
        PCONFIG_FLOAT(1) = getFormItemFloat(F("plugin_140_yuk_kal_deg"));
        Settings.ind_yuk_kal_degeri = PCONFIG_FLOAT(1);

        IND_KAPASITE = getFormItemFloat(F("plugin_140_kapasite_deg"));
        IND_TAKSIMAT = getFormItemInt(F("plugin_140_taksimat_deg"));
        IND_NOKTA = getFormItemInt(F("plugin_140_nokta_deg"));
        IND_FILTRE = getFormItemInt(F("plugin_140_filtre_deg"));
        IND_HIZ = getFormItemInt(F("plugin_140_hiz_deg"));
        IND_TCPMODBUS_AKTiF = isFormItemChecked(F("plugin_140_tcpmodbus_aktif"));
        IND_UDP_AKTiF = isFormItemChecked(F("plugin_140_udp_aktif"));
        IND_TCP_AKTiF = isFormItemChecked(F("plugin_140_tcp_aktif"));
        IND_TCP_PORT = getFormItemInt(F("plugin_140_tcp_port"));

        IND_Bartender = isFormItemChecked(F("plugin_140_bartender"));

        ExtraTaskSettings.TaskDevicePrint[0] = getFormItemInt(F("plugin_140_tek_prn"));
        ExtraTaskSettings.TaskDevicePrint[1] = getFormItemInt(F("plugin_140_art_prn"));
        ExtraTaskSettings.TaskDevicePrint[2] = getFormItemInt(F("plugin_140_top_prn"));
        ExtraTaskSettings.TaskDevicePrint[3] = getFormItemInt(F("plugin_140_sd_prn"));
        options2[ExtraTaskSettings.TaskDevicePrint[0]].toCharArray(Settings.tek_prn, options2[ExtraTaskSettings.TaskDevicePrint[0]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[1]].toCharArray(Settings.art_prn, options2[ExtraTaskSettings.TaskDevicePrint[1]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[2]].toCharArray(Settings.top_prn, options2[ExtraTaskSettings.TaskDevicePrint[2]].length() + 1);
        options2[ExtraTaskSettings.TaskDevicePrint[3]].toCharArray(Settings.sd_prn, options2[ExtraTaskSettings.TaskDevicePrint[3]].length() + 1);

        //SaveCustomTaskSettings(event->TaskIndex, (byte *)&deviceTemplate, sizeof(deviceTemplate));
        ind_sum_f_d[254];
        ind_sum_f = 0;
        ind_i = 0;
        //SaveSettings();
        success = true;        
        break;
      }

    case PLUGIN_INIT:
      {
        scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
        /*regBank.setId(1);
        regBank.add(10001);  //R1 HIZLI_1 RÖLE DURUMU
        regBank.add(10002);  //R2 HIZLI_2 RÖLE DURUMU
        regBank.add(10003);  //R3 SEHPA RÖLE DURUMU
        regBank.add(10004);  //R4 RÖLE

        regBank.add(40001);  //TARTI_1 NET TARTIM DEĞERİ LOW
        regBank.add(40002);  //TARTI_1 NET TARTIM DEĞERİ HIGH
        regBank.add(40003);  //TARTI_2 DARA TARTIM DEĞERİ LOW
        regBank.add(40004);  //TARTI_2 DARA TARTIM DEĞERİ HIGH
        regBank.add(40005);  //TARTI_2 BRUT TARTIM DEĞERİ LOW
        regBank.add(40006);  //TARTI_2 BRUT TARTIM DEĞERİ HIGH
        regBank.add(40008);  //TARTI_2 İÇ SAYIM TARTIM DEĞERİ LOW
        regBank.add(40009);  //TARTI_2 İÇ SAYIM TARTIM DEĞERİ HIGH

        regBank.add(40010);  //TARTI_1 URUN_1 HIZLI DEĞERİ LOW
        regBank.add(40011);  //TARTI_1 URUN_1 HIZLI DEĞERİ HIGH
        regBank.add(40012);  //TARTI_1 URUN_1 YAVAS DEĞERİ LOW
        regBank.add(40013);  //TARTI_1 URUN_1 YAVAS DEĞERİ HIGH

        regBank.add(40051);  //URUN SECİMİ
        regBank.add(40052);  //URUN HEDEF KAYDET
        regBank.add(40053);  //URUN HEDEF KAYDET
        regBank.add(40054);  //DARA SIFIR

        regBank.add(40060);  //TARTI_1 AKTİF
        regBank.add(40061);  //TARTI_1 KAPASİTE LOW
        regBank.add(40062);  //TARTI_1 KAPASİTE HIGH
        regBank.add(40063);  //TARTI_1 KALİBRASYON KİLOSU LOW
        regBank.add(40064);  //TARTI_1 KALİBRASYON KİLOSU HIGH
        regBank.add(40065);  //TARTI_1 İÇ SAYIM DEĞERİ LOW
        regBank.add(40066);  //TARTI_1 İÇ SAYIM DEĞERİ HIGH
        regBank.add(40067);  //TARTI_1 KAPASİTE KAYDET
        regBank.add(40068);  //TARTI_1 SIFIR DEĞERİ KAYDET
        regBank.add(40069);  //TARTI_1 YÜK KALİBASYONU KAYDET
        regBank.add(40070);  //TARTI_1 TARTIM HIZI
        regBank.add(40071);  //TARTI_1 TAKSİMAT

        regBank.add(40080);  //TARTI_2 AKTİF
        regBank.add(40081);  //TARTI_2 KAPASİTE LOW
        regBank.add(40082);  //TARTI_2 KAPASİTE HIGH
        regBank.add(40083);  //TARTI_2 KALİBRASYON KİLOSU LOW
        regBank.add(40084);  //TARTI_2 KALİBRASYON KİLOSU HIGH
        regBank.add(40085);  //TARTI_2 İÇ SAYIM DEĞERİ LOW
        regBank.add(40086);  //TARTI_2 İÇ SAYIM DEĞERİ HIGH
        regBank.add(40087);  //TARTI_2 KAPASİTE KAYDET
        regBank.add(40088);  //TARTI_2 SIFIR DEĞERİ KAYDET
        regBank.add(40089);  //TARTI_2 YÜK KALİBASYONU KAYDET
        regBank.add(40090);  //TARTI_2 TARTIM HIZI
        regBank.add(40091);  //TARTI_2 TAKSİMAT

        regBank.add(40100);  //HIZLI_1 RÖLE GECİKME
        regBank.add(40101);  //HIZLI_2 RÖLE GECİKME
        regBank.add(40102);  //SEHPA RÖLE GECİKME
        regBank.add(40103);  //DARA BUTONU SIFIR BUTONU
        regBank.add(40104);  //DARA AKTİF
        regBank.add(40109);  //ROLE AYAR KAYDET

        regBank.add(40110);  //START STOP BUTON
        slave._device = &regBank;
        slave.setBaud(&Serial, RS232Baud, RS232Format);*/
        switch (IND_NOKTA) {
          case 0: bol = 1; break;
          case 1: bol = 10; break;
          case 2: bol = 100; break;
          case 3: bol = 1000; break;
          case 4: bol = 10000; break;
        }
        if (fileExists(SERIDATA_IND)) {
          fs::File form = tryOpenFile(SERIDATA_IND, "r");
          tartimString_s = "";
          while (form.position() < form.size()) {
            tartimString_s += form.readStringUntil('\r');
            tartimString_s.trim();
          }
          form.close();
        }
#ifdef IND_PANO
        pcf8574.pinMode(P0, OUTPUT, LOW);
        pcf8574.pinMode(P1, OUTPUT, LOW);
        pcf8574.pinMode(P2, OUTPUT, LOW);
        pcf8574.pinMode(P3, OUTPUT, LOW);
        pcf8574.pinMode(P4, INPUT_PULLUP);
        pcf8574.pinMode(P5, INPUT_PULLUP);
        pcf8574.pinMode(P6, INPUT_PULLUP);
        pcf8574.pinMode(P7, INPUT_PULLUP);
        pcf8574.digitalWrite(P0, LOW);
        pcf8574.digitalWrite(P1, LOW);
        pcf8574.digitalWrite(P2, LOW);
        pcf8574.digitalWrite(P3, LOW);
        //XML_INPUT_PIN_C  = {'0','0','0','0'};
        XML_OUTPUT_PIN_C[0] = '0';
        XML_OUTPUT_PIN_C[1] = '0';
        XML_OUTPUT_PIN_C[2] = '0';
        XML_OUTPUT_PIN_C[3] = '0';
        pcf8574.begin();
#endif
#ifdef IND_GRAFIK
        u8g2.begin();
        u8g2_1.begin();

        u8g2.clearBuffer();
        u8g2_1.clearBuffer();
        u8g2.drawXBMP(0, 0, 192, 68, ENDUTEK_LOGO);
        u8g2.sendBuffer();
        u8g2_1.drawXBMP(0, -32, 192, 68, ENDUTEK_LOGO);
        u8g2_1.sendBuffer();
        delay(5000);

        u8g2.clearBuffer();
        u8g2.sendBuffer();
        u8g2_1.clearBuffer();
        u8g2_1.sendBuffer();

        u8g2_1.setFont(u8g2_font_ncenB12_tr);
        u8g2_1.drawStr(10, 20, Settings.Name);  // write something to the internal memory
        u8g2_1.sendBuffer();
        delay(2000);
        keyPad.begin();
        keyPad.loadKeyMap(keymap);
        u8g2.clearBuffer();
        u8g2_1.clearBuffer();
#endif
        if (IND_TCP_PORT != 0) {
          sernetServer_140 = new WiFiServer(IND_TCP_PORT);
          sernetServer_140->begin();
          sernetServer_140->setNoDelay(true);
        }
        if (IND_TCPMODBUS_AKTiF)
          Mb_140.begin();
        Settings.WebAPP = 140;
        success = true;
        break;
      }

      /*case PLUGIN_FIFTY_PER_SECOND:
      {
        long ind_tartim_degeri = long(float((scale.read() - (Settings.ind_sifir_kal_degeri + ind_sifir_degeri)) * Settings.ind_yuk_kal_degeri) + (XML_DARA_S.toFloat() * bol));
        if (int(IND_TAKSIMAT) == 1) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
          }
        } else if (int(IND_TAKSIMAT) == 2) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 5) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 4: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 10) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 9: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 8: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 7: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case 6: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case 5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case 4: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case -5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case -6: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case -7: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case -8: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -9: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 20) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 19: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 18: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 17: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case 16: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case 15: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case 14: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case 13: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case 12: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case 11: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case 10: ind_tartim_degeri_son = ind_tartim_degeri + 10; break;
            case 9: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case 8: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case 7: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case 6: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case 5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case 4: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case -5: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case -6: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case -7: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case -8: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case -9: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case -10: ind_tartim_degeri_son = ind_tartim_degeri - 10; break;
            case -11: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case -12: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case -13: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case -14: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case -15: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case -16: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case -17: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case -18: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -19: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 20) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 19: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 18: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 17: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case 16: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case 15: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case 14: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case 13: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case 12: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case 11: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case 10: ind_tartim_degeri_son = ind_tartim_degeri + 10; break;
            case 9: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case 8: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case 7: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case 6: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case 5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case 4: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case -5: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case -6: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case -7: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case -8: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case -9: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case -10: ind_tartim_degeri_son = ind_tartim_degeri - 10; break;
            case -11: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case -12: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case -13: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case -14: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case -15: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case -16: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case -17: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case -18: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -19: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 20) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 49: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 48: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 47: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case 46: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case 45: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case 44: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case 43: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case 42: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case 41: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case 40: ind_tartim_degeri_son = ind_tartim_degeri + 10; break;
            case 39: ind_tartim_degeri_son = ind_tartim_degeri + 11; break;
            case 38: ind_tartim_degeri_son = ind_tartim_degeri + 12; break;
            case 37: ind_tartim_degeri_son = ind_tartim_degeri + 13; break;
            case 36: ind_tartim_degeri_son = ind_tartim_degeri + 14; break;
            case 35: ind_tartim_degeri_son = ind_tartim_degeri + 15; break;
            case 34: ind_tartim_degeri_son = ind_tartim_degeri + 16; break;
            case 33: ind_tartim_degeri_son = ind_tartim_degeri + 17; break;
            case 32: ind_tartim_degeri_son = ind_tartim_degeri + 18; break;
            case 31: ind_tartim_degeri_son = ind_tartim_degeri + 19; break;
            case 30: ind_tartim_degeri_son = ind_tartim_degeri + 20; break;
            case 29: ind_tartim_degeri_son = ind_tartim_degeri + 21; break;
            case 28: ind_tartim_degeri_son = ind_tartim_degeri + 22; break;
            case 27: ind_tartim_degeri_son = ind_tartim_degeri + 23; break;
            case 26: ind_tartim_degeri_son = ind_tartim_degeri + 24; break;
            case 25: ind_tartim_degeri_son = ind_tartim_degeri - 25; break;
            case 24: ind_tartim_degeri_son = ind_tartim_degeri - 24; break;
            case 23: ind_tartim_degeri_son = ind_tartim_degeri - 23; break;
            case 22: ind_tartim_degeri_son = ind_tartim_degeri - 22; break;
            case 21: ind_tartim_degeri_son = ind_tartim_degeri - 21; break;
            case 20: ind_tartim_degeri_son = ind_tartim_degeri - 20; break;
            case 19: ind_tartim_degeri_son = ind_tartim_degeri - 19; break;
            case 18: ind_tartim_degeri_son = ind_tartim_degeri - 18; break;
            case 17: ind_tartim_degeri_son = ind_tartim_degeri - 17; break;
            case 16: ind_tartim_degeri_son = ind_tartim_degeri - 16; break;
            case 15: ind_tartim_degeri_son = ind_tartim_degeri - 15; break;
            case 14: ind_tartim_degeri_son = ind_tartim_degeri - 14; break;
            case 13: ind_tartim_degeri_son = ind_tartim_degeri - 13; break;
            case 12: ind_tartim_degeri_son = ind_tartim_degeri - 12; break;
            case 11: ind_tartim_degeri_son = ind_tartim_degeri - 11; break;
            case 10: ind_tartim_degeri_son = ind_tartim_degeri - 10; break;
            case 9: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case 8: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case 7: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case 6: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case 5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case 4: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case -5: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case -6: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case -7: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case -8: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case -9: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case -10: ind_tartim_degeri_son = ind_tartim_degeri + 10; break;
            case -11: ind_tartim_degeri_son = ind_tartim_degeri + 11; break;
            case -12: ind_tartim_degeri_son = ind_tartim_degeri + 12; break;
            case -13: ind_tartim_degeri_son = ind_tartim_degeri + 13; break;
            case -14: ind_tartim_degeri_son = ind_tartim_degeri + 14; break;
            case -15: ind_tartim_degeri_son = ind_tartim_degeri + 15; break;
            case -16: ind_tartim_degeri_son = ind_tartim_degeri + 16; break;
            case -17: ind_tartim_degeri_son = ind_tartim_degeri + 17; break;
            case -18: ind_tartim_degeri_son = ind_tartim_degeri + 18; break;
            case -19: ind_tartim_degeri_son = ind_tartim_degeri + 19; break;
            case -20: ind_tartim_degeri_son = ind_tartim_degeri + 20; break;
            case -21: ind_tartim_degeri_son = ind_tartim_degeri + 21; break;
            case -22: ind_tartim_degeri_son = ind_tartim_degeri + 22; break;
            case -23: ind_tartim_degeri_son = ind_tartim_degeri + 23; break;
            case -24: ind_tartim_degeri_son = ind_tartim_degeri + 24; break;
            case -25: ind_tartim_degeri_son = ind_tartim_degeri - 25; break;
            case -26: ind_tartim_degeri_son = ind_tartim_degeri - 24; break;
            case -27: ind_tartim_degeri_son = ind_tartim_degeri - 23; break;
            case -28: ind_tartim_degeri_son = ind_tartim_degeri - 22; break;
            case -29: ind_tartim_degeri_son = ind_tartim_degeri - 21; break;
            case -30: ind_tartim_degeri_son = ind_tartim_degeri - 20; break;
            case -31: ind_tartim_degeri_son = ind_tartim_degeri - 19; break;
            case -32: ind_tartim_degeri_son = ind_tartim_degeri - 18; break;
            case -33: ind_tartim_degeri_son = ind_tartim_degeri - 17; break;
            case -34: ind_tartim_degeri_son = ind_tartim_degeri - 16; break;
            case -35: ind_tartim_degeri_son = ind_tartim_degeri - 15; break;
            case -36: ind_tartim_degeri_son = ind_tartim_degeri - 14; break;
            case -37: ind_tartim_degeri_son = ind_tartim_degeri - 13; break;
            case -38: ind_tartim_degeri_son = ind_tartim_degeri - 12; break;
            case -39: ind_tartim_degeri_son = ind_tartim_degeri - 11; break;
            case -40: ind_tartim_degeri_son = ind_tartim_degeri - 10; break;
            case -41: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case -42: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case -43: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case -44: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case -45: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case -46: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case -47: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case -48: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -49: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        }
        dtostrf((ind_tartim_degeri_son / bol), 8, IND_NOKTA, goster_net_c);
        dtostrf((ind_dara_degeri), 8, IND_NOKTA, goster_dara_c);
        if (ind_dara_degeri < 0)
          dtostrf(((ind_tartim_degeri_son / bol) + (-1 * ind_dara_degeri)), 8, IND_NOKTA, goster_brut_c);
        else
          dtostrf(((ind_tartim_degeri_son / bol) + ind_dara_degeri), 8, IND_NOKTA, goster_brut_c);
        XML_NET_S = String(goster_net_c);
        XML_DARA_S = String(goster_dara_c);
        XML_BRUT_S = String(goster_brut_c);
        if (millis() > stabilTimer_l) {
          stabilTimer_l = millis() + 200;
          stabilTartim_s = XML_NET_S;
          if (ind_stabil_sayisi >= 2) {
            ind_stabil_sayisi = 0;
            XML_STABIL_S = "ST";
          } else
            XML_STABIL_S = "US";
        } else {
          if (stabilTartim_s == XML_NET_S)
            ind_stabil_sayisi++;
          else
            ind_stabil_sayisi = 0;
        }
        XML_TARIH_S = node_time.getDateString('-');
        XML_SAAT_S = node_time.getTimeString(':');
        dtostrf(XML_NET_S.toFloat(), (Settings.net_bitis_byte - Settings.net_bas_byte), IND_NOKTA, XML_NET_C);
        dtostrf(XML_DARA_S.toFloat(), (Settings.dara_bitis_byte - Settings.dara_bas_byte), IND_NOKTA, XML_DARA_C);
        dtostrf(XML_BRUT_S.toFloat(), (Settings.net_bitis_byte - Settings.net_bas_byte), IND_NOKTA, XML_BRUT_C);
        dtostrf(XML_ADET_S.toFloat(), (Settings.adet_bitis_byte - Settings.adet_bas_byte), 0, XML_ADET_C);
        file_data = tartimString_s;
        //file_data = string_convert(file_data);
        parseSystemVariables(file_data, false);
        file_data += "\r\n";
        //Serial.print(file_data);
#ifdef IND_GRAFIK
        SendUDPCommand(0, (const char *)file_data.c_str(), file_data.length());
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indseridata");
        if (IND_EKRAN_MOD == 0) {
          char batery[1] = { 54 };
          char yazici[1] = { 58 };
          //u8g2.setContrast(250);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_battery19_tn);
          u8g2.drawStr(1, 20, &batery[0]);
          //u8g2.sendBuffer();

          u8g2.setFont(u8g2_font_t0_11_tf);
          int wifi_bas = 190 - (WiFi.SSID().length() * 6);
          u8g2.drawStr(wifi_bas, 22, WiFi.SSID().c_str());


          long rssi = WiFi.RSSI();
          if (rssi >= -55) {
            u8g2.drawBox(177, 8, 2, 1);
            u8g2.drawBox(180, 7, 2, 2);
            u8g2.drawBox(183, 5, 2, 4);
            u8g2.drawBox(186, 3, 2, 6);
            u8g2.drawBox(189, 1, 2, 8);
          } else if (rssi< -55 & rssi > - 65) {
            u8g2.drawBox(177, 8, 2, 1);
            u8g2.drawBox(180, 7, 2, 2);
            u8g2.drawBox(183, 5, 2, 4);
            u8g2.drawBox(186, 3, 2, 6);
          } else if (rssi< -65 & rssi > - 75) {
            u8g2.drawBox(177, 8, 2, 1);
            u8g2.drawBox(180, 7, 2, 2);
            u8g2.drawBox(183, 5, 2, 4);
          } else if (rssi< -75 & rssi > - 85) {
            u8g2.drawBox(177, 8, 2, 1);
            u8g2.drawBox(180, 7, 2, 2);
          } else if (rssi< -85 & rssi > - 96) {
            u8g2.drawBox(177, 8, 2, 1);
          }
          u8g2.drawXBMP(15, 0, 15, 19, ETIKET_YAZICI);
          u8g2.drawXBMP(30, 0, 15, 19, FIS_YAZICI);

          u8g2.setFont(u8g2_font_logisoso38_tr);
          u8g2.drawStr(-7, 64, XML_NET_S.c_str());  // write something to the internal memory
          u8g2.sendBuffer();

          u8g2_1.clearBuffer();
          u8g2_1.setFont(u8g2_font_logisoso38_tr);
          u8g2_1.drawStr(-7, 32, XML_NET_S.c_str());  // write something to the internal memory
          u8g2_1.setFont(u8g2_font_ncenB10_tr);
          u8g2_1.drawStr(175, 26, "kg");
          u8g2_1.sendBuffer();
        } else {
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB14_tr);
          u8g2.drawStr(1, 32, XML_NET_S.c_str());  // write something to the internal memory
          u8g2.setFont(u8g2_font_ncenB10_tr);
          u8g2.drawStr(72, 28, "kg");

          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(20, 16, "NET");  // write something to the internal memory

          u8g2.setFont(u8g2_font_ncenB14_tr);
          u8g2.drawStr(96, 32, XML_DARA_S.c_str());  // write something to the internal memory
          u8g2.setFont(u8g2_font_ncenB10_tr);
          u8g2.drawStr(165, 28, "kg");

          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(120, 16, "DARA");  // write something to the internal memory
          u8g2.sendBuffer();

          u8g2_1.clearBuffer();
          u8g2_1.setFont(u8g2_font_ncenB08_tr);
          u8g2_1.drawStr(1, 10, "PLU NO:");
          u8g2_1.drawStr(100, 10, "KOD:");
          u8g2_1.drawStr(136, 10, XML_PLU_KOD_S.c_str());
          u8g2_1.drawStr(52, 10, key_data.c_str());
          u8g2_1.drawStr(5, 20, XML_PLU_ADI_S.c_str());
          u8g2_1.sendBuffer();
        }
        if (keyPad.isPressed()) {
          if ((keyPad.getChar() <= 57) && (keyPad.getChar() >= 48))
            key_data += char(keyPad.getChar());
          if (keyPad.getChar() == 'C')
            key_data = "";
          if (keyPad.getChar() == 'E')
            ind_dara_degeri = key_data.toFloat() / bol;
          if (keyPad.getChar() == 'D') {
            if (fileExists(FILE_PLU)) {
              int pluno = 0;
              fs::File form = tryOpenFile(FILE_PLU, "r");
              String s = "0";
              while (form.position() < form.size()) {
                s = form.readStringUntil('\n');
                if (s.indexOf(",") > 0) {
                  pluno++;
                  int say = s.indexOf(",");
                  if (pluno == key_data.toInt()) {
                    XML_PLU_ADI_S = s.substring(0, say);
                    XML_BARKOD_S = s.substring((say + 1), (say + 8));
                    XML_PLU_KOD_S = XML_BARKOD_S;
                  }
                  s = "";
                } else
                  s.trim();
              }
              form.close();
            }
          }
          if (keyPad.getChar() == 'W') {
            int pluno_click = key_data.toInt();
            pluno_click++;
            key_data = String(pluno_click);
            if (fileExists(FILE_PLU)) {
              int pluno = 0;
              fs::File form = tryOpenFile(FILE_PLU, "r");
              String s = "0";
              while (form.position() < form.size()) {
                s = form.readStringUntil('\n');
                if (s.indexOf(",") > 0) {
                  pluno++;
                  int say = s.indexOf(",");
                  if (pluno == key_data.toInt()) {
                    XML_PLU_ADI_S = s.substring(0, say);
                    XML_BARKOD_S = s.substring((say + 1), (say + 8));
                    XML_PLU_KOD_S = XML_BARKOD_S;
                  }
                  s = "";
                } else
                  s.trim();
              }
              form.close();
            }
          }
          if (keyPad.getChar() == 'S') {
            if (key_data.toInt() >= 0) {
              int pluno_click = key_data.toInt();
              pluno_click--;
              key_data = String(pluno_click);
            }
            if (fileExists(FILE_PLU)) {
              int pluno = 0;
              fs::File form = tryOpenFile(FILE_PLU, "r");
              String s = "0";
              while (form.position() < form.size()) {
                s = form.readStringUntil('\n');
                if (s.indexOf(",") > 0) {
                  pluno++;
                  int say = s.indexOf(",");
                  if (pluno == key_data.toInt()) {
                    XML_PLU_ADI_S = s.substring(0, say);
                    XML_BARKOD_S = s.substring((say + 1), (say + 8));
                    XML_PLU_KOD_S = XML_BARKOD_S;
                  }
                  s = "";
                } else
                  s.trim();
              }
              form.close();
            }
          }
          if (keyPad.getChar() == 'A') {
            String yaz_data = "eyzpluart#";
            yaz_data += key_data;
            yaz_data += "#1#1#1";
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, yaz_data.c_str());
            delay(300);
          }
          delay(200);
        }
#endif
        if (IND_TCP_AKTiF) {
          if (sernetServer_140->hasClient()) {
            if (sernetClients_140) { sernetClients_140.stop(); }
            sernetClients_140 = sernetServer_140->available();
          }
          if (sernetClients_140.connected()) {
            addLogMove(LOG_LEVEL_INFOS, "Client Bagli");
            connectionState = 1;
            int bytes_read = 0;
            uint8_t net_buf[32];
            int count = sernetClients_140.available();
            if (count > 0) {
              addLogMove(LOG_LEVEL_INFOS, "client data");
              bytes_read = sernetClients_140.read(net_buf, count);
              String net_data = (char *)net_buf;
              String net_data_s = net_data.substring(0, net_data.length() - 2);
              if (net_data_s == "tare") {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indsifir");
              }
            }
          } else {
            if (connectionState == 1) {
              connectionState = 0;
              sernetClients_140.setTimeout(10);
            }
          }
        }
        success = true;
        break;
      }*/

      case PLUGIN_FIFTY_PER_SECOND:
      {
        dtostrf(ind_tartim_degeri_son, 8, ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_webapinettartim_C);
        dtostrf(ind_tartim_degeri_son1, 8, ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_webapinettartim_son_C);
        if (ind_tartim_degeri_son1 == ind_tartim_degeri_son) {
          dtostrf(XML_NET_S.toFloat(),  (ExtraTaskSettings.TaskDeviceValueBit[0] - ExtraTaskSettings.TaskDeviceValueBas[0]), IND_NOKTA, XML_NET_C);
          dtostrf(XML_DARA_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[1] - ExtraTaskSettings.TaskDeviceValueBas[1]), IND_NOKTA, XML_DARA_C);
          dtostrf(XML_BRUT_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[2] - ExtraTaskSettings.TaskDeviceValueBas[2]), IND_NOKTA, XML_BRUT_C);
          dtostrf(XML_ADET_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[3] - ExtraTaskSettings.TaskDeviceValueBas[3]), 0, XML_ADET_C);
          ind_stabil_sayisi++;
        } else
          ind_stabil_sayisi = 0;
        if (ind_stabil_sayisi >= (IND_HIZ)) {
          XML_STABIL_S = "ST";
          if ((ind_tartim_degeri_son > 0) && (hayvan_modu == 0)) {
            hayvan_modu = 2;
          }
        } else {
          XML_STABIL_S = "US";
          if ((webapinettartim < 0) && (hayvan_modu == 2))
            hayvan_modu = 0;
        }
        file_data = tartimString_s;
        parseSystemVariables(file_data, false);
        Serial1.print(file_data);
        Serial.print(file_data);
        success = true;
        break;
      }

    //case PLUGIN_ONE_HUNDRED_PER_SECOND:
      case PLUGIN_TEN_PER_SECOND:
      {
        long ind_tartim_degeri = long(float((scale.read() - (Settings.ind_sifir_kal_degeri + ind_sifir_degeri)) * Settings.ind_yuk_kal_degeri) + (XML_DARA_S.toFloat() * bol));
        if (int(IND_TAKSIMAT) == 1) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan)
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
        } else if (int(IND_TAKSIMAT) == 2) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 5) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 4: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 10) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 9: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 8: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 7: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case 6: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case 5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case 4: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case -5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case -6: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case -7: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case -8: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -9: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 20) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 19: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 18: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 17: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case 16: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case 15: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case 14: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case 13: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case 12: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case 11: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case 10: ind_tartim_degeri_son = ind_tartim_degeri + 10; break;
            case 9: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case 8: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case 7: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case 6: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case 5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case 4: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case -5: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case -6: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case -7: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case -8: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case -9: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case -10: ind_tartim_degeri_son = ind_tartim_degeri - 10; break;
            case -11: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case -12: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case -13: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case -14: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case -15: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case -16: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case -17: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case -18: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -19: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 20) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 19: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 18: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 17: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case 16: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case 15: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case 14: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case 13: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case 12: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case 11: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case 10: ind_tartim_degeri_son = ind_tartim_degeri + 10; break;
            case 9: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case 8: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case 7: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case 6: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case 5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case 4: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case -5: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case -6: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case -7: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case -8: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case -9: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case -10: ind_tartim_degeri_son = ind_tartim_degeri - 10; break;
            case -11: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case -12: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case -13: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case -14: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case -15: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case -16: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case -17: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case -18: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -19: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        } else if (int(IND_TAKSIMAT) == 20) {
          int kalan = ind_tartim_degeri % int(IND_TAKSIMAT);
          switch (kalan) {
            case 49: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case 48: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case 47: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case 46: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case 45: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case 44: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case 43: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case 42: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case 41: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case 40: ind_tartim_degeri_son = ind_tartim_degeri + 10; break;
            case 39: ind_tartim_degeri_son = ind_tartim_degeri + 11; break;
            case 38: ind_tartim_degeri_son = ind_tartim_degeri + 12; break;
            case 37: ind_tartim_degeri_son = ind_tartim_degeri + 13; break;
            case 36: ind_tartim_degeri_son = ind_tartim_degeri + 14; break;
            case 35: ind_tartim_degeri_son = ind_tartim_degeri + 15; break;
            case 34: ind_tartim_degeri_son = ind_tartim_degeri + 16; break;
            case 33: ind_tartim_degeri_son = ind_tartim_degeri + 17; break;
            case 32: ind_tartim_degeri_son = ind_tartim_degeri + 18; break;
            case 31: ind_tartim_degeri_son = ind_tartim_degeri + 19; break;
            case 30: ind_tartim_degeri_son = ind_tartim_degeri + 20; break;
            case 29: ind_tartim_degeri_son = ind_tartim_degeri + 21; break;
            case 28: ind_tartim_degeri_son = ind_tartim_degeri + 22; break;
            case 27: ind_tartim_degeri_son = ind_tartim_degeri + 23; break;
            case 26: ind_tartim_degeri_son = ind_tartim_degeri + 24; break;
            case 25: ind_tartim_degeri_son = ind_tartim_degeri - 25; break;
            case 24: ind_tartim_degeri_son = ind_tartim_degeri - 24; break;
            case 23: ind_tartim_degeri_son = ind_tartim_degeri - 23; break;
            case 22: ind_tartim_degeri_son = ind_tartim_degeri - 22; break;
            case 21: ind_tartim_degeri_son = ind_tartim_degeri - 21; break;
            case 20: ind_tartim_degeri_son = ind_tartim_degeri - 20; break;
            case 19: ind_tartim_degeri_son = ind_tartim_degeri - 19; break;
            case 18: ind_tartim_degeri_son = ind_tartim_degeri - 18; break;
            case 17: ind_tartim_degeri_son = ind_tartim_degeri - 17; break;
            case 16: ind_tartim_degeri_son = ind_tartim_degeri - 16; break;
            case 15: ind_tartim_degeri_son = ind_tartim_degeri - 15; break;
            case 14: ind_tartim_degeri_son = ind_tartim_degeri - 14; break;
            case 13: ind_tartim_degeri_son = ind_tartim_degeri - 13; break;
            case 12: ind_tartim_degeri_son = ind_tartim_degeri - 12; break;
            case 11: ind_tartim_degeri_son = ind_tartim_degeri - 11; break;
            case 10: ind_tartim_degeri_son = ind_tartim_degeri - 10; break;
            case 9: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case 8: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case 7: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case 6: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case 5: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case 4: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case 3: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case 2: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case 1: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
            case 0: ind_tartim_degeri_son = ind_tartim_degeri; break;
            case -1: ind_tartim_degeri_son = ind_tartim_degeri + 1; break;
            case -2: ind_tartim_degeri_son = ind_tartim_degeri + 2; break;
            case -3: ind_tartim_degeri_son = ind_tartim_degeri + 3; break;
            case -4: ind_tartim_degeri_son = ind_tartim_degeri + 4; break;
            case -5: ind_tartim_degeri_son = ind_tartim_degeri + 5; break;
            case -6: ind_tartim_degeri_son = ind_tartim_degeri + 6; break;
            case -7: ind_tartim_degeri_son = ind_tartim_degeri + 7; break;
            case -8: ind_tartim_degeri_son = ind_tartim_degeri + 8; break;
            case -9: ind_tartim_degeri_son = ind_tartim_degeri + 9; break;
            case -10: ind_tartim_degeri_son = ind_tartim_degeri + 10; break;
            case -11: ind_tartim_degeri_son = ind_tartim_degeri + 11; break;
            case -12: ind_tartim_degeri_son = ind_tartim_degeri + 12; break;
            case -13: ind_tartim_degeri_son = ind_tartim_degeri + 13; break;
            case -14: ind_tartim_degeri_son = ind_tartim_degeri + 14; break;
            case -15: ind_tartim_degeri_son = ind_tartim_degeri + 15; break;
            case -16: ind_tartim_degeri_son = ind_tartim_degeri + 16; break;
            case -17: ind_tartim_degeri_son = ind_tartim_degeri + 17; break;
            case -18: ind_tartim_degeri_son = ind_tartim_degeri + 18; break;
            case -19: ind_tartim_degeri_son = ind_tartim_degeri + 19; break;
            case -20: ind_tartim_degeri_son = ind_tartim_degeri + 20; break;
            case -21: ind_tartim_degeri_son = ind_tartim_degeri + 21; break;
            case -22: ind_tartim_degeri_son = ind_tartim_degeri + 22; break;
            case -23: ind_tartim_degeri_son = ind_tartim_degeri + 23; break;
            case -24: ind_tartim_degeri_son = ind_tartim_degeri + 24; break;
            case -25: ind_tartim_degeri_son = ind_tartim_degeri - 25; break;
            case -26: ind_tartim_degeri_son = ind_tartim_degeri - 24; break;
            case -27: ind_tartim_degeri_son = ind_tartim_degeri - 23; break;
            case -28: ind_tartim_degeri_son = ind_tartim_degeri - 22; break;
            case -29: ind_tartim_degeri_son = ind_tartim_degeri - 21; break;
            case -30: ind_tartim_degeri_son = ind_tartim_degeri - 20; break;
            case -31: ind_tartim_degeri_son = ind_tartim_degeri - 19; break;
            case -32: ind_tartim_degeri_son = ind_tartim_degeri - 18; break;
            case -33: ind_tartim_degeri_son = ind_tartim_degeri - 17; break;
            case -34: ind_tartim_degeri_son = ind_tartim_degeri - 16; break;
            case -35: ind_tartim_degeri_son = ind_tartim_degeri - 15; break;
            case -36: ind_tartim_degeri_son = ind_tartim_degeri - 14; break;
            case -37: ind_tartim_degeri_son = ind_tartim_degeri - 13; break;
            case -38: ind_tartim_degeri_son = ind_tartim_degeri - 12; break;
            case -39: ind_tartim_degeri_son = ind_tartim_degeri - 11; break;
            case -40: ind_tartim_degeri_son = ind_tartim_degeri - 10; break;
            case -41: ind_tartim_degeri_son = ind_tartim_degeri - 9; break;
            case -42: ind_tartim_degeri_son = ind_tartim_degeri - 8; break;
            case -43: ind_tartim_degeri_son = ind_tartim_degeri - 7; break;
            case -44: ind_tartim_degeri_son = ind_tartim_degeri - 6; break;
            case -45: ind_tartim_degeri_son = ind_tartim_degeri - 5; break;
            case -46: ind_tartim_degeri_son = ind_tartim_degeri - 4; break;
            case -47: ind_tartim_degeri_son = ind_tartim_degeri - 3; break;
            case -48: ind_tartim_degeri_son = ind_tartim_degeri - 2; break;
            case -49: ind_tartim_degeri_son = ind_tartim_degeri - 1; break;
          }
        }
        dtostrf((ind_tartim_degeri_son / bol), 8, IND_NOKTA, goster_net_c);
        dtostrf((ind_dara_degeri), 8, IND_NOKTA, goster_dara_c);
        if (ind_dara_degeri < 0)
          dtostrf(((ind_tartim_degeri_son / bol) + (-1 * ind_dara_degeri)), 8, IND_NOKTA, goster_brut_c);
        else
          dtostrf(((ind_tartim_degeri_son / bol) + ind_dara_degeri), 8, IND_NOKTA, goster_brut_c);
        XML_NET_S  = String(goster_net_c);
        XML_DARA_S = String(goster_dara_c);
        XML_BRUT_S = String(goster_brut_c);
        /*if (millis() > stabilTimer_l) {
          stabilTimer_l = millis() + 200;
          stabilTartim_s = XML_NET_S;
          if (ind_stabil_sayisi >= 2) {
            ind_stabil_sayisi = 0;
            XML_STABIL_S = "ST";
          } else
            XML_STABIL_S = "US";
        } else {
          if (stabilTartim_s == XML_NET_S)
            ind_stabil_sayisi++;
          else
            ind_stabil_sayisi = 0;
        }*/
        //XML_TARIH_S = node_time.getDateString('-');
        //XML_SAAT_S = node_time.getTimeString(':');
        
        //file_data = tartimString_s;
        //file_data = string_convert(file_data);
        //parseSystemVariables(file_data, false);
        //file_data += "\r\n";
        //Serial.print(file_data);
#ifdef IND_GRAFIK
        SendUDPCommand(0, (const char *)file_data.c_str(), file_data.length());
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indseridata");
        if (IND_EKRAN_MOD == 0) {
          char batery[1] = { 54 };
          char yazici[1] = { 58 };
          //u8g2.setContrast(250);
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_battery19_tn);
          u8g2.drawStr(1, 20, &batery[0]);
          //u8g2.sendBuffer();

          u8g2.setFont(u8g2_font_t0_11_tf);
          int wifi_bas = 190 - (WiFi.SSID().length() * 6);
          u8g2.drawStr(wifi_bas, 22, WiFi.SSID().c_str());


          long rssi = WiFi.RSSI();
          if (rssi >= -55) {
            u8g2.drawBox(177, 8, 2, 1);
            u8g2.drawBox(180, 7, 2, 2);
            u8g2.drawBox(183, 5, 2, 4);
            u8g2.drawBox(186, 3, 2, 6);
            u8g2.drawBox(189, 1, 2, 8);
          } else if (rssi< -55 & rssi > - 65) {
            u8g2.drawBox(177, 8, 2, 1);
            u8g2.drawBox(180, 7, 2, 2);
            u8g2.drawBox(183, 5, 2, 4);
            u8g2.drawBox(186, 3, 2, 6);
          } else if (rssi< -65 & rssi > - 75) {
            u8g2.drawBox(177, 8, 2, 1);
            u8g2.drawBox(180, 7, 2, 2);
            u8g2.drawBox(183, 5, 2, 4);
          } else if (rssi< -75 & rssi > - 85) {
            u8g2.drawBox(177, 8, 2, 1);
            u8g2.drawBox(180, 7, 2, 2);
          } else if (rssi< -85 & rssi > - 96) {
            u8g2.drawBox(177, 8, 2, 1);
          }
          u8g2.drawXBMP(15, 0, 15, 19, ETIKET_YAZICI);
          u8g2.drawXBMP(30, 0, 15, 19, FIS_YAZICI);

          u8g2.setFont(u8g2_font_logisoso38_tr);
          u8g2.drawStr(-7, 64, XML_NET_S.c_str());  // write something to the internal memory
          u8g2.sendBuffer();

          u8g2_1.clearBuffer();
          u8g2_1.setFont(u8g2_font_logisoso38_tr);
          u8g2_1.drawStr(-7, 32, XML_NET_S.c_str());  // write something to the internal memory
          u8g2_1.setFont(u8g2_font_ncenB10_tr);
          u8g2_1.drawStr(175, 26, "kg");
          u8g2_1.sendBuffer();
        } else {
          u8g2.clearBuffer();
          u8g2.setFont(u8g2_font_ncenB14_tr);
          u8g2.drawStr(1, 32, XML_NET_S.c_str());  // write something to the internal memory
          u8g2.setFont(u8g2_font_ncenB10_tr);
          u8g2.drawStr(72, 28, "kg");

          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(20, 16, "NET");  // write something to the internal memory

          u8g2.setFont(u8g2_font_ncenB14_tr);
          u8g2.drawStr(96, 32, XML_DARA_S.c_str());  // write something to the internal memory
          u8g2.setFont(u8g2_font_ncenB10_tr);
          u8g2.drawStr(165, 28, "kg");

          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.drawStr(120, 16, "DARA");  // write something to the internal memory
          u8g2.sendBuffer();

          u8g2_1.clearBuffer();
          u8g2_1.setFont(u8g2_font_ncenB08_tr);
          u8g2_1.drawStr(1, 10, "PLU NO:");
          u8g2_1.drawStr(100, 10, "KOD:");
          u8g2_1.drawStr(136, 10, XML_PLU_KOD_S.c_str());
          u8g2_1.drawStr(52, 10, key_data.c_str());
          u8g2_1.drawStr(5, 20, XML_PLU_ADI_S.c_str());
          u8g2_1.sendBuffer();
        }
        if (keyPad.isPressed()) {
          if ((keyPad.getChar() <= 57) && (keyPad.getChar() >= 48))
            key_data += char(keyPad.getChar());
          if (keyPad.getChar() == 'C')
            key_data = "";
          if (keyPad.getChar() == 'E')
            ind_dara_degeri = key_data.toFloat() / bol;
          if (keyPad.getChar() == 'D') {
            if (fileExists(FILE_PLU)) {
              int pluno = 0;
              fs::File form = tryOpenFile(FILE_PLU, "r");
              String s = "0";
              while (form.position() < form.size()) {
                s = form.readStringUntil('\n');
                if (s.indexOf(",") > 0) {
                  pluno++;
                  int say = s.indexOf(",");
                  if (pluno == key_data.toInt()) {
                    XML_PLU_ADI_S = s.substring(0, say);
                    XML_BARKOD_S = s.substring((say + 1), (say + 8));
                    XML_PLU_KOD_S = XML_BARKOD_S;
                  }
                  s = "";
                } else
                  s.trim();
              }
              form.close();
            }
          }
          if (keyPad.getChar() == 'W') {
            int pluno_click = key_data.toInt();
            pluno_click++;
            key_data = String(pluno_click);
            if (fileExists(FILE_PLU)) {
              int pluno = 0;
              fs::File form = tryOpenFile(FILE_PLU, "r");
              String s = "0";
              while (form.position() < form.size()) {
                s = form.readStringUntil('\n');
                if (s.indexOf(",") > 0) {
                  pluno++;
                  int say = s.indexOf(",");
                  if (pluno == key_data.toInt()) {
                    XML_PLU_ADI_S = s.substring(0, say);
                    XML_BARKOD_S = s.substring((say + 1), (say + 8));
                    XML_PLU_KOD_S = XML_BARKOD_S;
                  }
                  s = "";
                } else
                  s.trim();
              }
              form.close();
            }
          }
          if (keyPad.getChar() == 'S') {
            if (key_data.toInt() >= 0) {
              int pluno_click = key_data.toInt();
              pluno_click--;
              key_data = String(pluno_click);
            }
            if (fileExists(FILE_PLU)) {
              int pluno = 0;
              fs::File form = tryOpenFile(FILE_PLU, "r");
              String s = "0";
              while (form.position() < form.size()) {
                s = form.readStringUntil('\n');
                if (s.indexOf(",") > 0) {
                  pluno++;
                  int say = s.indexOf(",");
                  if (pluno == key_data.toInt()) {
                    XML_PLU_ADI_S = s.substring(0, say);
                    XML_BARKOD_S = s.substring((say + 1), (say + 8));
                    XML_PLU_KOD_S = XML_BARKOD_S;
                  }
                  s = "";
                } else
                  s.trim();
              }
              form.close();
            }
          }
          if (keyPad.getChar() == 'A') {
            String yaz_data = "eyzpluart#";
            yaz_data += key_data;
            yaz_data += "#1#1#1";
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, yaz_data.c_str());
            delay(300);
          }
          delay(200);
        }
#endif
        if (IND_TCP_AKTiF) {
          if (sernetServer_140->hasClient()) {
            if (sernetClients_140) { sernetClients_140.stop(); }
            sernetClients_140 = sernetServer_140->available();
          }
          if (sernetClients_140.connected()) {
            addLogMove(LOG_LEVEL_INFO, "Client Bagli");
            connectionState = 1;
            int bytes_read = 0;
            uint8_t net_buf[32];
            int count = sernetClients_140.available();
            if (count > 0) {
              addLogMove(LOG_LEVEL_INFO, "client data");
              bytes_read = sernetClients_140.read(net_buf, count);
              String net_data = (char *)net_buf;
              String net_data_s = net_data.substring(0, net_data.length() - 2);
              if (net_data_s == "tare") {
                ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indsifir");
              }
            }
          } else {
            if (connectionState == 1) {
              connectionState = 0;
              sernetClients_140.setTimeout(10);
            }
          }
        }
        tartimdata_s = XML_NET_C;
        if (IND_TCPMODBUS_AKTiF) {
          Mb_140.Run();
          //slave.run();
          unsigned int reg0 = f_2uint_int1(XML_NET_S.toFloat());
          unsigned int reg1 = f_2uint_int2(XML_NET_S.toFloat());
          Mb_140.MBHoldingRegister[0] = reg0;
          Mb_140.MBHoldingRegister[1] = reg1;
          //regBank.set(40001, (word)reg0);
          //regBank.set(40002, (word)reg1);
          unsigned int reg2 = f_2uint_int1(XML_DARA_S.toFloat());
          unsigned int reg3 = f_2uint_int2(XML_DARA_S.toFloat());
          Mb_140.MBHoldingRegister[2] = reg2;
          Mb_140.MBHoldingRegister[3] = reg3;
          //regBank.set(40003, (word)reg2);
          //regBank.set(40004, (word)reg3);
          unsigned int reg4 = f_2uint_int1(XML_BRUT_S.toFloat());
          unsigned int reg5 = f_2uint_int2(XML_BRUT_S.toFloat());
          Mb_140.MBHoldingRegister[4] = reg4;
          Mb_140.MBHoldingRegister[5] = reg5;
          //regBank.set(40005, (word)reg4);
          //regBank.set(40006, (word)reg5);
          unsigned int reg8 = f_2uint_int1(scale.read());
          unsigned int reg9 = f_2uint_int2(scale.read());
          Mb_140.MBHoldingRegister[8] = reg8;
          Mb_140.MBHoldingRegister[9] = reg9;
          //regBank.set(40007, (word)reg8);
          //regBank.set(40008, (word)reg9);
          switch (Mb_140.MBHoldingRegister[10]) {
            case 1: ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "inddaraekle"); break;
            case 2: ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "inddarasil"); break;
            case 3: ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indsifir"); break;
            case 4: ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "indsifirkal"); break;
            case 5:
              int carp = 0;
              switch (ExtraTaskSettings.TaskDeviceValueDecimals[0]) {
                case 0: carp = 1; break;
                case 1: carp = 10; break;
                case 2: carp = 100; break;
                case 3: carp = 1000; break;
                case 4: carp = 10000; break;
              }
              float kal_yuk_degeri = f_2uint_float(Mb_140.MBHoldingRegister[6], Mb_140.MBHoldingRegister[7]) * carp;
              String komut = "indyukkal#";
              komut += String(kal_yuk_degeri);
              ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, komut.c_str());
              break;
          }
          if (Mb_140.MBHoldingRegister[10] != 0)
            Mb_140.MBHoldingRegister[10] = 0;
#ifdef IND_PANO 
          if (pcf8574.digitalRead(P4) == LOW) {
            XML_INPUT_PIN_C[0] = '0';
            Mb_140.MBHoldingRegister[11] == 1;
          } else
            XML_INPUT_PIN_C[0] = '1';
          if (pcf8574.digitalRead(P5) == LOW) {
            XML_INPUT_PIN_C[1] = '0';
            Mb_140.MBHoldingRegister[12] == 1;
          } else
            XML_INPUT_PIN_C[1] = '1';
          if (pcf8574.digitalRead(P6) == LOW)
            XML_INPUT_PIN_C[2] = '0';
          else
            XML_INPUT_PIN_C[2] = '1';
          if (pcf8574.digitalRead(P7) == LOW)
            XML_INPUT_PIN_C[3] = '0';
          else
            XML_INPUT_PIN_C[3] = '1';

          Mb_140.MBInputRegister[0] = int(XML_INPUT_PIN_C[0]) - 48;
          Mb_140.MBInputRegister[1] = int(XML_INPUT_PIN_C[1]) - 48;
          Mb_140.MBInputRegister[2] = int(XML_INPUT_PIN_C[2]) - 48;
          Mb_140.MBInputRegister[3] = int(XML_INPUT_PIN_C[3]) - 48;

          if ((Mb_140.MBHoldingRegister[11] == 1) && (output_1_aktif)) {
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#0#1");
            output_1_aktif = false;
          } else if ((Mb_140.MBHoldingRegister[11] == 0) && (!output_1_aktif)) {
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#0#0");
            output_1_aktif = true;
          }
          if ((Mb_140.MBHoldingRegister[12] == 1) && (output_2_aktif)) {
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#1#1");
            output_2_aktif = false;
          } else if ((Mb_140.MBHoldingRegister[12] == 0) && (!output_2_aktif)) {
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#1#0");
            output_2_aktif = true;
          }
          if ((Mb_140.MBHoldingRegister[13] == 1) && (output_3_aktif)) {
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#2#1");
            output_3_aktif = false;
          } else if ((Mb_140.MBHoldingRegister[13] == 0) && (!output_3_aktif)) {
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#2#0");
            output_3_aktif = true;
          }
          if ((Mb_140.MBHoldingRegister[14] == 1) && (output_4_aktif)) {
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#3#1");
            output_4_aktif = false;
          } else if ((Mb_140.MBHoldingRegister[14] == 0) && (!output_4_aktif)) {
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "pcfoutput#3#0");
            output_4_aktif = true;
          }
#endif
        }
        if (IND_UDP_AKTiF)
          sendSysInfoUDP(1);
        if (IND_TCP_AKTiF) {
          sernetClients_140.print(file_data);
          sernetClients_140.flush();
        }

        if (ind_i >= IND_FILTRE) {
          ind_i--;
          ind_sum_f = ind_sum_f - ind_sum_f_d[0];
          for (uint8_t i = 0; i < IND_FILTRE; i++)
            ind_sum_f_d[i] = ind_sum_f_d[i + 1];
        } else {
            ind_sum_f_d[ind_i] = ind_tartim_degeri_son;
            ind_sum_f = ind_sum_f + ind_sum_f_d[ind_i];
            ind_tartim_degeri_son1 = ind_sum_f / (ind_i + 1);
            ind_i++;
        }
        success = true;
        break;
      }

      case PLUGIN_WRITE:
      {
        if (Settings.UDPPort > 0) {
          //Serial1.println("BARKOD OKUNDU");
          //Serial.println("BARKOD OKUNDU");
          string.replace("\r","");
          string.replace("\n","");
          XML_QRKOD_S = string;
          String barkod = "scan";
          barkod += XML_QRKOD_S;
          barkod +="\r\n";
          Serial1.print(barkod);
          //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
          string = "";
        }
        success = true;
        break;
      }
  }
  return success;
}
#endif  // USES_P140