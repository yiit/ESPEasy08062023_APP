#include "../Helpers/Misc.h"

#include "../Commands/InternalCommands.h"

#include "../../ESPEasy-Globals.h"
#include "../../ESPEasy_common.h"
#include "../../_Plugin_Helper.h"
#include "../ESPEasyCore/ESPEasy_backgroundtasks.h"
#include "../ESPEasyCore/Serial.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/Statistics.h"
#include "../Helpers/ESPEasy_FactoryDefault.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/PeriodicalActions.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringParser.h"

#if FEATURE_SD
#include <SD.h>
#endif


bool remoteConfig(struct EventStruct *event, const String& string)
{
  // FIXME TD-er: Why have an event here as argument? It is not used.
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("remoteConfig"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  bool   success = false;
  String command = parseString(string, 1);

  if (equals(command, F("config")))
  {
    // Command: "config,task,<taskname>,<actual Set Config command>"
    if (equals(parseString(string, 2), F("task")))
    {
      String configTaskName = parseStringKeepCase(string, 3);

      // FIXME TD-er: This command is not using the tolerance setting
      // tolerantParseStringKeepCase(Line, 4);
      String configCommand = parseStringToEndKeepCase(string, 4);

      if ((configTaskName.isEmpty()) || (configCommand.isEmpty())) {
        return success;
      }
      taskIndex_t index = findTaskIndexByName(configTaskName);

      if (validTaskIndex(index))
      {
        event->setTaskIndex(index);
        success = PluginCall(PLUGIN_SET_CONFIG, event, configCommand);
      }
    } else {
      addLog(LOG_LEVEL_ERROR, F("Expected syntax: config,task,<taskname>,<config command>"));
    }
  }
  return success;
}

/********************************************************************************************\
   delay in milliseconds with background processing
 \*********************************************************************************************/
void delayBackground(unsigned long dsdelay)
{
  unsigned long timer = millis() + dsdelay;

  while (!timeOutReached(timer)) {
    backgroundtasks();
  }
}

/********************************************************************************************\
   Toggle controller enabled state
 \*********************************************************************************************/
bool setControllerEnableStatus(controllerIndex_t controllerIndex, bool enabled)
{
  if (!validControllerIndex(controllerIndex)) { return false; }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("setControllerEnableStatus"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  // Only enable controller if it has a protocol configured
  if ((Settings.Protocol[controllerIndex] != 0) || !enabled) {
    Settings.ControllerEnabled[controllerIndex] = enabled;
    return true;
  }
  return false;
}

/********************************************************************************************\
   Toggle task enabled state
 \*********************************************************************************************/
bool setTaskEnableStatus(struct EventStruct *event, bool enabled)
{
  if (!validTaskIndex(event->TaskIndex)) { return false; }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("setTaskEnableStatus"));
  #endif // ifndef BUILD_NO_RAM_TRACKER

  // Only enable task if it has a Plugin configured
  if (validPluginID(Settings.TaskDeviceNumber[event->TaskIndex]) || !enabled) {
    String dummy;

    if (!enabled) {
      PluginCall(PLUGIN_EXIT, event, dummy);
    }
    Settings.TaskDeviceEnabled[event->TaskIndex] = enabled;

    if (enabled) {
      if (!PluginCall(PLUGIN_INIT, event, dummy)) {
        return false;
      }

      // Schedule the task to be executed almost immediately
      Scheduler.schedule_task_device_timer(event->TaskIndex, millis() + 10);
    }
    return true;
  }
  return false;
}

/********************************************************************************************\
   Clear task settings for given task
 \*********************************************************************************************/
void taskClear(taskIndex_t taskIndex, bool save)
{
  if (!validTaskIndex(taskIndex)) { return; }
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("taskClear"));
  #endif // ifndef BUILD_NO_RAM_TRACKER
  Settings.clearTask(taskIndex);
  clearTaskCache(taskIndex); // Invalidate any cached values.
  ExtraTaskSettings.clear(); 
  ExtraTaskSettings.TaskIndex = taskIndex;

  if (save) {
    addLog(LOG_LEVEL_INFO, F("taskClear() save settings"));
    SaveTaskSettings(taskIndex);
    SaveSettings();
  }
}

/********************************************************************************************\
   check the program memory hash
   The const MD5_MD5_MD5_MD5_BoundariesOfTheSegmentsGoHere... needs to remain unchanged as it will be replaced by
   - 16 bytes md5 hash, followed by
   - 4 * uint32_t start of memory segment 1-4
   - 4 * uint32_t end of memory segment 1-4
   currently there are only two segemts included in the hash. Unused segments have start adress 0.
   Execution time 520kb @80Mhz: 236ms
   Returns: 0 if hash compare fails, number of checked bytes otherwise.
   The reference hash is calculated by a .py file and injected into the binary.
   Caution: currently the hash sits in an unchecked segment. If it ever moves to a checked segment, make sure
   it is excluded from the calculation !
 \*********************************************************************************************/
#if defined(ARDUINO_ESP8266_RELEASE_2_3_0)
void dump(uint32_t addr) { // Seems already included in core 2.4 ...
  serialPrint(String(addr, HEX));
  serialPrint(": ");

  for (uint32_t a = addr; a < addr + 16; a++)
  {
    serialPrint(String(pgm_read_byte(a), HEX));
    serialPrint(" ");
  }
  serialPrintln();
}

#endif // if defined(ARDUINO_ESP8266_RELEASE_2_3_0)

/*
   uint32_t progMemMD5check(){
    checkRAM(F("progMemMD5check"));
 #define BufSize 10
    uint32_t calcBuffer[BufSize];
    CRCValues.numberOfCRCBytes = 0;
    memcpy (calcBuffer,CRCValues.compileTimeMD5,16);                                                  // is there still the dummy in memory
       ? - the dummy needs to be replaced by the real md5 after linking.
    if( memcmp (calcBuffer, "MD5_MD5_MD5_",12)==0){                                                   // do not memcmp with CRCdummy
       directly or it will get optimized away.
        addLog(LOG_LEVEL_INFO, F("CRC  : No program memory checksum found. Check output of crc2.py"));
        return 0;
    }
    MD5Builder md5;
    md5.begin();
    for (int l = 0; l<4; l++){                                                                            // check max segments,  if the
       pointer is not 0
        uint32_t *ptrStart = (uint32_t *)&CRCValues.compileTimeMD5[16+l*4];
        uint32_t *ptrEnd =   (uint32_t *)&CRCValues.compileTimeMD5[16+4*4+l*4];
        if ((*ptrStart) == 0) break;                                                                      // segment not used.
        for (uint32_t i = *ptrStart; i< (*ptrEnd) ; i=i+sizeof(calcBuffer)){                              // "<" includes last byte
             for (int buf = 0; buf < BufSize; buf ++){
                calcBuffer[buf] = pgm_read_dword((uint32_t*)i+buf);                                       // read 4 bytes
                CRCValues.numberOfCRCBytes+=sizeof(calcBuffer[0]);
             }
             md5.add(reinterpret_cast<const uint8_t *>(&calcBuffer[0]),(*ptrEnd-i)<sizeof(calcBuffer) ? (*ptrEnd-i):sizeof(calcBuffer) );
                    // add buffer to md5.
                At the end not the whole buffer. md5 ptr to data in ram.
        }
   }
   md5.calculate();
   md5.getBytes(CRCValues.runTimeMD5);
   if ( CRCValues.checkPassed())  {
      addLog(LOG_LEVEL_INFO, F("CRC  : program checksum       ...OK"));
      return CRCValues.numberOfCRCBytes;
   }
   addLog(LOG_LEVEL_INFO, F("CRC  : program checksum       ...FAIL"));
   return 0;
   }
 */

/********************************************************************************************\
   Handler for keeping ExtraTaskSettings up to date using cache
 \*********************************************************************************************/
String getTaskDeviceName(taskIndex_t TaskIndex) {
  return Cache.getTaskDeviceName(TaskIndex);
}

/********************************************************************************************\
   Handler for getting Value Names from TaskIndex

   - value names can be accessed with task variable index
   - maximum number of variables <= defined number of variables in plugin
 \*********************************************************************************************/
String getTaskValueName(taskIndex_t TaskIndex, uint8_t TaskValueIndex) {
  const int valueCount = getValueCountForTask(TaskIndex);
  if (TaskValueIndex < valueCount) {
    return Cache.getTaskDeviceValueName(TaskIndex, TaskValueIndex);
  }
  return EMPTY_STRING;
}

/********************************************************************************************\
   If RX and TX tied together, perform emergency reset to get the system out of boot loops
 \*********************************************************************************************/
void emergencyReset()
{
  // Direct Serial is allowed here, since this is only an emergency task.
  ESPEASY_SERIAL_0.begin(9600);
  ESPEASY_SERIAL_0.write(0xAA);
  ESPEASY_SERIAL_0.write(0x55);
  delay(1);

  if (ESPEASY_SERIAL_0.available() == 2) {
    if ((ESPEASY_SERIAL_0.read() == 0xAA) && (ESPEASY_SERIAL_0.read() == 0x55))
    {
      serialPrintln(F("\n\n\rSystem will reset to factory defaults in 10 seconds..."));
      delay(10000);
      ResetFactory();
    }
  }
}

/********************************************************************************************\
   Delayed reboot, in case of issues, do not reboot with high frequency as it might not help...
 \*********************************************************************************************/
void delayedReboot(int rebootDelay, ESPEasy_Scheduler::IntendedRebootReason_e reason)
{
  // Direct Serial is allowed here, since this is only an emergency task.
  while (rebootDelay != 0)
  {
    serialPrint(F("Delayed Reset "));
    serialPrintln(String(rebootDelay));
    rebootDelay--;
    delay(1000);
  }
  reboot(reason);
}

void reboot(ESPEasy_Scheduler::IntendedRebootReason_e reason) {
  prepareShutdown(reason);
  #if defined(ESP32)
  ESP.restart();
  #else // if defined(ESP32)
  ESP.reset();
  #endif // if defined(ESP32)
}

void FeedSW_watchdog()
{
  #ifdef ESP8266
  ESP.wdtFeed();
  #endif // ifdef ESP8266
}

void SendValueLogger(taskIndex_t TaskIndex)
{
#if !defined(BUILD_NO_DEBUG) || FEATURE_SD
  bool   featureSD = false;
  String logger;
  # if FEATURE_SD
  featureSD = true;
  # endif // if FEATURE_SD

  if (featureSD 
      # ifndef BUILD_NO_DEBUG
      || loglevelActiveFor(LOG_LEVEL_DEBUG)
      #endif
  ) {
    const deviceIndex_t DeviceIndex = getDeviceIndex_from_TaskIndex(TaskIndex);

    if (validDeviceIndex(DeviceIndex)) {
      const uint8_t valueCount = getValueCountForTask(TaskIndex);

      for (uint8_t varNr = 0; varNr < valueCount; varNr++)
      {
        logger += node_time.getDateString('-');
        logger += ' ';
        logger += node_time.getTimeString(':');
        logger += ',';
        logger += Settings.Unit;
        logger += ',';
        logger += getTaskDeviceName(TaskIndex);
        logger += ',';
        logger += getTaskValueName(TaskIndex, varNr);
        logger += ',';
        logger += formatUserVarNoCheck(TaskIndex, varNr);
        logger += F("\r\n");
      }
      # ifndef BUILD_NO_DEBUG
      addLog(LOG_LEVEL_DEBUG, logger);
      #endif
    }
  }
#endif // if !defined(BUILD_NO_DEBUG) || FEATURE_SD

#if FEATURE_SD
  String   filename = patch_fname(F("VALUES.CSV"));
  fs::File logFile  = SD.open(filename, "a+");

  if (logFile) {
    logFile.print(logger);
  }
  logFile.close();
#endif // if FEATURE_SD
}

// #######################################################################################################
// ############################ quite acurate but slow color converter####################################
// #######################################################################################################
// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGB(float H, float S, float I, int rgb[3]) {
  int r, g, b;

  H = fmod(H, 360);                           // cycle H around to 0-360 degrees
  H = 3.14159f * H / static_cast<float>(180); // Convert to radians.
  S = S / 100;
  S = S > 0 ? (S < 1 ? S : 1) : 0;            // clamp S and I to interval [0,1]
  I = I / 100;
  I = I > 0 ? (I < 1 ? I : 1) : 0;

  // Math! Thanks in part to Kyle Miller.
  if (H < 2.09439f) {
    r = 255 * I / 3 * (1 + S * cosf(H) / cosf(1.047196667f - H));
    g = 255 * I / 3 * (1 + S * (1 - cosf(H) / cosf(1.047196667f - H)));
    b = 255 * I / 3 * (1 - S);
  } else if (H < 4.188787f) {
    H = H - 2.09439f;
    g = 255 * I / 3 * (1 + S * cosf(H) / cosf(1.047196667f - H));
    b = 255 * I / 3 * (1 + S * (1 - cosf(H) / cosf(1.047196667f - H)));
    r = 255 * I / 3 * (1 - S);
  } else {
    H = H - 4.188787f;
    b = 255 * I / 3 * (1 + S * cosf(H) / cosf(1.047196667f - H));
    r = 255 * I / 3 * (1 + S * (1 - cosf(H) / cosf(1.047196667f - H)));
    g = 255 * I / 3 * (1 - S);
  }
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
}

// uses H 0..360 S 1..100 I/V 1..100 (according to homie convention)
// Source https://blog.saikoled.com/post/44677718712/how-to-convert-from-hsi-to-rgb-white

void HSV2RGBW(float H, float S, float I, int rgbw[4]) {
  int   r, g, b, w;
  float cos_h, cos_1047_h;

  H = fmod(H, 360);                           // cycle H around to 0-360 degrees
  H = 3.14159f * H / static_cast<float>(180); // Convert to radians.
  S = S / 100;
  S = S > 0 ? (S < 1 ? S : 1) : 0;            // clamp S and I to interval [0,1]
  I = I / 100;
  I = I > 0 ? (I < 1 ? I : 1) : 0;

  if (H < 2.09439f) {
    cos_h      = cosf(H);
    cos_1047_h = cosf(1.047196667f - H);
    r          = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    g          = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    b          = 0;
    w          = 255 * (1 - S) * I;
  } else if (H < 4.188787f) {
    H          = H - 2.09439f;
    cos_h      = cosf(H);
    cos_1047_h = cosf(1.047196667f - H);
    g          = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    b          = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    r          = 0;
    w          = 255 * (1 - S) * I;
  } else {
    H          = H - 4.188787f;
    cos_h      = cosf(H);
    cos_1047_h = cosf(1.047196667f - H);
    b          = S * 255 * I / 3 * (1 + cos_h / cos_1047_h);
    r          = S * 255 * I / 3 * (1 + (1 - cos_h / cos_1047_h));
    g          = 0;
    w          = 255 * (1 - S) * I;
  }

  rgbw[0] = r;
  rgbw[1] = g;
  rgbw[2] = b;
  rgbw[3] = w;
}

// Convert RGB Color to HSV Color
void RGB2HSV(uint8_t r, uint8_t g, uint8_t b, float hsv[3]) {
  float rf     = static_cast<float>(r) / 255.0f;
  float gf     = static_cast<float>(g) / 255.0f;
  float bf     = static_cast<float>(b) / 255.0f;
  float maxval = rf;

  if (gf > maxval) { maxval = gf; }

  if (bf > maxval) { maxval = bf; }
  float minval = rf;

  if (gf < minval) { minval = gf; }

  if (bf < minval) { minval = bf; }
  float h = 0.0f, s, v = maxval;
  float f = maxval - minval;

  s = maxval == 0.0f ? 0.0f : f / maxval;

  if (maxval == minval) {
    h = 0.0f; // achromatic
  } else {
    if (maxval == rf) {
      h = (gf - bf) / f + (gf < bf ? 6.0f : 0.0f);
    } else if (maxval == gf) {
      h = (bf - rf) / f + 2.0f;
    } else if (maxval == bf) {
      h = (rf - gf) / f + 4.0f;
    }
    h /= 6.0f;
  }

  hsv[0] = h * 360.0f;
  hsv[1] = s * 255.0f;
  hsv[2] = v * 255.0f;
}

// Simple bitwise get/set functions

uint8_t get8BitFromUL(uint32_t number, uint8_t bitnr) {
  return (number >> bitnr) & 0xFF;
}

void set8BitToUL(uint32_t& number, uint8_t bitnr, uint8_t value) {
  uint32_t mask     = (0xFFUL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}

uint8_t get4BitFromUL(uint32_t number, uint8_t bitnr) {
  return (number >> bitnr) &  0x0F;
}

void set4BitToUL(uint32_t& number, uint8_t bitnr, uint8_t value) {
  uint32_t mask     = (0x0FUL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}

uint8_t get3BitFromUL(uint32_t number, uint8_t bitnr) {
  return (number >> bitnr) &  0x07;
}

void set3BitToUL(uint32_t& number, uint8_t bitnr, uint8_t value) {
  uint32_t mask     = (0x07UL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}

uint8_t get2BitFromUL(uint32_t number, uint8_t bitnr) {
  return (number >> bitnr) &  0x03;
}

void set2BitToUL(uint32_t& number, uint8_t bitnr, uint8_t value) {
  uint32_t mask     = (0x03UL << bitnr);
  uint32_t newvalue = ((value << bitnr) & mask);

  number = (number & ~mask) | newvalue;
}

float getCPUload() {
  return 100.0f - Scheduler.getIdleTimePct();
}

int getLoopCountPerSec() {
  return loopCounterLast / 30;
}

int getUptimeMinutes() {
  return wdcounter / 2;
}

/******************************************************************************
 * scan an int array of specified size for a value
 *****************************************************************************/
bool intArrayContains(const int arraySize, const int array[], const int& value) {
  for (int i = 0; i < arraySize; i++) {
    if (array[i] == value) { return true; }
  }
  return false;
}

bool intArrayContains(const int arraySize, const uint8_t array[], const uint8_t& value) {
  for (int i = 0; i < arraySize; i++) {
    if (array[i] == value) { return true; }
  }
  return false;
}

#ifndef BUILD_NO_RAM_TRACKER
void logMemUsageAfter(const __FlashStringHelper *function, int value) {
  // Store free memory in an int, as subtracting may sometimes result in negative value.
  // The recorded used memory is not an exact value, as background (or interrupt) tasks may also allocate or free heap memory.
  static int last_freemem = ESP.getFreeHeap();
  const int  freemem_end  = ESP.getFreeHeap();

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log;
    if (log.reserve(128)) {
      log  = F("After ");
      log += function;

      if (value >= 0) {
        log += value;
      }

      while (log.length() < 30) { log += ' '; }
      log += F("Free mem after: ");
      log += freemem_end;

      while (log.length() < 55) { log += ' '; }
      log += F("diff: ");
      log += last_freemem - freemem_end;
      addLogMove(LOG_LEVEL_DEBUG, log);
    }
  }

  last_freemem = freemem_end;
}

#endif // ifndef BUILD_NO_RAM_TRACKER



void indikator_secimi(struct EventStruct* event,long indikator, String plugin_indikator) {
  byte choice1 = indikator;
  if (Settings.UseCasVersion) {
    String options1[5];
    options1[0] = F("CAS EC2");
    options1[1] = F("CAS BI-200");
    options1[2] = F("CAS CI-200");
    options1[3] = F("CAS DB-II");
    options1[4] = F("CAS DH");
    int optionValues1[5];
    optionValues1[0] = 50;
    optionValues1[1] = 51;
    optionValues1[2] = 52;
    optionValues1[3] = 53;
    optionValues1[4] = 54;
    addFormSelector(F("İndikatör"), plugin_indikator, 5, options1, optionValues1, choice1);
  } else {
    String options1[37];
    options1[0] = F("A12E");
    options1[1] = F("BAYKON");
    options1[2] = F("CAS");
    options1[3] = F("ERTE");
    options1[4] = F("SK210 SK330");
    options1[5] = F("ESIT");
    options1[6] = F("RINSTRUM");
    options1[7] = F("SERBEST");
    options1[8] = F("OCS-C E");
    options1[9] = F("OCS-C Y");
    options1[10] = F("OCS-SX");
    options1[11] = F("KELI");
    options1[12] = F("T18");
    options1[13] = F("DH");
    options1[14] = F("SUPER");
    options1[15] = F("JADEVER");
    options1[16] = F("RAW-RCW-RHW-RMW");
    options1[17] = F("TUNAYLAR");
    options1[18] = F("DiNiARGEO");
    options1[19] = F("DiGiDEViCE");
    options1[20] = F("METTLER TOLEDO");
    options1[21] = F("BASTER");
    options1[22] = F("AVERY");
    options1[23] = F("MEGA");
    options1[24] = F("LAUMAS");
    options1[25] = F("RiCELAKE");
    options1[26] = F("RCC");
    options1[27] = F("DT");
    options1[28] = F("OCS-K");
    options1[29] = F("DENSi CM");
    options1[30] = F("RCP");
    options1[31] = F("X12");
    options1[32] = F("DGT-60 Multi");
    options1[33] = F("LEADER");
    options1[34] = F("TEM");
    options1[35] = F("WT-03");
    options1[36] = F("909JR");
    int optionValues1[37];
    optionValues1[0] = 0;
    optionValues1[1] = 1;
    optionValues1[2] = 2;
    optionValues1[3] = 3;
    optionValues1[4] = 4;
    optionValues1[5] = 5;
    optionValues1[6] = 6;
    optionValues1[7] = 7;
    optionValues1[8] = 8;
    optionValues1[9] = 9;
    optionValues1[10] = 10;
    optionValues1[11] = 11;
    optionValues1[12] = 12;
    optionValues1[13] = 13;
    optionValues1[14] = 14;
    optionValues1[15] = 15;
    optionValues1[16] = 16;
    optionValues1[17] = 17;
    optionValues1[18] = 18;
    optionValues1[19] = 19;
    optionValues1[20] = 20;
    optionValues1[21] = 21;
    optionValues1[22] = 22;
    optionValues1[23] = 23;
    optionValues1[24] = 24;
    optionValues1[25] = 25;
    optionValues1[26] = 26;
    optionValues1[27] = 27;
    optionValues1[28] = 28;
    optionValues1[29] = 29;
    optionValues1[30] = 30;
    optionValues1[31] = 31;
    optionValues1[32] = 32;
    optionValues1[33] = 33;
    optionValues1[34] = 34;
    optionValues1[35] = 35;
    optionValues1[36] = 36;
    addFormSelector(F("İndikatör"), plugin_indikator, 37, options1, optionValues1, choice1);
  }
  addFormNumericBox(F("İşaret Byte"), F("isaret_byte_0"), ExtraTaskSettings.TaskDeviceIsaretByte, 0, 255);
  addFormNumericBox(F("Son Byte"), F("son_byte_0"), ExtraTaskSettings.TaskDeviceSonByte, 0, 255);
}

void indikator_secimi_kaydet(struct EventStruct* event, long indikator, boolean duzenle) {
  if (!duzenle) {
    switch (indikator) {
      case 0:
        ExtraTaskSettings.TaskDeviceIsaretByte = 2;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 3;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 9;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //A12
      case 1:
        ExtraTaskSettings.TaskDeviceIsaretByte = 2;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 4;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 10;
        ExtraTaskSettings.TaskDeviceSonByte = 13;
        ExtraTaskSettings.TaskDeviceValueBas[1] = 10;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 16;
        Settings.Tersle = false;
        break;  //BAYKON
      case 2:
        ExtraTaskSettings.TaskDeviceIsaretByte = 9;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 10;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 17;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //CAS
      case 3:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 2;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 7;
        ExtraTaskSettings.TaskDeviceSonByte = 13;
        Settings.Tersle = false;
        break;  //ERTE
      case 4:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 3;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 9;
        ExtraTaskSettings.TaskDeviceSonByte = 13;
        ExtraTaskSettings.TaskDeviceValueBas[1] = 9;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 16;
        Settings.Tersle = false;
        break;  //SK210 SK330
      case 5:
        ExtraTaskSettings.TaskDeviceIsaretByte = 0;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 1;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 7;
        ExtraTaskSettings.TaskDeviceSonByte = 13;
        Settings.Tersle = false;
        break;  //ESİT
      case 6:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 4;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 9;
        ExtraTaskSettings.TaskDeviceSonByte = 3;
        Settings.Tersle = false;
        break;  //RİNSTRUM
      case 8:
        ExtraTaskSettings.TaskDeviceIsaretByte = 0;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 1;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 6;
        ExtraTaskSettings.TaskDeviceSonByte = 127;
        Settings.Tersle = true;
        break;  //OCS-C E
      case 9:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 1;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 7;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //OCS-C Y
      case 10:
        ExtraTaskSettings.TaskDeviceIsaretByte = 18;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 19;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 25;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        ExtraTaskSettings.TaskDeviceValueBas[1] = 11;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 17;
        ExtraTaskSettings.TaskDeviceValueBas[2] = 3;
        ExtraTaskSettings.TaskDeviceValueBit[2] = 9;
        Settings.Tersle = false;
        break;  //OCS-SX
      case 11:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 2;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 8;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //KELİ
      case 12:
        ExtraTaskSettings.TaskDeviceIsaretByte = 6;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 8;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 14;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //T18
      case 13:
        ExtraTaskSettings.TaskDeviceIsaretByte = 2;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 3;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 10;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //DH
      case 14:
        ExtraTaskSettings.TaskDeviceIsaretByte = 0;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 2;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 8;
        ExtraTaskSettings.TaskDeviceSonByte = 61;
        Settings.Tersle = false;
        break;  //SUPER
      case 15:
        ExtraTaskSettings.TaskDeviceIsaretByte = 6;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 8;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 14;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //JADAVER
      //case 16 : ExtraTaskSettings.TaskDeviceIsaretByte =  0; ExtraTaskSettings.TaskDeviceValueBas[0] =  2; ExtraTaskSettings.TaskDeviceValueBit[0] =  8; ExtraTaskSettings.TaskDeviceSonByte = 10; break;
      case 16:
        ExtraTaskSettings.TaskDeviceIsaretByte = 6;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 7;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 14;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //RAW-RCW-RHW-RMW
      case 17:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 4;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 10;
        ExtraTaskSettings.TaskDeviceSonByte = 13;
        Settings.Tersle = false;
        break;  //TUNAYLAR
      case 18:
        ExtraTaskSettings.TaskDeviceIsaretByte = 7;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 8;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 14;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //DİNİ ARGEO
      case 19:
        ExtraTaskSettings.TaskDeviceIsaretByte = 4;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 4;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 9;
        ExtraTaskSettings.TaskDeviceSonByte = 9;
        Settings.Tersle = false;
        break;  //DİGİ DEVİCES
      case 20:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 3;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 10;
        ExtraTaskSettings.TaskDeviceSonByte = 13;
        ExtraTaskSettings.TaskDeviceValueBas[1] = 10;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 17;
        Settings.Tersle = false;
        break;  //METTLER TOLEDO
      case 21:
        ExtraTaskSettings.TaskDeviceIsaretByte = 0;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 1;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 7;
        ExtraTaskSettings.TaskDeviceSonByte = 13;
        Settings.Tersle = false;
        break;  //BASTER
      case 22:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 1;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 8;
        ExtraTaskSettings.TaskDeviceSonByte = 3;
        Settings.Tersle = false;
        break;  //AVERY BERKEL
      case 23:
        ExtraTaskSettings.TaskDeviceIsaretByte = 0;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 1;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 7;
        ExtraTaskSettings.TaskDeviceSonByte = 61;
        Settings.Tersle = true;
        break;  //MEGA
      case 24:
        ExtraTaskSettings.TaskDeviceIsaretByte = 0;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 1;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 6;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //LAUMAS
      case 25:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 3;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 9;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        Settings.Tersle = false;
        break;  //RICE LAKE
      case 26:
        ExtraTaskSettings.TaskDeviceIsaretByte = 4;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 5;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 12;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        ExtraTaskSettings.TaskDeviceValueBas[1] = 5;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 12;
        ExtraTaskSettings.TaskDeviceValueBas[2] = 4;
        ExtraTaskSettings.TaskDeviceValueBit[2] = 12;
        ExtraTaskSettings.TaskDeviceValueBas[3] = 5;
        ExtraTaskSettings.TaskDeviceValueBit[3] = 12;
        ExtraTaskSettings.TaskDeviceValueBas[4] = 4;
        ExtraTaskSettings.TaskDeviceValueBit[4] = 12;
        Settings.Tersle = false;
        break;  //DİKOMSAN RCC
      case 27:
        ExtraTaskSettings.TaskDeviceIsaretByte = 4;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 5;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 12;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        ExtraTaskSettings.TaskDeviceValueBas[3] = 5;
        ExtraTaskSettings.TaskDeviceValueBit[3] = 13;
        Settings.Tersle = false;
        break;  //DT
      case 28:
        ExtraTaskSettings.TaskDeviceIsaretByte = 0;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 0;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 7;
        ExtraTaskSettings.TaskDeviceSonByte = 61;
        Settings.Tersle = false;
        break;  //OCS-K
      case 29:
        ExtraTaskSettings.TaskDeviceIsaretByte = 7;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 0;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 6;
        ExtraTaskSettings.TaskDeviceSonByte = 61;
        Settings.Tersle = true;
        break;  //DENSİ CM
      case 30:
        ExtraTaskSettings.TaskDeviceIsaretByte = 6;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 7;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 14;
        ExtraTaskSettings.TaskDeviceSonByte = 10;
        ExtraTaskSettings.TaskDeviceValueBas[1] = 7;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 14;
        ExtraTaskSettings.TaskDeviceValueBas[6] = 6;
        ExtraTaskSettings.TaskDeviceValueBit[6] = 13;
        ExtraTaskSettings.TaskDeviceValueBas[7] = 6;
        ExtraTaskSettings.TaskDeviceValueBit[7] = 16;
        Settings.Tersle = false;
        break;  //DİKOMSAN RPC
      case 31:
        ExtraTaskSettings.TaskDeviceIsaretByte = 1;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 4;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 10;
        ExtraTaskSettings.TaskDeviceSonByte = 13;
        ExtraTaskSettings.TaskDeviceValueBas[1] = 10;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 16;
        Settings.Tersle = false;
        break;  //X12
      case 32:
        ExtraTaskSettings.TaskDeviceIsaretByte  =  3;
        ExtraTaskSettings.TaskDeviceValueBas[0] =  4;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 11;
        ExtraTaskSettings.TaskDeviceSonByte     = 10;
        //ExtraTaskSettings.TaskDeviceIsaretByte_2 = 18;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 19;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 26;
        Settings.Tersle = false;
        break;  //DİNİ ARGEO DGT60
      case 33:
        ExtraTaskSettings.TaskDeviceIsaretByte  =   0;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 202;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 209;
        ExtraTaskSettings.TaskDeviceSonByte     =  13;
        ExtraTaskSettings.TaskDeviceValueBas[1] = 139;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 147;
        ExtraTaskSettings.TaskDeviceValueBas[2] = 130;
        ExtraTaskSettings.TaskDeviceValueBit[2] = 138;
        ExtraTaskSettings.TaskDeviceValueBas[3] =   8;
        ExtraTaskSettings.TaskDeviceValueBit[3] =  31;
        ExtraTaskSettings.TaskDeviceValueBas[4] =  32;
        ExtraTaskSettings.TaskDeviceValueBit[4] =  55;
        ExtraTaskSettings.TaskDeviceValueBas[5] =  56;
        ExtraTaskSettings.TaskDeviceValueBit[5] =  79;
        ExtraTaskSettings.TaskDeviceValueBas[6] =  80;
        ExtraTaskSettings.TaskDeviceValueBit[6] = 103;
        ExtraTaskSettings.TaskDeviceValueBas[7] = 104;
        ExtraTaskSettings.TaskDeviceValueBit[7] = 129;
        ExtraTaskSettings.TaskDeviceValueBas[8] = 159;
        ExtraTaskSettings.TaskDeviceValueBit[8] = 165;
        ExtraTaskSettings.TaskDeviceValueBas[9] =   0;
        ExtraTaskSettings.TaskDeviceValueBit[9] =   0;
        Settings.Tersle = false;
        break;  //LEADER
      case 34:
        ExtraTaskSettings.TaskDeviceIsaretByte  =  0;
        ExtraTaskSettings.TaskDeviceValueBas[0] =  2;
        ExtraTaskSettings.TaskDeviceValueBit[0] =  9;
        ExtraTaskSettings.TaskDeviceSonByte     = 10;
        ExtraTaskSettings.TaskDeviceValueBas[1] =  2;
        ExtraTaskSettings.TaskDeviceValueBit[1] =  9;
        Settings.Tersle = false;
        break;  //TEM
      case 35:
        ExtraTaskSettings.TaskDeviceIsaretByte  =   1;
        ExtraTaskSettings.TaskDeviceValueBas[0] =   3;
        ExtraTaskSettings.TaskDeviceValueBit[0] =   9;
        ExtraTaskSettings.TaskDeviceSonByte     = 103;
        Settings.Tersle = false;
        break;  //RICE LAKE
      case 36:
        ExtraTaskSettings.TaskDeviceIsaretByte  =  0;
        ExtraTaskSettings.TaskDeviceValueBas[0] =  2;
        ExtraTaskSettings.TaskDeviceValueBit[0] =  9;
        ExtraTaskSettings.TaskDeviceSonByte     = 13;
        Settings.Tersle = false;
        break;  //909JR
      case 50:
        ExtraTaskSettings.TaskDeviceIsaretByte  =  6;
        ExtraTaskSettings.TaskDeviceValueBas[0] =  6;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 14;
        ExtraTaskSettings.TaskDeviceSonByte     = 10;
        ExtraTaskSettings.TaskDeviceValueBas[1] =  6;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 14;
        ExtraTaskSettings.TaskDeviceValueBas[2] =  6;
        ExtraTaskSettings.TaskDeviceValueBit[2] = 14;
        ExtraTaskSettings.TaskDeviceValueBas[3] =  6;
        ExtraTaskSettings.TaskDeviceValueBit[3] = 14;
        ExtraTaskSettings.TaskDeviceValueBas[4] =  6;
        ExtraTaskSettings.TaskDeviceValueBit[4] = 14;
        Settings.Tersle = false;
        break;  //CAS EC2
      case 51:
        ExtraTaskSettings.TaskDeviceIsaretByte  =  6;
        ExtraTaskSettings.TaskDeviceValueBas[0] =  8;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 14;
        ExtraTaskSettings.TaskDeviceSonByte     = 10;
        Settings.Tersle = false;
        break;  //CAS BI-200
      case 52:
        ExtraTaskSettings.TaskDeviceIsaretByte  =  7;
        ExtraTaskSettings.TaskDeviceValueBas[0] =  8;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 16;
        ExtraTaskSettings.TaskDeviceSonByte     = 10;
        Settings.Tersle = false;
        break;  //CAS CI-200
      case 53:
        ExtraTaskSettings.TaskDeviceIsaretByte  = 10;
        ExtraTaskSettings.TaskDeviceValueBas[0] = 11;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 17;
        ExtraTaskSettings.TaskDeviceSonByte     = 44;
        ExtraTaskSettings.TaskDeviceValueBas[1] = 19;
        ExtraTaskSettings.TaskDeviceValueBit[1] = 25;
        Settings.Tersle = false;
        break;  //CAS DB-II
      case 54:
        ExtraTaskSettings.TaskDeviceIsaretByte  =  2;
        ExtraTaskSettings.TaskDeviceValueBas[0] =  3;
        ExtraTaskSettings.TaskDeviceValueBit[0] = 10;
        ExtraTaskSettings.TaskDeviceSonByte     = 10;
        Settings.Tersle = false;
        break;  //DH
    }
  }
  else {
    ExtraTaskSettings.TaskDeviceIsaretByte = getFormItemInt(F("isaret_byte_0"));
    ExtraTaskSettings.TaskDeviceSonByte = getFormItemInt(F("son_byte_0"));
  }
}

void udp_client(struct EventStruct* event, long indikator, String udp_data, int eyz_mod) {
  hataTimer_l = millis();
  if (((udp_data.substring(ExtraTaskSettings.TaskDeviceIsaretByte + 2, ExtraTaskSettings.TaskDeviceIsaretByte + 3) == "3") ||  //daralı sabit eksi
       (udp_data.substring(ExtraTaskSettings.TaskDeviceIsaretByte + 2, ExtraTaskSettings.TaskDeviceIsaretByte + 3) == "2") ||  //darasız sabit eksi
       (udp_data.substring(ExtraTaskSettings.TaskDeviceIsaretByte + 2, ExtraTaskSettings.TaskDeviceIsaretByte + 3) == ":") ||  //darasız haraketli eksi
       (udp_data.substring(ExtraTaskSettings.TaskDeviceIsaretByte + 2, ExtraTaskSettings.TaskDeviceIsaretByte + 3) == ";"))
      && (indikator == 20))  //daralı haraketli eksi
    isaret_f = -1;
  else if (udp_data.substring(ExtraTaskSettings.TaskDeviceIsaretByte + 2, ExtraTaskSettings.TaskDeviceIsaretByte + 3) == "-")
    isaret_f = -1;
  else
    isaret_f = 1;
  webapinettartim = isaret_f * (udp_data.substring(ExtraTaskSettings.TaskDeviceValueBas[0]+2, ExtraTaskSettings.TaskDeviceValueBit[0]+2).toFloat());
  webapidaratartim = (udp_data.substring(ExtraTaskSettings.TaskDeviceValueBas[1], ExtraTaskSettings.TaskDeviceValueBit[1]).toFloat());
  webapibruttartim = webapidaratartim + webapinettartim;
  UserVar[event->BaseVarIndex] = webapinettartim;
  UserVar[event->BaseVarIndex + 1] = webapidaratartim;
  UserVar[event->BaseVarIndex + 2] = webapibruttartim;
  float bol = 1;
  if ((indikator == 1) || (indikator == 3) || (indikator == 4) || (indikator == 20) || (indikator == 31)) {
    switch (ExtraTaskSettings.TaskDeviceValueDecimals[0]) {
      case 0: bol = 1; break;
      case 1: bol = 10; break;
      case 2: bol = 100; break;
      case 3: bol = 1000; break;
    }
  }
  XML_NET_S = String((webapinettartim / bol), int(ExtraTaskSettings.TaskDeviceValueDecimals[0]));
  XML_DARA_S = String((webapidaratartim / bol), int(ExtraTaskSettings.TaskDeviceValueDecimals[1]));
  XML_BRUT_S = String((webapibruttartim / bol), int(ExtraTaskSettings.TaskDeviceValueDecimals[2]));
  XML_ADET_S = String(webapiadet, 0);
  XML_ADET_GRAMAJ_S = String(webapiadetgr, 0);
  XML_PLU_NO_S = String(webapipluno, 0);
  XML_BIRIM_FIYAT_S = String(webapibfiyat, 0);
  XML_TUTAR_S = String(webapitutar, 0);
  XML_TARIH_S = node_time.getDateString('-');
  XML_SAAT_S = node_time.getTimeString(':');
  dtostrf(XML_NET_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[0] - ExtraTaskSettings.TaskDeviceValueBas[0]), ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_NET_C);
  dtostrf(XML_DARA_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[1] - ExtraTaskSettings.TaskDeviceValueBas[1]), ExtraTaskSettings.TaskDeviceValueDecimals[1], XML_DARA_C);
  dtostrf(XML_BRUT_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[2] - ExtraTaskSettings.TaskDeviceValueBas[2]), ExtraTaskSettings.TaskDeviceValueDecimals[2], XML_BRUT_C);
  dtostrf(XML_ADET_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[3] - ExtraTaskSettings.TaskDeviceValueBas[3]), 0, XML_ADET_C);
  if ((eyz_mod == 1) && (webapinettartim > 0.001))
    ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, "eyztek");
}

void serial_error(struct EventStruct* event, int yazdir, String komut) {
  if ((millis() - hataTimer_l) > 2000) {
    //SensorSendAll();
    if ((yazdir == 6) && (oto_yazdir == true)) {
      XML_NET_S    = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], ExtraTaskSettings.TaskDeviceValueBit[0]);
      XML_TARIH_S  = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[1], ExtraTaskSettings.TaskDeviceValueBit[1]);
      XML_SAAT_S   = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[2], ExtraTaskSettings.TaskDeviceValueBit[2]);
      XML_MESAJ1_S = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[3], ExtraTaskSettings.TaskDeviceValueBit[3]);
      XML_MESAJ2_S = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[4], ExtraTaskSettings.TaskDeviceValueBit[4]);
      XML_MESAJ3_S = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[5], ExtraTaskSettings.TaskDeviceValueBit[5]);
      XML_MESAJ4_S = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[6], ExtraTaskSettings.TaskDeviceValueBit[6]);
      XML_MESAJ5_S = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[7], ExtraTaskSettings.TaskDeviceValueBit[7]);
      XML_MESAJ6_S = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[8], ExtraTaskSettings.TaskDeviceValueBit[8]);
      XML_MESAJ7_S = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[9], ExtraTaskSettings.TaskDeviceValueBit[9]);
      //XML_MESAJ8_S = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], ExtraTaskSettings.TaskDeviceValueBit[0]);
      //XML_MESAJ9_S = paketVeri_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], ExtraTaskSettings.TaskDeviceValueBit[0]);
      paketVeri_s = "";
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, komut.c_str());
      oto_yazdir = false;
    } else if ((yazdir == 3) && (oto_yazdir == true)) {
      ExecuteCommand_all(EventValueSource::Enum::VALUE_SOURCE_WEB_FRONTEND, komut.c_str());
      oto_yazdir = false;
    } else {
      XML_NET_S = "ERROR";
      XML_NET_S.toCharArray(XML_NET_C, XML_NET_S.length()+1);
      XML_DARA_S = "ERROR";
      XML_BRUT_S = "ERROR";
      XML_ADET_S = "ERROR";
      XML_ADET_GRAMAJ_S = "ERROR";
      XML_PLU_NO_S = "ERROR";
      XML_BIRIM_FIYAT_S = "ERROR";
      XML_TUTAR_S = "ERROR";
      XML_NET_S_2 = "ERROR";
      XML_DARA_S_2 = "ERROR";
      UserVar[event->BaseVarIndex]     = NAN;
      UserVar[event->BaseVarIndex + 1] = NAN;
      UserVar[event->BaseVarIndex + 2] = NAN;
      UserVar[event->BaseVarIndex + 3] = NAN;
      UserVar[event->BaseVarIndex + 4] = NAN;
      UserVar[event->BaseVarIndex + 5] = NAN;
      UserVar[event->BaseVarIndex + 6] = NAN;
      UserVar[event->BaseVarIndex + 7] = NAN;
      UserVar[event->BaseVarIndex + 8] = NAN;
      UserVar[event->BaseVarIndex + 9] = NAN;
      webapinettartim = 0;
      webapidaratartim = 0;
      webapibruttartim = 0;
      webapiadet = 0;
      tartimdata_s = XML_NET_S;
    }
  }
  //Serial.flush();
}

void tersle(struct EventStruct* event, String data_s) {
  String tartim;
  int karakter_say = ExtraTaskSettings.TaskDeviceValueBit[0] - (ExtraTaskSettings.TaskDeviceValueBas[0]);
  for (int i = karakter_say; i > 0; i--)
    tartim += data_s.substring(((i + ExtraTaskSettings.TaskDeviceValueBas[0]) - 1) , (i + ExtraTaskSettings.TaskDeviceValueBas[0]));
  tartimString_s = tartim;
  XML_NET_S = tartim;
  dtostrf(XML_NET_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[0] - ExtraTaskSettings.TaskDeviceValueBas[0]), ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_NET_C);
}

void isaret(struct EventStruct* event, long indikator, String data_s) {
  if (((data_s.substring(ExtraTaskSettings.TaskDeviceIsaretByte, ExtraTaskSettings.TaskDeviceIsaretByte + 1) == "3") ||  //daralı sabit eksi
       (data_s.substring(ExtraTaskSettings.TaskDeviceIsaretByte, ExtraTaskSettings.TaskDeviceIsaretByte + 1) == "2") ||  //darasız sabit eksi
       (data_s.substring(ExtraTaskSettings.TaskDeviceIsaretByte, ExtraTaskSettings.TaskDeviceIsaretByte + 1) == ":") ||  //darasız haraketli eksi
       (data_s.substring(ExtraTaskSettings.TaskDeviceIsaretByte, ExtraTaskSettings.TaskDeviceIsaretByte + 1) == ";"))    //daralı haraketli eksi
      && ((indikator == 4) || (indikator == 20) || (indikator == 31)))  
    isaret_f = -1;
  else if (data_s.substring(ExtraTaskSettings.TaskDeviceIsaretByte, ExtraTaskSettings.TaskDeviceIsaretByte + 1) == "-")
    isaret_f = -1;
  else
    isaret_f = 1;
}

void formul_seri(struct EventStruct* event, String data_s, long indikator) {
  XML_NET_V = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], ExtraTaskSettings.TaskDeviceValueBit[0]);
  XML_DARA_V = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[1], ExtraTaskSettings.TaskDeviceValueBit[1]);
  XML_BRUT_V = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[2], ExtraTaskSettings.TaskDeviceValueBit[2]);
  if (((indikator == 8) || (indikator == 9)) && ExtraTaskSettings.TaskDeviceValueDecimals[0] == 1) {
    XML_NET_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], ExtraTaskSettings.TaskDeviceValueBit[0]);
    XML_NET_S.replace(")", "0.");
    XML_NET_S.replace("!", "1.");
    XML_NET_S.replace("@", "2.");
    XML_NET_S.replace("#", "3.");
    XML_NET_S.replace("$", "4.");
    XML_NET_S.replace("%", "5.");
    XML_NET_S.replace("^", "6.");
    XML_NET_S.replace("&", "7.");
    XML_NET_S.replace("*", "8.");
    XML_NET_S.replace("(", "9.");
  } else {
    webapinettartim = isaret_f * (data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], ExtraTaskSettings.TaskDeviceValueBit[0]).toFloat());
    webapidaratartim = (data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[1], ExtraTaskSettings.TaskDeviceValueBit[1]).toFloat());
    webapibruttartim = webapidaratartim + webapinettartim;
    webapiadet = (data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[3], ExtraTaskSettings.TaskDeviceValueBit[3]).toFloat());
    webapiadetgr = (data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[4], ExtraTaskSettings.TaskDeviceValueBit[4]).toFloat());
    int carpan = 100;
    switch (int(ExtraTaskSettings.TaskDeviceValueDecimals[5])) {
      case 0: carpan = 1; break;
      case 1: carpan = 10; break;
      case 2: carpan = 100; break;
      case 3: carpan = 1000; break;
    }
    int pluno    = (data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[5], ExtraTaskSettings.TaskDeviceValueBit[5]).toFloat()) * carpan;
    webapipluno  = pluno;
    webapibfiyat = (data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[6], ExtraTaskSettings.TaskDeviceValueBit[6]).toFloat());
    webapitutar  = (data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[7], ExtraTaskSettings.TaskDeviceValueBit[7]).toFloat());
    /*UserVar[event->BaseVarIndex] = webapinettartim;
    UserVar[event->BaseVarIndex + 1] = webapidaratartim;
    UserVar[event->BaseVarIndex + 2] = webapibruttartim;
    UserVar[event->BaseVarIndex + 3] = webapiadet;
    UserVar[event->BaseVarIndex + 4] = webapiadetgr;
    UserVar[event->BaseVarIndex + 5] = webapipluno;
    UserVar[event->BaseVarIndex + 6] = webapibfiyat;
    UserVar[event->BaseVarIndex + 7] = webapitutar;*/
    float bol = 1;
    if ((indikator == 1) || (indikator == 3) || (indikator == 4) || (indikator == 20) || (indikator == 31)) {
      switch (ExtraTaskSettings.TaskDeviceValueDecimals[0]) {
        case 0: bol = 1; break;
        case 1: bol = 10; break;
        case 2: bol = 100; break;
        case 3: bol = 1000; break;
      }
    }
    XML_NET_S = String((webapinettartim / bol), int(ExtraTaskSettings.TaskDeviceValueDecimals[0]));
    XML_DARA_S = String((webapidaratartim / bol), int(ExtraTaskSettings.TaskDeviceValueDecimals[1]));
    XML_BRUT_S = String((webapibruttartim / bol), int(ExtraTaskSettings.TaskDeviceValueDecimals[2]));
    XML_ADET_S = String(webapiadet, 0);
    XML_ADET_GRAMAJ_S = String(webapiadetgr, int(ExtraTaskSettings.TaskDeviceValueDecimals[4]));
    XML_PLU_NO_S = String(webapipluno, 0);
    XML_BIRIM_FIYAT_S = String(webapibfiyat, 0);
    XML_TUTAR_S = String(webapitutar, 0);
  }
  XML_TARIH_S = node_time.getDateString('-');
  XML_SAAT_S = node_time.getTimeString(':');
  event->String1 = XML_NET_S;
  event->String2 = XML_DARA_S;
  event->String3 = XML_BRUT_S;
  event->String4 = XML_ADET_S;
  event->String5 = XML_ADET_GRAMAJ_S;
  dtostrf(XML_NET_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[0] - ExtraTaskSettings.TaskDeviceValueBas[0]), ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_NET_C);
  dtostrf(XML_DARA_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[1] - ExtraTaskSettings.TaskDeviceValueBas[1]), ExtraTaskSettings.TaskDeviceValueDecimals[1], XML_DARA_C);
  dtostrf(XML_BRUT_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[2] - ExtraTaskSettings.TaskDeviceValueBas[2]), ExtraTaskSettings.TaskDeviceValueDecimals[2], XML_BRUT_C);
  dtostrf(XML_ADET_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[3] - ExtraTaskSettings.TaskDeviceValueBas[3]), 0, XML_ADET_C);
}


void formul_kontrol(struct EventStruct* event, String data_s, int yazdir, boolean yazdir_aktif) {
  //if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[7]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[7])) && String(ExtraTaskSettings.TaskDeviceFormula[7]).length() > 1)
  //XML_TUTAR_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[7], ExtraTaskSettings.TaskDeviceValueBit[7]);
  //data_s.toUpperCase();
  if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[7]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[7])) && String(ExtraTaskSettings.TaskDeviceFormula[7]).length() > 1)
    XML_PLU_ADI_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[7], (data_s.length()));
  //else if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[6]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[6])) && String(ExtraTaskSettings.TaskDeviceFormula[6]).length() > 1)
  //XML_BIRIM_FIYAT_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[6], ExtraTaskSettings.TaskDeviceValueBit[6]);
  else if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[6]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[6])) && String(ExtraTaskSettings.TaskDeviceFormula[6]).length() > 1)
    XML_PLU_NO_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[6], (data_s.length()));
  /*else if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[5]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[5])) && String(ExtraTaskSettings.TaskDeviceFormula[5]).length() > 1) {
    int carpan = 100;
    switch (int(ExtraTaskSettings.TaskDeviceValueDecimals[5])) {
      case 0: carpan = 1; break;
      case 1: carpan = 10; break;
      case 2: carpan = 100; break;
      case 3: carpan = 1000; break;
    }
    int pluno = (data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[5], ExtraTaskSettings.TaskDeviceValueBit[5]).toFloat()) * carpan;
    XML_PLU_NO_S = String(pluno);
  }*/
  else if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[5]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[5])) && String(ExtraTaskSettings.TaskDeviceFormula[5]).length() > 1)
    XML_QRKOD_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[5], (data_s.length()));
  else if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[4]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[4])) && String(ExtraTaskSettings.TaskDeviceFormula[4]).length() > 1)
    XML_ADET_GRAMAJ_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[4], ExtraTaskSettings.TaskDeviceValueBit[4]);
  else if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[3]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[3])) && String(ExtraTaskSettings.TaskDeviceFormula[3]).length() > 1)
    XML_ADET_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[3], ExtraTaskSettings.TaskDeviceValueBit[3]);  
  else if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[2]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[2])) && String(ExtraTaskSettings.TaskDeviceFormula[2]).length() > 1)
    XML_BRUT_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[2], ExtraTaskSettings.TaskDeviceValueBit[2]);
  else if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[1]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[1])) && String(ExtraTaskSettings.TaskDeviceFormula[1]).length() > 1)
    XML_DARA_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[1], ExtraTaskSettings.TaskDeviceValueBit[1]);
  else if ((data_s.substring(0, String(ExtraTaskSettings.TaskDeviceFormula[0]).length()) == String(ExtraTaskSettings.TaskDeviceFormula[0])) && String(ExtraTaskSettings.TaskDeviceFormula[0]).length() > 1)
    XML_NET_S = data_s.substring(ExtraTaskSettings.TaskDeviceValueBas[0], ExtraTaskSettings.TaskDeviceValueBit[0]);
  
  /*else if (String(ExtraTaskSettings.TaskDeviceFormula[1]).length() == 0) {
    float dara = XML_BRUT_S.toFloat() - XML_NET_S.toFloat();
    XML_DARA_S = String(dara, int(ExtraTaskSettings.TaskDeviceValueDecimals[0]));
  }  

  else if (String(ExtraTaskSettings.TaskDeviceFormula[2]).length() == 0) {
    float brut = XML_NET_S.toFloat() + XML_DARA_S.toFloat();
    XML_BRUT_S = String(brut, int(ExtraTaskSettings.TaskDeviceValueDecimals[0]));
  }*/
  XML_TARIH_S = node_time.getDateString('-');
  XML_SAAT_S = node_time.getTimeString(':');
  dtostrf(XML_NET_S.toFloat(),  (ExtraTaskSettings.TaskDeviceValueBit[0] - ExtraTaskSettings.TaskDeviceValueBas[0]), ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_NET_C);
  dtostrf(XML_DARA_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[1] - ExtraTaskSettings.TaskDeviceValueBas[1]), ExtraTaskSettings.TaskDeviceValueDecimals[1], XML_DARA_C);
  dtostrf(XML_BRUT_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[2] - ExtraTaskSettings.TaskDeviceValueBas[2]), ExtraTaskSettings.TaskDeviceValueDecimals[2], XML_BRUT_C);
  dtostrf(XML_ADET_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[3] - ExtraTaskSettings.TaskDeviceValueBas[3]), 0, XML_ADET_C);
  UserVar[event->BaseVarIndex] = XML_NET_S.toFloat();
  UserVar[event->BaseVarIndex + 1] = XML_DARA_S.toFloat();
  UserVar[event->BaseVarIndex + 2] = XML_BRUT_S.toFloat();
  UserVar[event->BaseVarIndex + 3] = XML_ADET_S.toFloat();
  UserVar[event->BaseVarIndex + 4] = XML_ADET_GRAMAJ_S.toFloat();
  UserVar[event->BaseVarIndex + 5] = XML_PLU_NO_S.toFloat();
  UserVar[event->BaseVarIndex + 6] = XML_BIRIM_FIYAT_S.toFloat();
  UserVar[event->BaseVarIndex + 7] = XML_TUTAR_S.toFloat();
  if ((yazdir == 3) && (yazdir_aktif))
    oto_yazdir = true;
}

unsigned long str2int(char* string) {
  unsigned long temp = atof(string);
  return temp;
}

unsigned int f_2uint_int1(float float_number) {  // split the float and return first unsigned integer

  union f_2uint {
    float f;
    uint16_t i[2];
  };

  union f_2uint f_number;
  f_number.f = float_number;

  return f_number.i[0];
}

unsigned int f_2uint_int2(float float_number) {  // split the float and return first unsigned integer

  union f_2uint {
    float f;
    uint16_t i[2];
  };

  union f_2uint f_number;
  f_number.f = float_number;

  return f_number.i[1];
}

float f_2uint_float(unsigned int uint1, unsigned int uint2) {  // reconstruct the float from 2 unsigned integers

  union f_2uint {
    float f;
    uint16_t i[2];
  };

  union f_2uint f_number;
  f_number.i[0] = uint1;
  f_number.i[1] = uint2;

  return f_number.f;
}