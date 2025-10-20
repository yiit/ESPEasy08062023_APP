#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P105

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
//######################## Plugin 105: Temperature and Humidity sensor BLE Mijia ########################
//#######################################################################################################
//
// Based on https://github.com/dtony/Mijia-ESP32-Bridge/blob/master/src/main.cpp
//
#define PLUGIN_105
#define PLUGIN_ID_105         105
#define PLUGIN_NAME_105       "Environment - BLE Mijia"
#define PLUGIN_VALUENAME1_105 "Temperature"
#define PLUGIN_VALUENAME2_105 "Humidity"
#define PLUGIN_VALUENAME3_105 "Battery"

#include "BLEDevice.h"
#include "BLERemoteCharacteristic.h"
#include <esp_gap_ble_api.h>

//#define MIJIA_DATA_SERVICE    "0000FEEA-0000-1000-8000-00805f9b34fb"
//#define MIJIA_DATA_CHAR       "00002AA1-0000-1000-8000-00805f9b34fb"
#define MIJIA_DATA_SERVICE "49535343-FE7D-4AE5-8FA9-9FAFD205E455"
#define MIJIA_DATA_CHAR "49535343-8841-43F4-A8D4-ECBE34729BB3"
#define MIJIA_BATTERY_SERVICE "0000180f-0000-1000-8000-00805f9b34fb"
#define MIJIA_BATTERY_CHAR    "00002a19-0000-1000-8000-00805f9b34fb"

#ifndef BLE_ESP32_MULTI
#define BLE_ESP32_MULTI
BLEClient* bleclients[TASKS_MAX];
#endif

#ifndef BLE_NOTIFY_TASK
#define BLE_NOTIFY_TASK
static int ble_notify_task = -1;
#endif

int butonbas_P105 = 0;
boolean BLE_aktif = true;

const uint8_t notificationOn[]  = {0x1, 0x0};

//static void p105_notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
void p105_notifyCallback(
  void* locBLERemoteCharacteristic,
  unsigned char* tData,
  unsigned int plength,
  bool pisNotify) {

  BLERemoteCharacteristic* pBLERemoteCharacteristic = (BLERemoteCharacteristic*)locBLERemoteCharacteristic;

  if (ble_notify_task > -1) {
    XML_QRKOD_S = "";
    String data = String((char *)tData);
    for (int say = 0; say < plength; say++) {
      if (char(tData[say-1]) == 10)
        break;
      XML_QRKOD_S += char(tData[say-1]); 
    }
    //Serial.println(XML_QRKOD_S);
    //float temperature = data.toDouble();
    //float humidity = data.substring(9, 13).toDouble();
    byte varIndex = (ble_notify_task * VARS_PER_TASK);
    //UserVar[varIndex] = temperature;
    //UserVar[varIndex + 1] = humidity;
/*  Serial.println(pBLERemoteCharacteristic->toString().c_str());
    Serial.printf("%.1f / %.1f\n", temperature, humidity);*/
    // Disconnect from BT
    //esp_ble_gap_disconnect(*bleclient->getPeerAddress().getNative());  // OMG just disconnect please
    bleclients[ble_notify_task]->disconnect();
    //struct EventStruct *TempEvent;
    //TempEvent = new EventStruct;
    //TempEvent->TaskIndex = ble_notify_task;
    //TempEvent->sensorType = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
    //sendData(TempEvent);
    //delete TempEvent;
    ble_notify_task = -1;
  }
}

boolean Plugin_105(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_105;
        Device[deviceCount].Type = DEVICE_TYPE_DUMMY;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_TEMP_HUM;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1; //3rd is the battery
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;//true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_105);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_105));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_105));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_105));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        char deviceAddress[18];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceAddress, sizeof(deviceAddress));
        addFormTextBox( F("BLE Address"), F("p105_addr"), deviceAddress, 18);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        char deviceAddress[18];
        String tmpString = web_server.arg(F("p105_addr"));
        strncpy(deviceAddress, tmpString.c_str(), sizeof(deviceAddress));
        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceAddress, sizeof(deviceAddress));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (!BLEDevice::getInitialized())
        {
          BLEDevice::init(Settings.Name);
//          delay(200);
        }
        bleclients[event->TaskIndex] = BLEDevice::createClient();
        Settings.TaskDevicePluginConfigLong[event->TaskIndex][0] = 0;
        pinMode(0,INPUT_PULLUP);
        success = true;
        break;
      }


    case PLUGIN_READ:
      {
        while(digitalRead(0) == LOW) {
          butonbas_P105++;
          //Serial.println(butonbas_P105);
        }
        if (butonbas_P105 > 500) {
          if (BLE_aktif) {
            BLE_aktif = false;
            butonbas_P105;
          }
        }
        if (BLE_aktif) {
          //Serial.println("BLE_aktif");
          char deviceAddress[18];
          boolean notifysetted = false;
          boolean success = false;
          unsigned long *ptimer;
          ptimer = (unsigned long *)&Settings.TaskDevicePluginConfigLong[event->TaskIndex][0];

          LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceAddress, sizeof(deviceAddress));

          String log = F("BLE connection to ");
          log += String(deviceAddress);
          addLog(LOG_LEVEL_DEBUGS, log);
          if (ble_notify_task == -1) {

            if (UserVar[event->BaseVarIndex + 2] < 1 || (millis() > *ptimer)) // request battery status one time per hour
            {
//              Serial.println("BLE Battery requesting");
              esp_ble_gap_set_prefer_conn_params(*BLEDevice::getAddress().getNative(), 0x10, 0x20, 3, 300);
              success = bleclients[event->TaskIndex]->connect(BLEAddress(deviceAddress));
              if (success) {
//                Serial.println("connected");
//                delay(100);
                std::string rValue = bleclients[event->TaskIndex]->getValue(BLEUUID(MIJIA_BATTERY_SERVICE), BLEUUID(MIJIA_BATTERY_CHAR));
                int battery = 0;
                if (rValue.length() > 0) {
                  battery = int((char)rValue[0]);
                }
                UserVar[event->BaseVarIndex + 2] = int(battery);
//              Serial.println(battery);
                *ptimer = millis() + 3600000;
              }
            }
            // Connect to the remote BLE Server
            if (!success) {
              //Serial.println("reconnect");
              //esp_ble_gap_set_prefer_conn_params(*BLEDevice::getAddress().getNative(), 0x10, 0x20, 0, 100);
              esp_ble_gap_set_prefer_conn_params(*BLEDevice::getAddress().getNative(), 0x10, 0x20, 0, 100);
              success = bleclients[event->TaskIndex]->connect(BLEAddress(deviceAddress));
            }
            if (success) {
//            Serial.println("register notify");
              ble_notify_task = event->TaskIndex;           // Serial.println(" - Connected to server");
            // Obtain a reference to the service we are after in the remote BLE server.
              BLERemoteService* pRemoteService = bleclients[event->TaskIndex]->getService(BLEUUID(MIJIA_DATA_SERVICE));
//            delay(100);
              if (pRemoteService != nullptr) {
                //Serial.println(" - Found BLE service");
                //BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(MIJIA_DATA_CHAR));
                BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(MIJIA_DATA_CHAR));
                if (pRemoteCharacteristic != nullptr) {
                  //Serial.println(" - Found our characteristic");
                  if (pRemoteCharacteristic->canNotify()) {
                    //Serial.println(" - Register for notifications");
                    pRemoteCharacteristic->registerForNotify((notify_callback)p105_notifyCallback);
                    //pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
                    notifysetted = true;
                  }
                }
              }
            }

            if (!notifysetted) {
              //Serial.println("force disconnect");
              bleclients[event->TaskIndex]->disconnect();
              ble_notify_task = -1; //release the line
              //esp_ble_gap_disconnect(*bleclient->getPeerAddress().getNative());  // OMG just disconnect please
              //delete bleclient;
              //bleclient = NULL;
            }
          }
//        Serial.println("end of read");
          if (success && notifysetted) {
            log += " success.";
            addLog(LOG_LEVEL_DEBUGS, log);
          } else {
            log += " failed.";
            addLog(LOG_LEVEL_ERRORS, log);
            success = false;
          }
        }
        break;
      }

      case PLUGIN_SERIAL_IN:
      {
        while (Serial.available()) {
          char inChar = Serial.read();
          if (inChar == 255) // binary data...
          {
            Serial.flush();
            break;
          }
          if (isprint(inChar))
            tartimString_s += (String)inChar;
          if (inChar == Settings.son_byte) {
            hataTimer_l = millis();
            webapinettartim  = isaret_f * (tartimString_s.substring(Settings.net_bas_byte, Settings.net_bitis_byte).toFloat());
            webapidaratartim = tartimString_s.substring(Settings.dara_bas_byte, Settings.dara_bitis_byte).toFloat();
            webapibruttartim = webapidaratartim + webapinettartim;
            webapiadet       = (tartimString_s.substring(Settings.adet_bas_byte,   Settings.adet_bitis_byte).toFloat());
            webapiadetgr     = (tartimString_s.substring(Settings.adetgr_bas_byte, Settings.adetgr_bitis_byte).toFloat());
            int pluno        = (tartimString_s.substring(Settings.pluno_bas_byte,  Settings.pluno_bitis_byte).toFloat());
            webapipluno      = pluno;
            webapibfiyat     = (tartimString_s.substring(Settings.bfiyat_bas_byte, Settings.bfiyat_bitis_byte).toFloat());
            webapitutar      = (tartimString_s.substring(Settings.tutar_bas_byte,  Settings.tutar_bitis_byte).toFloat());
            UserVar[event->BaseVarIndex]     = webapinettartim;
            UserVar[event->BaseVarIndex + 1] = webapidaratartim;
            UserVar[event->BaseVarIndex + 2] = webapibruttartim;
            UserVar[event->BaseVarIndex + 3] = webapiadet;
            UserVar[event->BaseVarIndex + 4] = webapiadetgr;
            UserVar[event->BaseVarIndex + 5] = webapipluno;
            UserVar[event->BaseVarIndex + 6] = webapibfiyat;
            UserVar[event->BaseVarIndex + 7] = webapitutar;
            XML_NET_S         = String((webapinettartim ), Settings.nokta_byte);
            XML_DARA_S        = String((webapidaratartim), Settings.nokta_byte);
            XML_BRUT_S        = String((webapibruttartim), Settings.nokta_byte);
            XML_ADET_S        = String(webapiadet,   0);
            XML_ADET_GRAMAJ_S = String(webapiadetgr, 0);
            XML_PLU_NO_S      = String(webapipluno,  0);
            XML_BIRIM_FIYAT_S = String(webapibfiyat, 0);
            XML_TUTAR_S       = String(webapitutar,  0);
            XML_TARIH_S       = node_time.getDateString('-');
            XML_SAAT_S        = node_time.getTimeString(':');
            dtostrf(XML_NET_S.toFloat(),  (Settings.net_bitis_byte  -  Settings.net_bas_byte), Settings.nokta_byte, XML_NET_C);
            dtostrf(XML_DARA_S.toFloat(), (Settings.dara_bitis_byte - Settings.dara_bas_byte), Settings.nokta_byte, XML_DARA_C);
            dtostrf(XML_BRUT_S.toFloat(), (Settings.net_bitis_byte  -  Settings.net_bas_byte), Settings.nokta_byte, XML_BRUT_C);
            dtostrf(XML_ADET_S.toFloat(), (Settings.adet_bitis_byte - Settings.adet_bas_byte), 0, XML_ADET_C);
            String Serial_Data = tartimString_s;
            Serial_Data += XML_QRKOD_S;
            Serial.println(Serial_Data);
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
#endif // USES_105