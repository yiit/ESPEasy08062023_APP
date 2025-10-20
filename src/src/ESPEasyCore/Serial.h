#ifndef ESPEASYCORE_SERIAL_H
#define ESPEASYCORE_SERIAL_H

#include "../../ESPEasy_common.h"

#define INPUT_BUFFER_SIZE          4096

extern int butonbas;
extern uint8_t SerialInByte;
extern int  SerialInByteCounter;
extern char InputBuffer_Serial[INPUT_BUFFER_SIZE + 2];

extern uint8_t Serial2InByte;
extern int  Serial2InByteCounter;
extern char InputBuffer_Serial2[INPUT_BUFFER_SIZE + 2];

#ifdef ESP32
#if FEATURE_ETHERNET || defined(HAS_WIFI)
void serial1();
#ifdef ESP32_NORMAL
void serial2();
#endif
#endif
/*#ifdef HAS_BLUETOOTH
extern uint8_t SerialInByteBT;
extern int SerialInByteCounterBT;
extern char InputBuffer_SerialBT[INPUT_BUFFER_SIZE + 2];

void serialBT();
#endif*/
#endif

void initSerial();

void serial();

bool process_serialWriteBuffer();

// For now, only send it to the serial buffer and try to process it.
// Later we may want to wrap it into a log.
void serialPrint(const __FlashStringHelper *text);
void serialPrint(const String& text);

void serialPrintln(const __FlashStringHelper *text);
void serialPrintln(const String& text);

void serialPrintln();

// Do not add helper functions for other types, since those types can only be
// explicit matched at a constructor, not a function declaration.

/*
   void serialPrint(char c);

   void serialPrint(unsigned long value);

   void serialPrint(long value);

   void serialPrintln(unsigned long value);
 */


#endif // ifndef ESPEASYCORE_SERIAL_H
