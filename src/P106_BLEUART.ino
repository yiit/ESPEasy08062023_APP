#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P106

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
#ifdef ESP32
//#######################################################################################################
//######################## Plugin 106: BLE_UART ########################
//#######################################################################################################
//
// Based on https://github.com/dtony/Mijia-ESP32-Bridge/blob/master/src/main.cpp
//
#define PLUGIN_106
#define PLUGIN_ID_106 106
#define PLUGIN_NAME_106 "BLE_UART"
#define PLUGIN_VALUENAME1_106 "NET"

#define P106_INDIKATOR PCONFIG(0)

#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
#include <NimBLE2904.h>
#include <NimBLEDescriptor.h>
#include <NimBLECharacteristic.h>
#include <NimBLEUUID.h>
#include <NimBLERemoteCharacteristic.h>

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define SCALE_BATTERY_SERVICE "0000180f-0000-1000-8000-00805f9b34fb"
#define SCALE_BATTERY_CHAR "00002a19-0000-1000-8000-00805f9b34fb"

NimBLECharacteristic tartimCharacteristics(SERVICE_UUID, NIMBLE_PROPERTY::NOTIFY);

NimBLECharacteristic batteryCharacteristics(SCALE_BATTERY_CHAR, NIMBLE_PROPERTY::NOTIFY);

BLEServer *pServer = NULL;

int bleled_pin_P106 = 27;
boolean bleled_aktif = true;

int blebuton_pin_P106 = 13;
int blebuton_bas_P106 = 0;
int butonbas_P106 = 0;
boolean BLE_aktif = true;
bool deviceConnected = false;
bool oldDeviceConnected = false;

int battery_pin_P106 = 33;
float ADC_VALUE = 0;
float voltage_value = 0;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      Serial.println("*********");
      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++)
        Serial.print(rxValue[i]);

      Serial.println();
      Serial.println("*********");
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, rxValue.c_str());
    }
  }
};

boolean Plugin_106(byte function, struct EventStruct *event, String &string) {
  boolean success = false;

  switch (function) {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_106;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;  //true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_106);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_106));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormSubHeader(F("İndikatör Ayarları"));
        indikator_secimi(event, PCONFIG_LONG(0), F("plugin_100_indikator"));
        addFormCheckBox(F("İndikatör Data Düzenleme"), F("duzenle"), PCONFIG(4));
        addFormNote(F("<font color='red'>Baslangıç-Bitiş Datasının Değişimine İzin Verir.</font>"));
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG_LONG(0) = getFormItemInt(F("plugin_100_indikator"));
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));

        PCONFIG(4) = isFormItemChecked(F("duzenle"));
        indikator_secimi_kaydet(event, PCONFIG_LONG(0), PCONFIG(4));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!BLEDevice::getInitialized())
          BLEDevice::init(Settings.Name);
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(new MyServerCallbacks());

        BLEService *pService = pServer->createService(SERVICE_UUID);
        BLEService *batteryService = pServer->createService(SCALE_BATTERY_SERVICE);

        pService->addCharacteristic(&tartimCharacteristics);
        //tartimDescriptor.setValue("tartim");
        //tartimCharacteristics.addDescriptor(&tartimDescriptor);

        batteryService->addCharacteristic(&batteryCharacteristics);
        //batteryDescriptor.setValue("battery");
        //batteryCharacteristics.addDescriptor(&batteryDescriptor);

        BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, NIMBLE_PROPERTY::WRITE);

        pRxCharacteristic->setCallbacks(new MyCallbacks());
        pService->start();
        batteryService->start();

        BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pServer->getAdvertising()->start();
        pinMode(blebuton_pin_P106, INPUT_PULLUP);
        pinMode(bleled_pin_P106, OUTPUT);
        pinMode(14, OUTPUT);
        pinMode(12, OUTPUT);
        digitalWrite(14, HIGH);
        digitalWrite(12, HIGH);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        while (digitalRead(blebuton_pin_P106) == LOW) {
          blebuton_bas_P106++;
          Serial.println(blebuton_bas_P106);
          if (blebuton_bas_P106 > butonbas_P106) {
            butonbas_P106 = blebuton_bas_P106 + 100;
            if (bleled_aktif) {
              digitalWrite(bleled_pin_P106, HIGH);
              bleled_aktif = false;
            } else {
              digitalWrite(bleled_pin_P106, LOW);
              bleled_aktif = true;
            }
          }
        }
        ADC_VALUE = analogRead(battery_pin_P106);
        if (blebuton_bas_P106 > 500) {
          if (BLE_aktif) {
            BLE_aktif = false;
            blebuton_bas_P106 = 0;
            BLEDevice::deinit(true);
            deviceConnected = false;
            digitalWrite(bleled_pin_P106, LOW);
          }
        }
        if (BLE_aktif) {
          if (deviceConnected) {
            digitalWrite(bleled_pin_P106, HIGH);
            tartimCharacteristics.setValue(XML_NET_S.c_str());
            tartimCharacteristics.notify();
            voltage_value = (ADC_VALUE * 3.3) / (4095);
            //          int voltage_value_yuzde = (((ADC_VALUE/3.3)*10) * 100)/(4095);
            int voltage_value_yuzde = (100 / (0.75 - 0.2) * (voltage_value - 0.2));
            //Serial.println(voltage_value_yuzde);
            //Serial.println(String(voltage_value));
            if ((voltage_value_yuzde < 100) && (voltage_value_yuzde >= 0)) {
              batteryCharacteristics.setValue(voltage_value_yuzde);
              batteryCharacteristics.notify();
            } else if (voltage_value_yuzde > 100) {
              voltage_value_yuzde = 100;
              batteryCharacteristics.setValue(voltage_value_yuzde);
              batteryCharacteristics.notify();
            }
          }
          if (!deviceConnected && oldDeviceConnected) {
            delay(500);
            pServer->startAdvertising();  // restart advertising
            digitalWrite(bleled_pin_P106, LOW);
            oldDeviceConnected = deviceConnected;
          }
          if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
          }
        }
        break;
      }

    case PLUGIN_ONCE_A_SECOND:
      {
        serial_error(event, 0, "");
        success = true;
        break;
      }

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
            isaret(event, PCONFIG_LONG(0), tartimString_s);
            formul_seri(event, tartimString_s, PCONFIG_LONG(0));
            tartimString_s = "";
            Serial.flush();
          }
        }
        success = true;
        break;
      }
  }
  return success;
}

#endif
#endif  // USES_106