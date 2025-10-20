/*#include "src/DataStructs/DeviceStruct.h"
#ifdef USES_P105
#ifdef ESP32
//#######################################################################################################
//######################## Plugin 105: Temperature and Humidity sensor BLE Mijia ########################
//#######################################################################################################
#define PLUGIN_105
#define PLUGIN_ID_105         105
#define PLUGIN_NAME_105       "Environment - BLE Mijia"
#define PLUGIN_VALUENAME1_105 "Barkod"

char bleServerAddress[18];

#define bleServerName "BarCode Bluetooth BLE"

static BLEUUID bmeServiceUUID            ("0000FEEA-0000-1000-8000-00805f9b34fb");
static BLEUUID barcodeCharacteristicUUID ("00002AA1-0000-1000-8000-00805f9b34fb");192

boolean doConnect = false;
boolean connected = false;
boolean BLE_Set   = true;

const uint8_t notificationOn[]  = {0x1, 0x0};
const uint8_t notificationOff[] = {0x0, 0x0};

boolean newbarcode = false;

bool connectToServer(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient(); 
  pClient->connect(pAddress);
  BLERemoteService* pRemoteService = pClient->getService(bmeServiceUUID);
  if (pRemoteService == nullptr) {
    return (false);
  }
  barcodeCharacteristic = pRemoteService->getCharacteristic(barcodeCharacteristicUUID);
  if (barcodeCharacteristic == nullptr) {
    return false;
  }
  barcodeCharacteristic->registerForNotify(barcodeNotifyCallback);
  return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    //if (advertisedDevice.getAddress() == BLEAddress(bleServerAddress)) {
    if (advertisedDevice.getName() == bleServerName) { //Check if the name of the advertiser matches
      advertisedDevice.getScan()->stop(); //Scan can be stopped, we found what we are looking for
      pServerAddress = new BLEAddress(bleServerAddress);//advertisedDevice.getAddress()); //Address of advertiser is the one we need
      //pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true; //Set indicator, stating that we are ready to connect
    }
  }
};

void barcodeNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  XML_QRKOD_S = "";
  for (int say = 0; say < length; say++) {
    if (char(pData[say-1]) == 10)
      break;
    XML_QRKOD_S += char(pData[say-1]); 
  }
  Serial.println(XML_QRKOD_S);
  newbarcode = true;
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
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 1; //3rd is the battery
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
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
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&bleServerAddress, sizeof(bleServerAddress));
        addFormTextBox( F("BLE Address"), F("p105_addr"), bleServerAddress, 18);
        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        String tmpString = web_server.arg(F("p105_addr"));
        strncpy(bleServerAddress, tmpString.c_str(), sizeof(bleServerAddress));
        SaveCustomTaskSettings(event->TaskIndex, (byte*)&bleServerAddress, sizeof(bleServerAddress));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&bleServerAddress, sizeof(bleServerAddress));
        BLEDevice::init("BLE");
        BLEScan* pBLEScan = BLEDevice::getScan();
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
        //pBLEScan->setInterval(1349);
        //pBLEScan->setWindow(449);
        pBLEScan->setActiveScan(false);
        pBLEScan->start(30);
        success = true;
        break;
      }

    case PLUGIN_READ:
      {
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&bleServerAddress, sizeof(bleServerAddress));
        if (doConnect == true) {
           if (connectToServer(*pServerAddress)) {
              Serial.println("We are now connected to the BLE Server.");
              barcodeCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
              connected = true;
           } 
           else
             Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
           doConnect = false;
        }
        if (newbarcode) {
          BLEDevice::deinit(true);
          newbarcode = false;
        }
        break;
      }
  }
  return success;
}

#endif
#endif // USES_P105*/