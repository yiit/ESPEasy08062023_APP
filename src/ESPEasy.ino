
#ifdef CONTINUOUS_INTEGRATION
# pragma GCC diagnostic error "-Wall"
#else // ifdef CONTINUOUS_INTEGRATION
# pragma GCC diagnostic warning "-Wall"
#endif // ifdef CONTINUOUS_INTEGRATION

// Include this as first, to make sure all defines are active during the entire compile.
// See: https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=7980
// If Custom.h build from Arduino IDE is needed, uncomment #define USE_CUSTOM_H in ESPEasy_common.h
#include "src/ESPEasyCore/ESPEasyWifi.h"
#include "ESPEasy_common.h"
#include "ESPEasy-Globals.h"

extern void Plugin_999_Init();
extern void Plugin_999_Loop();
//#define USE_CUSTOM_H

#ifdef USE_CUSTOM_H

// make the compiler show a warning to confirm that this file is inlcuded
  # warning "**** Using Settings from Custom.h File ***"
#endif // ifdef USE_CUSTOM_H


// Needed due to preprocessor issues.
#ifdef PLUGIN_SET_GENERIC_ESP32
  # ifndef ESP32
    #  define ESP32
  # endif // ifndef ESP32
#endif // ifdef PLUGIN_SET_GENERIC_ESP32

/****************************************************************************************************************************\
 * Arduino project "ESP Easy" © Copyright www.letscontrolit.com
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'License.txt'.
 *
 * IDE download    : https://www.arduino.cc/en/Main/Software
 * ESP8266 Package : https://github.com/esp8266/Arduino
 *
 * Source Code     : https://github.com/ESP8266nu/ESPEasy
 * Support         : http://www.letscontrolit.com
 * Discussion      : http://www.letscontrolit.com/forum/
 *
 * Additional information about licensing can be found at : http://www.gnu.org/licenses
 \*************************************************************************************************************************/

// This file incorporates work covered by the following copyright and permission notice:

/****************************************************************************************************************************\
 * Arduino project "Nodo" © Copyright 2010..2015 Paul Tonkes
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'License.txt'.
 *
 * Voor toelichting op de licentievoorwaarden zie    : http://www.gnu.org/licenses
 * Uitgebreide documentatie is te vinden op          : http://www.nodo-domotica.nl
 * Compiler voor deze programmacode te downloaden op : http://arduino.cc
 \*************************************************************************************************************************/

//   Simple Arduino sketch for ESP module, supporting:
//   =================================================================================
//   Simple switch inputs and direct GPIO output control to drive relays, mosfets, etc
//   Analog input (ESP-7/12 only)
//   Pulse counters
//   Dallas OneWire DS18b20 temperature sensors
//   DHT11/22/12 humidity sensors
//   BMP085 I2C Barometric Pressure sensor
//   PCF8591 4 port Analog to Digital converter (I2C)
//   RFID Wiegand-26 reader
//   MCP23017 I2C IO Expanders
//   BH1750 I2C Luminosity sensor
//   Arduino Pro Mini with IO extender sketch, connected through I2C
//   LCD I2C display 4x20 chars
//   HC-SR04 Ultrasonic distance sensor
//   SI7021 I2C temperature/humidity sensors
//   TSL2561 I2C Luminosity sensor
//   TSOP4838 IR receiver
//   PN532 RFID reader
//   Sharp GP2Y10 dust sensor
//   PCF8574 I2C IO Expanders
//   PCA9685 I2C 16 channel PWM driver
//   OLED I2C display with SSD1306 driver
//   MLX90614 I2C IR temperature sensor
//   ADS1115 I2C ADC
//   INA219 I2C voltage/current sensor
//   BME280 I2C temp/hum/baro sensor
//   MSP5611 I2C temp/baro sensor
//   BMP280 I2C Barometric Pressure sensor
//   SHT1X temperature/humidity sensors
//   Ser2Net server
//   DL-Bus (Technische Alternative)

// Define globals before plugin sets to allow a personal override of the selected plugins
#include "ESPEasy-Globals.h"

// Must be included after all the defines, since it is using TASKS_MAX
#include "_Plugin_Helper.h"

// Plugin helper needs the defined controller sets, thus include after 'define_plugin_sets.h'
#include "src/Helpers/_CPlugin_Helper.h"


#include "src/ESPEasyCore/ESPEasy_setup.h"
#include "src/ESPEasyCore/ESPEasy_loop.h"

#include "src/Commands/InternalCommands.h"

#ifdef PHASE_LOCKED_WAVEFORM
# include <core_esp8266_waveform.h>
#endif // ifdef PHASE_LOCKED_WAVEFORM

#if FEATURE_ADC_VCC
ADC_MODE(ADC_VCC);
#endif // if FEATURE_ADC_VCC

#ifdef ESP_NOW_ACTIVE
#include "src/Globals/Settings.h"
#include <ctype.h> // isdigit fonksiyonu için

void updatePeerStatus(const uint8_t *mac_addr, bool active) {
  for (int i = 0; i < peerStatusCount; i++) {
    if (memcmp(peerStatusList[i].mac, mac_addr, 6) == 0) {
      peerStatusList[i].active = active;
      return;
    }
  }

  if (peerStatusCount < MAX_PAIRED_DEVICES) {
    memcpy(peerStatusList[peerStatusCount].mac, mac_addr, 6);
    peerStatusList[peerStatusCount].active = active;
    peerStatusCount++;
  }
}

void LoadPairedMac() {
  preferences.begin("espnow", true);
  int macDataLength = preferences.getBytesLength("paired_mac");
  pairedDeviceCount = macDataLength / 6;
  if (pairedDeviceCount > MAX_PAIRED_DEVICES) pairedDeviceCount = MAX_PAIRED_DEVICES;
  if (pairedDeviceCount > 0) {
    preferences.getBytes("paired_mac", pairedMacList, macDataLength);
    isPaired = true;
    for (int i = 0; i < pairedDeviceCount; i++) {
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, pairedMacList[i], 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      esp_now_add_peer(&peerInfo);
      updatePeerStatus(pairedMacList[i], false);
    }
  } else {
    isPaired = false;
  }
  preferences.end();
}

void RemovePairedMac(const char* macStr) {
  uint8_t mac[6];
  sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

  int index = -1;
  for (int i = 0; i < pairedDeviceCount; i++) {
    if (memcmp(pairedMacList[i], mac, 6) == 0) {
      index = i;
      break;
    }
  }

  if (index >= 0) {
    // listedeki öğeyi kaydırarak sil
    for (int i = index; i < pairedDeviceCount - 1; i++) {
      memcpy(pairedMacList[i], pairedMacList[i + 1], 6);
    }
    pairedDeviceCount--;

    // preferences’a gerçekten yaz
    preferences.begin("espnow", false);
    if (pairedDeviceCount > 0) {
      preferences.putBytes("paired_mac", pairedMacList, pairedDeviceCount * 6);
    } else {
      preferences.remove("paired_mac");  // hepsi silindiyse key'i kaldır
    }
    preferences.end();

    // RAM'den ve esp-now'dan da çıkar
    esp_now_del_peer(mac);
    Serial.println("🧹 MAC başarıyla silindi.");
  } else {
    Serial.println("⚠️ MAC adresi listede bulunamadı.");
  }
}

void addToDiscoveredList(const uint8_t *mac_addr) {
  for (int i = 0; i < pairedDeviceCount; i++) {
    if (memcmp(pairedMacList[i], mac_addr, 6) == 0) return;
  }
  for (int i = 0; i < discoveredCount; i++) {
    if (memcmp(discoveredMacList[i], mac_addr, 6) == 0) return;
  }
  if (discoveredCount < MAX_PAIRED_DEVICES) {
    memcpy(discoveredMacList[discoveredCount], mac_addr, 6);
    discoveredCount++;
  }
}

void SavePairedMac(const uint8_t *newMac) {
  preferences.begin("espnow", false);
  pairedDeviceCount = preferences.getBytesLength("paired_mac") / 6;
  if (pairedDeviceCount > MAX_PAIRED_DEVICES) pairedDeviceCount = MAX_PAIRED_DEVICES;
  preferences.getBytes("paired_mac", pairedMacList, pairedDeviceCount * 6);

  bool alreadyExists = false;
  for (int i = 0; i < pairedDeviceCount; i++) {
    if (memcmp(pairedMacList[i], newMac, 6) == 0) {
      alreadyExists = true;
      break;
    }
  }

  if (!alreadyExists) {
    if (pairedDeviceCount < MAX_PAIRED_DEVICES) {
      memcpy(pairedMacList[pairedDeviceCount], newMac, 6);
      pairedDeviceCount++;
    } else {
      for (int i = 0; i < MAX_PAIRED_DEVICES - 1; i++) {
        memcpy(pairedMacList[i], pairedMacList[i + 1], 6);
      }
      memcpy(pairedMacList[MAX_PAIRED_DEVICES - 1], newMac, 6);
    }
  }

  preferences.putBytes("paired_mac", pairedMacList, pairedDeviceCount * 6);
  preferences.end();

  updatePeerStatus(newMac, true);
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, newMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (!esp_now_is_peer_exist(newMac)) {
    esp_now_add_peer(&peerInfo);
  }

  Serial.println("✅ Eşleşmiş MAC adresi kaydedildi.");
}

void PrintMacAddress(const uint8_t *mac) {
  Serial.print("MAC Adresi: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

String macToStr(const uint8_t *mac) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void AddPeer(const uint8_t *mac_addr) {
  if (esp_now_is_peer_exist(mac_addr)) {
    Serial.println("ℹ️ Peer zaten kayıtlı.");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac_addr, 6);
  peerInfo.channel = 0;         // aynı kanal
  peerInfo.encrypt = false;     // şifreleme kullanmıyoruz

  esp_err_t result = esp_now_add_peer(&peerInfo);
  if (result == ESP_OK) {
    Serial.print("✅ Peer eklendi: ");
    PrintMacAddress(mac_addr);
  } else {
    Serial.printf("❌ Peer eklenemedi! Hata kodu: %d\n", result);
  }
}


void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int len) {
  addToDiscoveredList(mac_addr);  
  hataTimer_l = millis();
  char receivedData[len + 1];
  memcpy(receivedData, data, len);
  receivedData[len] = '\0';

  if (strcmp(receivedData, "PAIR_REQUEST") == 0) {
    Serial.println("Eslesme Istegi Alindi!");
    const char *response = "PAIR_RESPONSE";
	esp_now_send(mac_addr, (uint8_t *)response, strlen(response));
    isPaired = true;
	SavePairedMac(mac_addr);
	AddPeer(mac_addr);
    return;
  }

  else if (strcmp(receivedData, "PAIR_DEL") == 0) {
  Serial.println("🧹 Pair silme komutu alındı!");
  String macStr = macToStr(mac_addr);
  RemovePairedMac(macStr.c_str());
}
  else if (strcmp(receivedData, "PAIR_RESPONSE") == 0) {
    Serial.println("Eslesme Yaniti Alindi!");
    isPaired = true;
	SavePairedMac(mac_addr);
	PrintMacAddress(mac_addr);
    return;
  }

  else if (strcmp(receivedData, "inddara") == 0) {
    Serial.println("DARA!");
	digitalWrite(4,LOW);
	delay(1000);
	digitalWrite(4,HIGH);
  }

  else {
	String formattedText = String(receivedData);
	Serial.print("Gelen veri: ");
	Serial.println(receivedData);
	// Gelen veriyi sabit olarak LED ekrana yazdır
	// display.displayText(receivedData.c_str(), PA_CENTER, 0, 0, PA_NO_EFFECT, PA_NO_EFFECT); // Sabit pozisyon belirle
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
updatePeerStatus(mac_addr, status == ESP_NOW_SEND_SUCCESS);
}

void SendData(String data) {
  if (!isPaired || pairedDeviceCount == 0) return;

  for (int i = 0; i < pairedDeviceCount; i++) {
    esp_err_t result = esp_now_send(pairedMacList[i], (uint8_t *)data.c_str(), data.length());
    updatePeerStatus(pairedMacList[i], result == ESP_OK);
  }
}
#endif


#ifdef CORE_POST_2_5_0

/*********************************************************************************************\
* Pre-init
\*********************************************************************************************/
void preinit();
void preinit() {
  system_phy_set_powerup_option(3);
  // Global WiFi constructors are not called yet
  // (global class instances like WiFi, Serial... are not yet initialized)..
  // No global object methods or C++ exceptions can be called in here!
  // The below is a static class method, which is similar to a function, so it's ok.
  #ifndef CORE_POST_3_0_0
  //ESP8266WiFiClass::preinitWiFiOff();
  #endif

  // Prevent RF calibration on power up.
  // TD-er: disabled on 2021-06-07 as it may cause several issues with some boards.
  // It cannot be made a setting as we can't read anything of our own settings.
  //system_phy_set_powerup_option(RF_NO_CAL);
}

#endif // ifdef CORE_POST_2_5_0

#if defined(HAS_BLE) or defined(HAS_BLE_CLIENT)

//  None of these are required as they will be handled by the library with defaults.       Remove as you see fit for your needs  
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      pinMode(2, OUTPUT);
      digitalWrite(2, HIGH);
    }
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      pinMode(2, OUTPUT);
      digitalWrite(2, LOW);
    }
};

class MyClientCallbacks : public NimBLEClientCallbacks {
  void onConnect(NimBLEClient* pClient) override {
    Serial.println("🟢 Client bağlı.");
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
  }

  void onDisconnect(NimBLEClient* pClient) override {
    Serial.println("🔴 Client bağlantısı koptu!");
    clientConnected = false;  // 🧠 Yeniden bağlanma için trigger
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
  }
};


/*class MyClientCallbacks : public BLEClientCallbacks {
    void onConnect(BLEClient *pClient) {
        Serial.println("Client Connected to Server!");
    }

    void onDisconnect(BLEClient *pClient) {
        Serial.println("Client Disconnected from Server.");
    }
};*/

// Server'dan gelen veriyi işleme
/*class ServerRxCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            Serial.print("Server received: ");
            Serial.println(rxValue.c_str());
            // Alınan veriyi Client'a gönder
            if (clientConnected && pRemoteRxCharacteristic) {
                pRemoteRxCharacteristic->writeValue(rxValue);
            }
        }
    }
};*/

class ServerRxCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
        if (rxValue.length() > 0) {
            // Gelen veriyi konsola yazdır
            //Serial.print("Server received: ");
            Serial1.println(rxValue.c_str());
            ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, rxValue.c_str());

            // Gelen veriyi Client'a yazmayı deneyin
            if (clientConnected && pRemoteRxCharacteristic) {
                pRemoteRxCharacteristic->writeValue(rxValue);
                //Serial.println("Forwarded data to client.");
            } //else {
                //Serial.println("Client not connected or Remote RX not available.");
            //}
        } //else {
            //Serial.println("No data received.");
        //}
    }
};

// Client'dan Notify alındığında işleme
void notifyCallback(BLERemoteCharacteristic *pRemoteCharacteristic, uint8_t *data, size_t length, bool isNotify) {
    Serial.print("Client received: ");
    for (size_t i = 0; i < length; i++) {
        Serial.print((char)data[i]);
    }
    //Serial.println();
    // Alınan veriyi Server'a gönder
    if (deviceConnected && pTxCharacteristic) {
        pTxCharacteristic->setValue(data, length);
        pTxCharacteristic->notify();
    }
}

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        for (int i = 0; i < rxValue.length(); i++)
          Serial.write(rxValue[i]);
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, rxValue.c_str());
      }
    }
};

/*class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      for (int i = 0; i < rxValue.length(); i++)
        Serial.write(rxValue[i]);

      String komut = String(rxValue.c_str());
      komut.trim();  // 🔥 "\r\n" varsa temizle
      Serial.println(">> Komut temizlendi: " + komut);

      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, komut.c_str());
    }
  }
};*/

class MyCallbacksPrinter: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        for (int i = 0; i < rxValue.length(); i++)
          Serial1.write(rxValue[i]);
        Serial1.println();
      }
    }
};

#endif

#ifdef HAS_BLE_CLIENT

//#define TARGET_MAC_ADDRESS "64:2d:31:21:41:50"
//#define TARGET_MAC_ADDRESS "dc:0d:30:e3:12:6c"
#define TARGET_MAC_ADDRESS "dc:0d:30:f2:db:A8"

void connectToServer() {
  Serial.println("Scanning for BLE devices...");

  if (pClient != nullptr) {
    if (pClient->isConnected()) {
      pClient->disconnect();
    }
    NimBLEDevice::deleteClient(pClient);
    pClient = nullptr;
  }

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->clearResults();
  BLEScanResults results = pBLEScan->start(5);

  for (int i = 0; i < results.getCount(); ++i) {
      BLEAdvertisedDevice dev = results.getDevice(i);

      String found = String(dev.getAddress().toString().c_str());  // std::string → const char* → String
      if (found.equalsIgnoreCase(String(TARGET_MAC_ADDRESS))) {
        Serial.println("Target device found via MAC!");

          pClient = BLEDevice::createClient();
          pClient->setClientCallbacks(new MyClientCallbacks());  // ✅ EKLENDİ

          if (!pClient->connect(&dev)) {
              Serial.println("Connection failed");
              clientConnected = false;  // ✅ EKLENDİ
              return;
          }

          if (pClient->isConnected()) {
              Serial.println("Connected to device!");

              BLERemoteService* printerService = pClient->getService(BLEUUID(SERVICE_UUID_PRINTER));
              if (printerService) {
                  pRemoteRxCharacteristic = printerService->getCharacteristic(CHARACTERISTIC_UUID_PRINTER_RX);
                  clientConnected = true;
                  Serial.println("Ready to write to device.");
                  return;
              } else {
                  Serial.println("Service not found.");
              }
          }
      }
  }

  Serial.println("Target device not found.");
  clientConnected = false;  // ✅ EKLENDİ
}

TaskHandle_t BLETaskHandle = NULL;

void BLEConnectTask(void * parameter) {
  for (;;) {
    if (!clientConnected) {
      Serial.println("🔁 BLE bağlantısı kopmuş. Yeniden deniyorum...");
      connectToServer();
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
#endif



void setup() {
  ESPEasy_setup();
  Plugin_999_Init();
  randomNumber = random(1000, 9999);
  String fisno = node_time.getTimeString('0');
  fis_no = fisno.toInt();
#ifdef ESP32
  EEPROM.begin(1000);
  int address = 0;
  seri_no = EEPROM.readLong(address);
  address += sizeof(uint32_t);
  sno = EEPROM.readLong(address);
  address += sizeof(uint32_t);
  top_net = EEPROM.readFloat(address);
  address += sizeof(float);
  
  XML_SAYAC_1_S = String(EEPROM.readLong(100));
  dtostrf(XML_SAYAC_1_S.toInt(), 4, 0, XML_SAYAC_1_C);
  XML_SAYAC_1_SONSUZ_S = String(EEPROM.readLong(110));
  
  XML_SAYAC_2_S = String(EEPROM.readLong(104));
  dtostrf(XML_SAYAC_2_S.toInt(), 4, 0, XML_SAYAC_2_C);
  
  XML_SAYAC_3_S = String(EEPROM.readLong(118));
  dtostrf(XML_SAYAC_3_S.toInt(), 4, 0, XML_SAYAC_3_C);
  
  XML_SAYAC_4_S = String(EEPROM.readLong(126));
  dtostrf(XML_SAYAC_4_S.toInt(), 4, 0, XML_SAYAC_4_C);
#else
  EEPROM.begin(512);
  int address = 0;
  EEPROM.get(address, seri_no);
  address += sizeof(seri_no);
  EEPROM.get(address, top_net);
  address += sizeof(top_net);
  EEPROM.get(address, sno);
  address += sizeof(sno);
#endif

#ifdef HAS_BLE
  BLEDevice::init(Settings.Name); // BLE sunucusunu başlat
  pServer = BLEDevice::createServer(); // Create the BLE Server
  pServer->setCallbacks(new MyServerCallbacks());

  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallbacks());

  // UART servisini oluştur
  BLEService *pService = pServer->createService(SERVICE_UUID_UART);
  pTxCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID_TX,
                                          NIMBLE_PROPERTY::READ |
                                          NIMBLE_PROPERTY::NOTIFY
                                          );
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                                          CHARACTERISTIC_UUID_RX,
                                          NIMBLE_PROPERTY::WRITE
                                          );                                          
  //pRxCharacteristic->setCallbacks(new MyCallbacks());
  pRxCharacteristic->setCallbacks(new ServerRxCallbacks());
  pService->start();

  // Yazıcı servisini oluştur
  BLEService* pPrinterService = pServer->createService(SERVICE_UUID_PRINTER);
  BLECharacteristic * pPrinterRxCharacteristic = pPrinterService->createCharacteristic(
                                                 CHARACTERISTIC_UUID_PRINTER_RX,
                                                 NIMBLE_PROPERTY::WRITE
                                                 );
  pPrinterRxCharacteristic->setCallbacks(new MyCallbacksPrinter());
  pPrinterService->start();

  // BLE reklamlarını başlat
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID_UART);
  pAdvertising->addServiceUUID(SERVICE_UUID_PRINTER);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  // BLE Client Setup
  /*BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->start(1, true);*/

#endif

// Yeni BLE taskını başlat
#ifdef HAS_BLE_CLIENT
    xTaskCreatePinnedToCore(
      BLEConnectTask, // Görev fonksiyonu
      "BLEConnectTask", // Görev adı
      4096, // Yığın boyutu
      NULL, // Parametre
      1, // Öncelik
      &BLETaskHandle, // Görev tanıtıcısı
      1 // Çekirdek numarası (1 önerilir)
    );
#endif
#ifdef ESP_NOW_ACTIVE
  esp_now_init();
  ESP_NOW();
#endif
}

void loop() {
  ESPEasy_loop();
  Plugin_999_Loop();

#ifdef HAS_BLE  
  /*if (!deviceConnected) {
    BLEDevice::startAdvertising();
  }*/

  /*if (!clientConnected) {
    connectToServer();
  }*/

  if (deviceConnected) {
    String ble_data = XML_NET_C;
    ble_data += "\r\n";
    pTxCharacteristic->setValue((uint8_t*)ble_data.c_str(), ble_data.length());
    pTxCharacteristic->notify();
    delay(200); // bluetooth stack will go into congestion, if too many packets are sent
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(200); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
#endif
}

#ifdef ESP_NOW_ACTIVE

void ESP_NOW() {

esp_now_register_send_cb(OnDataSent);

LoadPairedMac();

if (!isPaired && !preferences.getBytesLength("paired_mac")) {
    Serial.println("Eslesmis Cihaz Yok. Eslesme Bekleniyor."); 
} else {
    Serial.println("Eslesmis cihaz bulundu:");
    
    // 'i' degiskenini 0 olarak baslat ve kayıtlı cihaz sayısına gore dongu yap
    int maxDeviceCount = sizeof(pairedMacList) / sizeof(pairedMacList[0]);
    for (int i = 0; i < pairedDeviceCount && i < maxDeviceCount; i++) {
        if (pairedMacList[i] != nullptr) { // Gecerli MAC adresi kontrolu
            AddPeer(pairedMacList[i]); // Eslesmis cihazı peer olarak ekle
            Serial.printf("Peer eklendi: Cihaz %d\n", i);
        } else {
            Serial.printf("Gecersiz MAC adresi: Cihaz %d\n", i);
        }
    }
  }
  
    // Gelen veri için callback fonksiyonu
    esp_now_register_recv_cb(OnDataRecv);
  
    //const char *pairMessage = "PAIR_REQUEST";
  
    // Peer ekle
    //esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    //esp_err_t peerResult = esp_now_add_peer(&peerInfo);
    /*if (peerResult != ESP_OK) {
      Serial.printf("Peer eklenemedi! Hata: %d (0x%X)\n", peerResult, peerResult);
    }*/
    
    // Mesaj gönder
    //esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)pairMessage, strlen(pairMessage));
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Eslesme Istegi Basariyla Gonderildi.");
    } else {
        Serial.printf("Eslesme Istegi Gonderilemedi. Hata: %d (0x%X)\n");
    }
  }
#endif
