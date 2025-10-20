#include "../ESPEasyCore/Serial.h"

#include "../ESPEasyCore/ESPEasyWifi.h"

#include "../Commands/InternalCommands.h"

#include "../Globals/Cache.h"
#include "../Globals/ESPEasy_Console.h"
#include "../Globals/Logging.h" //  For serialWriteBuffer
#include "../Globals/Settings.h"

#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/Memory.h"

#include "ESPEasy_common.h"
#include "ESPEasy-Globals.h"

/********************************************************************************************\
 * Get data from Serial Interface
 \*********************************************************************************************/
int butonbas = 0;
uint8_t SerialInByte;
int  SerialInByteCounter = 0;
char InputBuffer_Serial[INPUT_BUFFER_SIZE + 2];

uint8_t Serial2InByte;
int  Serial2InByteCounter = 0;
char InputBuffer_Serial2[INPUT_BUFFER_SIZE + 2];

/*#ifdef ESP32
#ifdef HAS_BLUETOOTH
uint8_t SerialInByteBT;
int SerialInByteCounterBT = 0;
char InputBuffer_SerialBT[INPUT_BUFFER_SIZE + 2];
#endif
#endif*/

#ifdef ESP32
#if FEATURE_ETHERNET
#define RXD2 5
#define TXD2 17
#endif
#if (defined(HAS_WIFI) || defined(HAS_BLE)) && !defined(PSRAM)
#if !defined(IND)
#define RXD2 17
#define TXD2 16
#elif defined(IND)
#define RXD2 17
#define TXD2 16
//#define RXD2 12
//#define TXD2 14
#endif
#elif defined(PSRAM)
#define RXD2 19
#define TXD2 23
#endif
#endif

#ifdef ESP8266
#define RXD2 15
#define TXD2 13
#endif

void initSerial()
{
  ESPEasy_Console.init();
  #ifdef ESP32_NORMAL
    if (Settings.BaudBit == 1)
      Serial.begin(Settings.BaudRate, SERIAL_7E1);
    else if (Settings.BaudBit == 2)
      Serial.begin(Settings.BaudRate, SERIAL_8O1);
    else
      Serial.begin(Settings.BaudRate, SERIAL_8N1);
  #else
    Serial.begin(Settings.BaudRate);
  #endif
#ifdef ESP32
#if FEATURE_ETHERNET
  if (Settings.BaudBit1 == 1)
    Serial1.begin(Settings.BaudRate1, SERIAL_7N1, RXD2, TXD2);
  else
    Serial1.begin(Settings.BaudRate1, SERIAL_8N1, RXD2, TXD2);
#endif
#ifdef HAS_BLUETOOTH
  if (Settings.BaudBit1 == 1)
    Serial1.begin(Settings.BaudRate1, SERIAL_7N1, RXD2, TXD2);
  else
    Serial1.begin(Settings.BaudRate1, SERIAL_8N1, RXD2, TXD2);
#endif
#ifdef HAS_WIFI
  if (Settings.BaudBit1 == 1)
    Serial1.begin(Settings.BaudRate1, SERIAL_7N1, RXD2, TXD2);
  else if (Settings.BaudBit1 == 2)
    Serial1.begin(Settings.BaudRate1, SERIAL_8O1, RXD2, TXD2);
  else
    Serial1.begin(Settings.BaudRate1, SERIAL_8N1, RXD2, TXD2);
#endif
#endif
#ifdef HAS_BLUETOOTH_2
  Serial2.begin(9600, SERIAL_8N1, 14, 12);
#endif
/*#ifdef ESP32
#ifdef HAS_BLUETOOTH
  bool button_durum;
  switch (Settings.WebAPP) {
    case 120: pinMode(bluetooth_buton, INPUT_PULLUP); button_durum = LOW; break;
    case 121: pinMode(bluetooth_buton, INPUT_PULLUP); button_durum = LOW; break;
    case 130: pinMode(bluetooth_buton, INPUT_PULLDOWN); button_durum = HIGH; break;
    case 131: pinMode(bluetooth_buton, INPUT_PULLDOWN); button_durum = HIGH; break;
    default: pinMode(bluetooth_buton, INPUT_PULLUP); button_durum = LOW; break;
  }

  while (digitalRead(bluetooth_buton) == button_durum) {
    butonbas++;
    if (butonbas > 500) {
      bluetooth_mod = 0;
      pinMode(bluetooth_led, OUTPUT);
      digitalWrite(bluetooth_led, LOW);
      switch (Settings.WebAPP) {
        case 120: 
        case 121: 
          Serial1.println("          VERSION 1.2");
          Serial1.println("\r\n      Bluetooth Mod Pasif!!!\r\n      WiFi AKTiF\r\n");
          Serial1.print("      SSID : ");
          Serial1.println(Settings.Name);
          Serial1.println("      IP Adres : 192.168.4.1");
          Serial1.println("\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
          break;
        case 130: 
        case 131: 
          Serial1.println("SIZE 55 mm, 40 mm");
          Serial1.println("DIRECTION 0,0");
          Serial1.println("REFERENCE 0,0");
          Serial1.println("OFFSET 0 mm");
          Serial1.println("SET PEEL OFF");
          Serial1.println("SET CUTTER OFF");
          Serial1.println("SET TEAR ON");
          Serial1.println("CLS");
          Serial1.println("CODEPAGE 857");
          Serial1.println("TEXT 400,280,\"0\",180,1,1,\"VERSION 1.1\"");
          Serial1.println("TEXT 400,240,\"0\",180,1,1,\"Bluetooth Mod Pasif!!!\"");
          Serial1.println("TEXT 400,200,\"0\",180,1,1,\"WiFi AKTiF\"");
          Serial1.print("TEXT 400,160,\"0\",180,1,1,\"SSID : ");
          Serial1.print(Settings.Name);
          Serial1.println("\"");
          Serial1.println("TEXT 380,110,\"0\",180,1,1,\"IP Adres : 192.168.4.1\"");
          Serial1.println("PRINT 1,1");
          break;
      }
      break;
    }
  }
  //addLog(LOG_LEVEL_INFO, F(butonbas));
  //addLog(LOG_LEVEL_INFO, F(Settings.bluetooth_mod));
  if (butonbas < 500) {
    if (Settings.bluetooth_mod == 1) {
      setWifiMode(WIFI_OFF);
      //addLog(LOG_LEVEL_INFO, F(Settings.bluetooth_mac_address));
      int data[6];
      sscanf(String(Settings.bluetooth_mac_address).c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &data[0], &data[1], &data[2], &data[3], &data[4], &data[5]);
      address[0] = (uint8_t)data[0];
      address[1] = (uint8_t)data[1];
      address[2] = (uint8_t)data[2];
      address[3] = (uint8_t)data[3];
      address[4] = (uint8_t)data[4];
      address[5] = (uint8_t)data[5];
      SerialBT.begin("ENFi32", true);
      SerialBT.setPin(BTpin);
      //sscanf(Settings.bluetooth_mac_address, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &address[0], &address[1], &address[2], &address[3], &address[4], &address[5]);
      //String bluetooth_mac_address;
      //strncpy(Settings.bluetooth_mac_address, bluetooth_mac_address.c_str(), sizeof(Settings.bluetooth_mac_address));
      //uint8_t adres[6] = {0x48, 0xE7, 0x29, 0x95, 0xDA, 0x32};
      BTconnected = SerialBT.connect(address);
      //Serial1.println((char*)address);
      if(BTconnected) {
        Serial1.println("Connected Successfully!");
      } else {
        while(!SerialBT.connected(10000)) {
          Serial1.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
        }
      }
      // Disconnect() may take up to 10 secs max
      if (SerialBT.disconnect()) {
        Serial1.println("Disconnected Successfully!");
      }
      // This would reconnect to the slaveName(will use address, if resolved) or address used with connect(slaveName/address).
      SerialBT.connect();
      *//*if(BTconnected) {
        Serial.println("Reconnected Successfully!");
      } else {
        while(!SerialBT.connected(10000)) {
          Serial1.println("Failed to reconnect. Make sure remote device is available and in range, then restart app.");
        }
      }*/
/*
      if (BTconnected) {
        Serial1.println("Bluetooth Baglanmadi!");
        //pinMode(bluetooth_led, OUTPUT);
        //digitalWrite(bluetooth_led, LOW);
      } else {
        if (!SerialBT.connected(10000)) {
          addLog(LOG_LEVEL_INFO, F("Failed to connect. Make sure remote device is available and in range, then restart app."));
          Serial1.println(F("Failed to connect. Make sure remote device is available and in range, then restart app."));
        }
      }
      if (SerialBT.disconnect()) {
        addLog(LOG_LEVEL_INFO, F("Bluetooth Baglandi!"));
        SerialBT.connect(address);
      }
      SerialBT.connect();*/
      /*if(BTconnected) {
        Serial.println("Reconnected Successfully!");
      } else {
        while(!SerialBT.connected(10000)) {
          Serial.println("Failed to reconnect. Make sure remote device is available and in range, then restart app.");
        }
      }*/
    /*}
    if (Settings.bluetooth_mod == 2) {
      //WiFi.mode(WIFI_OFF);
      setWifiMode(WIFI_OFF);
      setAP(false);
      SerialBT.begin(String(Settings.Name), false);
      addLog(LOG_LEVEL_INFO, F("SerialBT Server Aktif"));
      Serial.println("SerialBT Server Aktif");
      //pinMode(bluetooth_led, OUTPUT);
      //digitalWrite(bluetooth_led, HIGH);
    }
  } //else {
  //}
#endif
#endif*/
}

#ifdef ESP32_NORMAL
void serial2() {
  if (Serial2.available()) {
    String dummy;
    if (PluginCall(PLUGIN_SERIALBT_IN, 0, dummy)) {
      return;
    }
  }

  //if (!Settings.UseSerialBT) { return; }

  while (Serial2.available()) {
    Serial2InByte = Serial2.read();
    if ((escpos_mod) || (tspl_mod)) {
      if ((escpos_time > millis()) || (tspl_time > millis()))
        Serial1.write(Serial2InByte);
      else {
        tspl_mod = false;
        escpos_mod = false;
      }
    } else {
      if (Serial2InByte == 255)  // binary data...
      {
        Serial2.flush();
        return;
      }
      if (Serial2InByteCounter < INPUT_BUFFER_SIZE) {  // add char to string if it still fits
        InputBuffer_Serial2[Serial2InByteCounter++] = Serial2InByte;
      }
        if (Serial2InByte == '\n') {
        if (Serial2InByteCounter == 0) {  // empty command?
          break;
        }
        InputBuffer_Serial2[Serial2InByteCounter] = 0;  // serial data completed
        //SerialBT.println(InputBuffer_Serial);
        ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SERIALBT, InputBuffer_Serial2);
        //ExecuteCommand_internal(EventValueSource::Enum::VALUE_SOURCE_SERIALBT, InputBuffer_SerialBT);
        Serial2InByteCounter = 0;
        InputBuffer_Serial2[0] = 0;  // serial data processed, clear buffer
      }
    }
  }
}
#endif

/*#ifdef ESP32
#ifdef HAS_BLUETOOTH
void serialBT() {
  if (SerialBT.available()) {
    String dummy;
    if (PluginCall(PLUGIN_SERIALBT_IN, 0, dummy)) {
      return;
    }
  }

  if (!Settings.UseSerialBT) { return; }

  while (SerialBT.available()) {
    SerialInByteBT = SerialBT.read();
    if ((escpos_mod) || (tspl_mod)) {
      if ((escpos_time > millis()) || (tspl_time > millis()))
        Serial1.write(SerialInByteBT);
      else {
        tspl_mod = false;
        escpos_mod = false;
      }
    } else {
      if (SerialInByteBT == 255)  // binary data...
      {
        SerialBT.flush();
        return;
      }
      if (SerialInByteCounterBT < INPUT_BUFFER_SIZE) {  // add char to string if it still fits
        InputBuffer_SerialBT[SerialInByteCounterBT++] = SerialInByteBT;
      }
        if (SerialInByteBT == '\n') {
        if (SerialInByteCounterBT == 0) {  // empty command?
          break;
        }
        InputBuffer_SerialBT[SerialInByteCounterBT] = 0;  // serial data completed
        if (Settings.bluetooth_mod == 1) {
          Serial.print("\nQRKOD : ");
          Serial.write(InputBuffer_SerialBT);
          Serial.print("\n");
          Serial.print("NET : ");
          Serial.write(XML_NET_C);
          Serial.println();
        }
        //SerialBT.println(InputBuffer_Serial);
        //ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SERIALBT, InputBuffer_SerialBT);
        else
          ExecuteCommand_internal(EventValueSource::Enum::VALUE_SOURCE_SERIALBT, InputBuffer_SerialBT);
        SerialInByteCounterBT = 0;
        InputBuffer_SerialBT[0] = 0;  // serial data processed, clear buffer
      }
    }
  }
}
#endif
#endif*/

#ifdef ESP32
#if defined(HAS_WIFI) || FEATURE_ETHERNET
void serial1() {
  if (Serial1.available()) {
    String dummy;

    if (PluginCall(PLUGIN_SERIAL_IN, 0, dummy)) {
      return;
    }
  }

  if (!Settings.UseSerial1 || activeTaskUseSerial0()) { return; }

  while (Serial1.available()) {
    delay(0);
    SerialInByte = Serial1.read();

    if (SerialInByte == 255)  // binary data...
    {
      Serial1.flush();
      return;
    }

    if (isprint(SerialInByte)) {
      if (SerialInByteCounter < INPUT_BUFFER_SIZE) {  // add char to string if it still fits
        InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;
      }
    }

    //if ((SerialInByte == '\r') || (SerialInByte == '\n')) {
    if (SerialInByte == '\n') {  
      if (SerialInByteCounter == 0) {  // empty command?
        break;
      }
      InputBuffer_Serial[SerialInByteCounter] = 0;  // serial data completed
      //Serial1.write('>');
      //serialPrintln(InputBuffer_Serial);
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SERIAL, InputBuffer_Serial);
      SerialInByteCounter = 0;
      InputBuffer_Serial[0] = 0;  // serial data processed, clear buffer
    }
  }
}
#endif
#endif
/*#if FEATURE_ETHERNET 
void serial1() {
  if (Serial1.available()) {
    String dummy;

    if (PluginCall(PLUGIN_SERIAL_IN, 0, dummy)) {
      return;
    }
  }

  if (!Settings.UseSerial1 || activeTaskUseSerial0()) { return; }

  while (Serial1.available()) {
    delay(0);
    SerialInByte = Serial1.read();

    if (SerialInByte == 255)  // binary data...
    {
      Serial1.flush();
      return;
    }

    if (isprint(SerialInByte)) {
      if (SerialInByteCounter < INPUT_BUFFER_SIZE) {  // add char to string if it still fits
        InputBuffer_Serial[SerialInByteCounter++] = SerialInByte;
      }
    }

    if ((SerialInByte == '\r') || (SerialInByte == '\n')) {
      if (SerialInByteCounter == 0) {  // empty command?
        break;
      }
      InputBuffer_Serial[SerialInByteCounter] = 0;  // serial data completed
      Serial1.write('>');
      //serialPrintln(InputBuffer_Serial);
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_SERIAL, InputBuffer_Serial);
      SerialInByteCounter = 0;
      InputBuffer_Serial[0] = 0;  // serial data processed, clear buffer
    }
  }
}
#endif
#endif*/

void serial()
{
  ESPEasy_Console.loop();
}

bool process_serialWriteBuffer()
{
  return ESPEasy_Console.process_serialWriteBuffer();
}

// For now, only send it to the serial buffer and try to process it.
// Later we may want to wrap it into a log.
void serialPrint(const __FlashStringHelper *text) {
  ESPEasy_Console.addToSerialBuffer(text);
}

void serialPrint(const String& text) {
  ESPEasy_Console.addToSerialBuffer(text);
}

void serialPrintln(const __FlashStringHelper *text) {
  ESPEasy_Console.addToSerialBuffer(text);
  serialPrintln();
}

void serialPrintln(const String& text) {
  ESPEasy_Console.addToSerialBuffer(text);
  serialPrintln();
}

void serialPrintln() {
  ESPEasy_Console.addNewlineToSerialBuffer();
  ESPEasy_Console.process_serialWriteBuffer();
}

// Do not add helper functions for other types, since those types can only be
// explicit matched at a constructor, not a function declaration.

/*
   void serialPrint(char c) {
   serialPrint(String(c));
   }


   void serialPrint(unsigned long value) {
   serialPrint(String(value));
   }

   void serialPrint(long value) {
   serialPrint(String(value));
   }

   void serialPrintln(unsigned long value) {
   serialPrintln(String(value));
   }
 */
