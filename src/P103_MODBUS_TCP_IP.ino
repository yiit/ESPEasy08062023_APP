#include "_Plugin_Helper.h"
#include "src/Helpers/Misc.h"
#include "src/DataTypes/SensorVType.h"

#ifdef USES_P103

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
//#######################################################################################################
//######################## Plugin 103: MODBUS->TCP/IP ########################
//#######################################################################################################
/*
  Plugin written by: Sergio Faustino sjfaustino__AT__gmail.com

  This plugin reads available values of an Eastron SDM120C Energy Meter.
  It will also work with all the other superior model such as SDM220T, SDM230 AND SDM630 series.
*/

#define PLUGIN_103
#define PLUGIN_ID_103 103
#define PLUGIN_NAME_103 "Modbus->TCP/IP"

#define P103_DEV_ID PCONFIG(0)
#define P103_DEV_ID_LABEL PCONFIG_LABEL(0)
#define P103_MODEL PCONFIG(1)
#define P103_MODEL_LABEL PCONFIG_LABEL(1)
#define P103_BAUDRATE PCONFIG(2)
#define P103_BAUDRATE_LABEL PCONFIG_LABEL(2)
#define P103_QUERY1 PCONFIG(3)
#define P103_QUERY2 PCONFIG(4)
#define P103_QUERY3 PCONFIG(5)
#define P103_QUERY4 PCONFIG(6)
#define P103_DEPIN CONFIG_PIN3

#define P103_DEV_ID_DFLT 1
#define P103_MODEL_DFLT 0     // SDM120C
#define P103_BAUDRATE_DFLT 1  // 9600 baud
#define P103_QUERY1_DFLT 0    // Voltage (V)
#define P103_QUERY2_DFLT 1    // Current (A)
#define P103_QUERY3_DFLT 2    // Power (W)
#define P103_QUERY4_DFLT 5    // Power Factor (cos-phi)

#define P103_NR_OUTPUT_VALUES 3
#define P103_NR_OUTPUT_OPTIONS 10
#define P103_QUERY1_CONFIG_POS 3

WiFiServer* sernetServer_103;
WiFiClient sernetClients_103;  //[MAX_SRV_CLIENTS];

#include <ESPeasySerial.h>
#include <SDM.h>  // Requires SDM library from Reaper7 - https://github.com/reaper7/SDM_Energy_Meter/

// These pointers may be used among multiple instances of the same plugin,
// as long as the same serial settings are used.
ESPeasySerial* Plugin_103_SoftSerial = nullptr;
SDM* Plugin_103_SDM = nullptr;
boolean Plugin_103_init = false;


// Forward declaration helper functions
const __FlashStringHelper* p103_getQueryString(uint8_t query);
const __FlashStringHelper* p103_getQueryValueString(uint8_t query);
unsigned int p103_getRegister(uint8_t query, uint8_t model);
float p103_readVal(uint8_t query, uint8_t node, unsigned int model);
long p103_readLongVal(uint8_t query, uint8_t node, unsigned int model);
int p103_writeVal(uint8_t query, uint8_t node, unsigned int model);



boolean Plugin_103(uint8_t function, struct EventStruct* event, String& string) {
  boolean success = false;

  static byte connectionState = 0;

  switch (function) {

    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_103;
        //Device[deviceCount].Type = DEVICE_TYPE_SERIAL_PLUS1;     // connected through 3 datapins
        Device[deviceCount].Type = DEVICE_TYPE_SERIAL;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_QUAD;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = P103_NR_OUTPUT_VALUES;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_103);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        for (uint8_t i = 0; i < VARS_PER_TASK; ++i) {
          if (i < P103_NR_OUTPUT_VALUES) {
            uint8_t choice = PCONFIG(i + P103_QUERY1_CONFIG_POS);
            safe_strncpy(
              ExtraTaskSettings.TaskDeviceValueNames[i],
              p103_getQueryValueString(choice),
              sizeof(ExtraTaskSettings.TaskDeviceValueNames[i]));
          } else {
            ZERO_FILL(ExtraTaskSettings.TaskDeviceValueNames[i]);
          }
        }
        break;
      }

    case PLUGIN_GET_DEVICEGPIONAMES:
      {
        serialHelper_getGpioNames(event);
        event->String3 = formatGpioName_output_optional(F("DE"));
        break;
      }

    case PLUGIN_WEBFORM_SHOW_CONFIG:
      {
        string += serialHelper_getSerialTypeLabel(event);
        success = true;
        break;
      }

    case PLUGIN_SET_DEFAULTS:
      {
        P103_DEV_ID = P103_DEV_ID_DFLT;
        P103_MODEL = P103_MODEL_DFLT;
        P103_BAUDRATE = P103_BAUDRATE_DFLT;
        P103_QUERY1 = P103_QUERY1_DFLT;
        P103_QUERY2 = P103_QUERY2_DFLT;
        P103_QUERY3 = P103_QUERY3_DFLT;
        P103_QUERY4 = P103_QUERY4_DFLT;

        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SHOW_SERIAL_PARAMS:
      {
        if (P103_DEV_ID == 0 || P103_DEV_ID > 247 || P103_BAUDRATE >= 6) {
          // Load some defaults
          P103_DEV_ID = P103_DEV_ID_DFLT;
          P103_MODEL = P103_MODEL_DFLT;
          P103_BAUDRATE = P103_BAUDRATE_DFLT;
          P103_QUERY1 = P103_QUERY1_DFLT;
          P103_QUERY2 = P103_QUERY2_DFLT;
          P103_QUERY3 = P103_QUERY3_DFLT;
          P103_QUERY4 = P103_QUERY4_DFLT;
        }
        {
          String options_baudrate[6];
          for (int i = 0; i < 6; ++i) {
            options_baudrate[i] = String(p103_storageValueToBaudrate(i));
          }
          addFormSelector(F("Baud Rate"), P103_BAUDRATE_LABEL, 6, options_baudrate, nullptr, P103_BAUDRATE);
          addUnit(F("baud"));
        }

        /*if (P103_MODEL == 0 && P103_BAUDRATE > 3)
        addFormNote(F("<span style=\"color:red\"> SDM120 only allows up to 9600 baud with default 2400!</span>"));

      if (P103_MODEL == 3 && P103_BAUDRATE == 0)
        addFormNote(F("<span style=\"color:red\"> SDM630 only allows 2400 to 38400 baud with default 9600!</span>"));
      */
        addFormNumericBox(F("Modbus Address"), P103_DEV_ID_LABEL, P103_DEV_ID, 1, 247);

        if (Plugin_103_SDM != nullptr) {
          addRowLabel(F("Checksum (pass/fail)"));
          String chksumStats;
          chksumStats = Plugin_103_SDM->getSuccCount();
          chksumStats += '/';
          chksumStats += Plugin_103_SDM->getErrCount();
          addHtml(chksumStats);
        }

        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        addFormNumericBox(F("TCP PORT"), F("plugin_103_port"), PCONFIG_LONG(7), 1, 65535);
        {
          const __FlashStringHelper* options_model[2] = { F("Y180"), F("Y200") };
          addFormSelector(F("Model Type"), P103_MODEL_LABEL, 2, options_model, nullptr, P103_MODEL);
        }

        {
          // In a separate scope to free memory of String array as soon as possible
          sensorTypeHelper_webformLoad_header();
          const __FlashStringHelper* options[P103_NR_OUTPUT_OPTIONS];
          for (int i = 0; i < P103_NR_OUTPUT_OPTIONS; ++i) {
            options[i] = p103_getQueryString(i);
          }
          for (uint8_t i = 0; i < P103_NR_OUTPUT_VALUES; ++i) {
            const uint8_t pconfigIndex = i + P103_QUERY1_CONFIG_POS;
            sensorTypeHelper_loadOutputSelector(event, pconfigIndex, i, P103_NR_OUTPUT_OPTIONS, options);
          }
        }


        success = true;
        break;
      }

    case PLUGIN_WEBFORM_SAVE:
      {
        PCONFIG_LONG(7) = getFormItemInt(F("plugin_103_port"));
        // Save output selector parameters.
        for (uint8_t i = 0; i < P103_NR_OUTPUT_VALUES; ++i) {
          const uint8_t pconfigIndex = i + P103_QUERY1_CONFIG_POS;
          const uint8_t choice = PCONFIG(pconfigIndex);
          sensorTypeHelper_saveOutputSelector(event, pconfigIndex, i, p103_getQueryValueString(choice));
        }

        P103_DEV_ID = getFormItemInt(P103_DEV_ID_LABEL);
        P103_MODEL = getFormItemInt(P103_MODEL_LABEL);
        P103_BAUDRATE = getFormItemInt(P103_BAUDRATE_LABEL);
        ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte"));
        ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte"));

        Plugin_103_init = false;  // Force device setup next time
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        if (Plugin_103_SoftSerial != nullptr) {
          delete Plugin_103_SoftSerial;
          Plugin_103_SoftSerial = nullptr;
        }

        Plugin_103_SoftSerial = new (std::nothrow) ESPeasySerial(static_cast<ESPEasySerialPort>(CONFIG_PORT), CONFIG_PIN1, CONFIG_PIN2, false, 128);
        if (Plugin_103_SoftSerial == nullptr) {
          break;
        }
        unsigned int baudrate = p103_storageValueToBaudrate(P103_BAUDRATE);
        Plugin_103_SoftSerial->begin(baudrate);

        if (Plugin_103_SDM != nullptr) {
          delete Plugin_103_SDM;
          Plugin_103_SDM = nullptr;
        }
        Plugin_103_SDM = new (std::nothrow) SDM(*Plugin_103_SoftSerial, baudrate, P103_DEPIN);
        if (Plugin_103_SDM != nullptr) {
          Plugin_103_SDM->begin();
          Plugin_103_init = true;
          success = true;
        }
        if (PCONFIG_LONG(7) != 0) {
          sernetServer_103 = new WiFiServer(PCONFIG_LONG(7));
          sernetServer_103->begin();
          sernetServer_103->setNoDelay(true);
        }
        break;
      }

    case PLUGIN_EXIT:
      {
        Plugin_103_init = false;
        if (Plugin_103_SoftSerial != nullptr) {
          delete Plugin_103_SoftSerial;
          Plugin_103_SoftSerial = nullptr;
        }
        if (Plugin_103_SDM != nullptr) {
          delete Plugin_103_SDM;
          Plugin_103_SDM = nullptr;
        }
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
        if (Plugin_103_init) {
          if (sernetServer_103->hasClient()) {
            if (sernetClients_103) { sernetClients_103.stop(); }
            sernetClients_103 = sernetServer_103->available();
          }
          if (sernetClients_103.connected()) {
            addLogMove(LOG_LEVEL_INFO, "Client Bagli");
            connectionState = 1;
            int bytes_read = 0;
            uint8_t net_buf[32];
            int count = sernetClients_103.available();
            if (count > 0) {
              addLogMove(LOG_LEVEL_INFOS, "client data");
              bytes_read = sernetClients_103.read(net_buf, count);
              String net_data = (char*)net_buf;
              String net_data_s = net_data.substring(0, net_data.length() - 2);
              if (net_data_s == "tare") {
                int model = P103_MODEL;
                uint8_t dev_id = P103_DEV_ID;
                p103_writeVal(P103_QUERY3, dev_id, model);
                addLogMove(LOG_LEVEL_INFOS, "Modbus Dara");
              }
            }
          } else {
            if (connectionState == 1) {
              connectionState = 0;
              sernetClients_103.setTimeout(10);
            }
          }
          success = true;
        }
        break;
      }

    case PLUGIN_READ:
      {
        if (Plugin_103_init) {
          int model = P103_MODEL;
          uint8_t dev_id = P103_DEV_ID;
          UserVar[event->BaseVarIndex] = p103_readVal(P103_QUERY1, dev_id, model);
          UserVar[event->BaseVarIndex + 1] = p103_readVal(P103_QUERY2, dev_id, model);
          if (P103_MODEL == 0) {
            //XML_NET_S = String(float(p103_readVal(P103_QUERY2, dev_id, model)), Settings.nokta_byte);
            //dtostrf(XML_NET_S.toFloat(), (Settings.net_bitis_byte - Settings.net_bas_byte), Settings.nokta_byte, XML_NET_C);
          }
          if (P103_MODEL == 1) {
            switch (Settings.nokta_byte) {
              case 0: bol = 1; break;
              case 1: bol = 10; break;
              case 2: bol = 100; break;
              case 3: bol = 1000; break;
            }
            //XML_NET_S = String(float(p103_readLongVal(P103_QUERY2, dev_id, model)) / bol, Settings.nokta_byte);
            //dtostrf(XML_NET_S.toFloat(), (Settings.net_bitis_byte - Settings.net_bas_byte), Settings.nokta_byte, XML_NET_C);
          }
          if (sernetClients_103) {
            String XML_DATA = "ST,GS,";
            XML_DATA += String(XML_NET_C);
            XML_DATA += "kg";
            sernetClients_103.println(XML_DATA);
            sernetClients_103.flush();
          }
          success = true;
        }
        break;
      }
  }
  return success;
}

float p103_readVal(uint8_t query, uint8_t node, unsigned int model) {
  if (Plugin_103_SDM == nullptr) return 0.0f;

  uint8_t retry_count = 3;
  bool success = false;
  float _tempvar = NAN;
  while (retry_count > 0 && !success) {
    Plugin_103_SDM->clearErrCode();
    _tempvar = Plugin_103_SDM->readVal(p103_getRegister(query, model), node);
    --retry_count;
    if (Plugin_103_SDM->getErrCode() == SDM_ERR_NO_ERROR) {
      success = true;
    }
    addLogMove(LOG_LEVEL_INFO, "Modbus Oku");
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("Modbus->TCP/IP: (");
    log += node;
    log += ',';
    log += model;
    log += F(") ");
    log += p103_getQueryString(query);
    log += F(": ");
    log += _tempvar;
    addLogMove(LOG_LEVEL_INFOS, log);
  }
  return _tempvar;
}

long p103_readLongVal(uint8_t query, uint8_t node, unsigned int model) {
  if (Plugin_103_SDM == nullptr) return 0.0f;

  uint8_t retry_count = 3;
  bool success = false;
  float _tempvar = NAN;
  while (retry_count > 0 && !success) {
    Plugin_103_SDM->clearErrCode();
    _tempvar = Plugin_103_SDM->readLongVal(p103_getRegister(query, model), node);
    --retry_count;
    if (Plugin_103_SDM->getErrCode() == SDM_ERR_NO_ERROR) {
      success = true;
    }
    addLogMove(LOG_LEVEL_INFO, "Modbus Oku");
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("Modbus->TCP/IP: (");
    log += node;
    log += ',';
    log += model;
    log += F(") ");
    log += p103_getQueryString(query);
    log += F(": ");
    log += _tempvar;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  return _tempvar;
}

int p103_writeVal(uint8_t query, uint8_t node, unsigned int model) {
  if (Plugin_103_SDM == nullptr) return 0.0f;

  uint8_t retry_count = 1;
  bool success = false;
  float _tempvar = NAN;
  while (retry_count > 0 && !success) {
    Plugin_103_SDM->clearErrCode();
    if (model == 0)
      _tempvar = Plugin_103_SDM->writeValY180(p103_getRegister(query, model), node);
    else if (model == 1)
      _tempvar = Plugin_103_SDM->writeValY200(p103_getRegister(query, model), node);
    --retry_count;
    if (Plugin_103_SDM->getErrCode() == SDM_ERR_NO_ERROR) {
      success = true;
    }
    addLogMove(LOG_LEVEL_INFO, "Modbus Dara Komut");
    sernetClients_103.println("OK");
  }
  if (loglevelActiveFor(LOG_LEVEL_INFO)) {
    String log = F("Modbus->TCP/IP: (");
    log += node;
    log += ',';
    log += model;
    log += F(") ");
    log += p103_getQueryString(query);
    log += F(": ");
    log += _tempvar;
    addLogMove(LOG_LEVEL_INFO, log);
  }
  return _tempvar;
}

unsigned int p103_getRegister(uint8_t query, uint8_t model) {
  if (model == 0) {  // Y180
    switch (query) {
      case 0: return Y180_ADC;
      case 1: return Y180_NET;
      case 2: return Y180_DARA;
    }
  } else if (model == 1) {  // Y200
    switch (query) {
      case 0: return Y200_ADC;
      case 1: return Y200_NET;
      case 2: return Y200_DARA;
    }
  }
  return 0;
}

const __FlashStringHelper* p103_getQueryString(uint8_t query) {
  switch (query) {
    case 0: return F("ADC");
    case 1: return F("NET");
    case 2: return F("DARA");
  }
  return F("");
}

const __FlashStringHelper* p103_getQueryValueString(uint8_t query) {
  switch (query) {
    case 0: return F("ADC");
    case 1: return F("NET");
    case 2: return F("DARA");
  }
  return F("");
}


int p103_storageValueToBaudrate(uint8_t baudrate_setting) {
  unsigned int baudrate = 9600;
  switch (baudrate_setting) {
    case 0: baudrate = 1200; break;
    case 1: baudrate = 2400; break;
    case 2: baudrate = 4800; break;
    case 3: baudrate = 9600; break;
    case 4: baudrate = 19200; break;
    case 5: baudrate = 38400; break;
  }
  return baudrate;
}



#endif  // USES_P103