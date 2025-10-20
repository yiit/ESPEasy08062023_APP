#include "../Helpers/SystemVariables.h"


#include "../../ESPEasy_common.h"

#include "../../ESPEasy-Globals.h"

#include "../CustomBuild/CompiletimeDefines.h"

#include "../DataStructs/TimingStats.h"

#include "../ESPEasyCore/ESPEasy_Log.h"
#include "../ESPEasyCore/ESPEasyNetwork.h"

#include "../Globals/CRCValues.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/ESPEasyWiFiEvent.h"
#if FEATURE_MQTT
# include "../Globals/MQTT.h"
#endif // if FEATURE_MQTT
#include "../Globals/NetworkState.h"
#include "../Globals/RuntimeData.h"
#include "../Globals/Settings.h"
#include "../Globals/Statistics.h"

#include "../Helpers/Convert.h"
#include "../Helpers/Hardware.h"
#include "../Helpers/Misc.h"
#include "../Helpers/Numerical.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/StringProvider.h"


#if defined(ESP8266)
  # include <ESP8266WiFi.h>
#endif // if defined(ESP8266)
#if defined(ESP32)
  # include <WiFi.h>
#endif // if defined(ESP32)


String getReplacementString(const String& format, String& s) {
  int startpos = s.indexOf(format);
  int endpos   = s.indexOf('%', startpos + 1);
  String R     = s.substring(startpos, endpos + 1);

#ifndef BUILD_NO_DEBUG

  if (loglevelActiveFor(LOG_LEVEL_DEBUG)) {
    String log = F("ReplacementString SunTime: ");
    log += R;
    log += F(" offset: ");
    log += ESPEasy_time::getSecOffset(R);
    addLogMove(LOG_LEVEL_DEBUG, log);
  }
#endif // ifndef BUILD_NO_DEBUG
  return R;
}

void replSunRiseTimeString(const String& format, String& s, boolean useURLencode) {
  String R = getReplacementString(format, s);

  repl(R, node_time.getSunriseTimeString(':', ESPEasy_time::getSecOffset(R)), s, useURLencode);
}

void replSunSetTimeString(const String& format, String& s, boolean useURLencode) {
  String R = getReplacementString(format, s);

  repl(R, node_time.getSunsetTimeString(':', ESPEasy_time::getSecOffset(R)), s, useURLencode);
}

String timeReplacement_leadZero(int value)
{
  char valueString[5] = { 0 };

  sprintf_P(valueString, PSTR("%02d"), value);
  return valueString;
}

// FIXME TD-er: Try to match these with  StringProvider::getValue
LabelType::Enum SystemVariables2LabelType(SystemVariables::Enum enumval) {
  LabelType::Enum label = LabelType::MAX_LABEL;
  switch (enumval)
  {
    case SystemVariables::IP:                label = LabelType::IP_ADDRESS; break;
    case SystemVariables::SUBNET:            label = LabelType::IP_SUBNET; break;
    case SystemVariables::DNS:               label = LabelType::DNS; break;
    case SystemVariables::DNS_1:             label = LabelType::DNS_1; break;
    case SystemVariables::DNS_2:             label = LabelType::DNS_2; break;
    case SystemVariables::GATEWAY:           label = LabelType::GATEWAY; break;
    case SystemVariables::CLIENTIP:          label = LabelType::CLIENT_IP; break;

    #if FEATURE_ETHERNET

    case SystemVariables::ETHWIFIMODE:       label = LabelType::ETH_WIFI_MODE; break; // 0=WIFI, 1=ETH
    case SystemVariables::ETHCONNECTED:      label = LabelType::ETH_CONNECTED; break; // 0=disconnected, 1=connected
    case SystemVariables::ETHDUPLEX:         label = LabelType::ETH_DUPLEX; break;
    case SystemVariables::ETHSPEED:          label = LabelType::ETH_SPEED; break;
    case SystemVariables::ETHSTATE:          label = LabelType::ETH_STATE; break;
    case SystemVariables::ETHSPEEDSTATE:     label = LabelType::ETH_SPEED_STATE; break;
    #endif // if FEATURE_ETHERNET
    case SystemVariables::LCLTIME:           label = LabelType::LOCAL_TIME; break;
    case SystemVariables::MAC:               label = LabelType::STA_MAC; break;
    case SystemVariables::RSSI:              label = LabelType::WIFI_RSSI; break;
    case SystemVariables::SUNRISE_S:         label = LabelType::SUNRISE_S; break;
    case SystemVariables::SUNSET_S:          label = LabelType::SUNSET_S; break;
    case SystemVariables::SUNRISE_M:         label = LabelType::SUNRISE_M; break;
    case SystemVariables::SUNSET_M:          label = LabelType::SUNSET_M; break;
    case SystemVariables::SYSBUILD_DESCR:    label = LabelType::BUILD_DESC; break;
    case SystemVariables::SYSBUILD_FILENAME: label = LabelType::BINARY_FILENAME; break;
    case SystemVariables::SYSBUILD_GIT:      label = LabelType::GIT_BUILD; break;
    case SystemVariables::SYSSTACK:          label = LabelType::FREE_STACK; break;
    case SystemVariables::UNIT_sysvar:       label = LabelType::UNIT_NR; break;
    #if FEATURE_ZEROFILLED_UNITNUMBER
    case SystemVariables::UNIT_0_sysvar:     label = LabelType::UNIT_NR_0; break;
    #endif // FEATURE_ZEROFILLED_UNITNUMBER
    case SystemVariables::FLASH_FREQ:        label = LabelType::FLASH_CHIP_SPEED; break;
    case SystemVariables::FLASH_SIZE:        label = LabelType::FLASH_CHIP_REAL_SIZE; break;
    case SystemVariables::FLASH_CHIP_VENDOR: label = LabelType::FLASH_CHIP_VENDOR; break;
    case SystemVariables::FLASH_CHIP_MODEL:  label = LabelType::FLASH_CHIP_MODEL; break;
    case SystemVariables::FS_SIZE:           label = LabelType::FS_SIZE; break;
    case SystemVariables::FS_FREE:           label = LabelType::FS_FREE; break;

    case SystemVariables::ESP_CHIP_ID:       label = LabelType::ESP_CHIP_ID; break;
    case SystemVariables::ESP_CHIP_FREQ:     label = LabelType::ESP_CHIP_FREQ; break;
    case SystemVariables::ESP_CHIP_MODEL:    label = LabelType::ESP_CHIP_MODEL; break;
    case SystemVariables::ESP_CHIP_REVISION: label = LabelType::ESP_CHIP_REVISION; break;
    case SystemVariables::ESP_CHIP_CORES:    label = LabelType::ESP_CHIP_CORES; break;
    case SystemVariables::ESP_BOARD_NAME:    label = LabelType::ESP_BOARD_NAME; break;

    case SystemVariables::TARIHV:            label = LabelType::XML_TARIH_V; break;
    case SystemVariables::SAATV:             label = LabelType::XML_SAAT_V; break;
    case SystemVariables::STABIL:            label = LabelType::XML_STABIL; break;
    case SystemVariables::NET:               label = LabelType::XML_NET_C; break;
    case SystemVariables::NET_S:             label = LabelType::XML_NET_S; break;
    case SystemVariables::NET_V:             label = LabelType::XML_NET_V; break;
    case SystemVariables::DARA:              label = LabelType::XML_DARA_C; break;
    case SystemVariables::DARA_S:            label = LabelType::XML_DARA_S; break;
    case SystemVariables::DARA_V:            label = LabelType::XML_DARA_V; break;
    case SystemVariables::BRUT:              label = LabelType::XML_BRUT_C; break;
    case SystemVariables::BRUT_S:            label = LabelType::XML_BRUT_S; break;
    case SystemVariables::BRUT_V:            label = LabelType::XML_BRUT_V; break;
    case SystemVariables::ADET:              label = LabelType::XML_ADET; break;
    case SystemVariables::ADETGRAMAJ:        label = LabelType::XML_ADET_GRAMAJ; break;
    case SystemVariables::PLUNO:             label = LabelType::XML_PLU_NO; break;
    case SystemVariables::PLUADI:            label = LabelType::XML_PLU_ADI; break;
    case SystemVariables::PLUKOD:            label = LabelType::XML_PLU_KOD; break;
    case SystemVariables::BARKOD:            label = LabelType::XML_BARKOD; break;
    case SystemVariables::BIRIMFIYAT:        label = LabelType::XML_BIRIM_FIYAT; break;
    case SystemVariables::TUTAR:             label = LabelType::XML_TUTAR; break;
    case SystemVariables::SNO:		           label = LabelType::XML_SNO; break;
    case SystemVariables::SAYAC_1:	         label = LabelType::XML_SAYAC_1; break;
    case SystemVariables::SAYAC_2:           label = LabelType::XML_SAYAC_2; break;
    case SystemVariables::SAYAC_3:           label = LabelType::XML_SAYAC_3; break;
    case SystemVariables::SAYAC_4:           label = LabelType::XML_SAYAC_4; break;
    case SystemVariables::SAYAC_1_C:         label = LabelType::XML_SAYAC_1_C; break;
    case SystemVariables::SAYAC_2_C:         label = LabelType::XML_SAYAC_2_C; break;
    case SystemVariables::SAYAC_3_C:         label = LabelType::XML_SAYAC_3_C; break;
    case SystemVariables::SAYAC_4_C:         label = LabelType::XML_SAYAC_4_C; break;
    case SystemVariables::SERINO:            label = LabelType::XML_SERI_NO; break;
    case SystemVariables::FISNO:	           label = LabelType::XML_FIS_NO; break;               
    case SystemVariables::TOPNET:            label = LabelType::XML_TOP_NET; break;
    case SystemVariables::TOPDARA:           label = LabelType::XML_TOP_DARA; break;
    case SystemVariables::TOPBRUT:           label = LabelType::XML_TOP_BRUT; break;    
    
    case SystemVariables::EAN8:              label = LabelType::XML_EAN8; break;
    case SystemVariables::EAN13:             label = LabelType::XML_EAN13; break; 
    case SystemVariables::QRKOD:             label = LabelType::XML_QRKOD; break;
    
    case SystemVariables::FIRMAADI:          label = LabelType::XML_FIRMA_ADI; break;
    case SystemVariables::PLAKANO:           label = LabelType::XML_PLAKA_NO; break;
    case SystemVariables::MUSTERIADI:        label = LabelType::XML_MUSTERI_ADI; break;
    case SystemVariables::OPERATORADI:       label = LabelType::XML_OPERATOR; break;
    case SystemVariables::AGIRLIK:           label = LabelType::XML_NET_V; break;

    case SystemVariables::KFONT1:            label = LabelType::KFONT1; break;
    case SystemVariables::KFONT2:            label = LabelType::KFONT2; break;
    case SystemVariables::KFONT3:            label = LabelType::KFONT3; break;
    case SystemVariables::KFONT4:            label = LabelType::KFONT4; break;
    case SystemVariables::KFONT5:            label = LabelType::KFONT5; break;
    case SystemVariables::KFONT6:            label = LabelType::KFONT6; break;

    case SystemVariables::FONT1:             label = LabelType::FONT1; break;
    case SystemVariables::FONT2:             label = LabelType::FONT2; break;
    case SystemVariables::FONT3:             label = LabelType::FONT3; break;
    case SystemVariables::FONT4:             label = LabelType::FONT4; break;
    case SystemVariables::FONT5:             label = LabelType::FONT5; break;
    case SystemVariables::FONT6:             label = LabelType::FONT6; break;

    case SystemVariables::FISBAS1:           label = LabelType::XML_FISBAS1; break;
    case SystemVariables::FISBAS2:           label = LabelType::XML_FISBAS2; break;
    case SystemVariables::FISBAS3:           label = LabelType::XML_FISBAS3; break;
    case SystemVariables::FISBAS4:           label = LabelType::XML_FISBAS4; break;

    case SystemVariables::V0:                label = LabelType::XML_V0; break;
    case SystemVariables::V1:                label = LabelType::XML_V1; break;
    case SystemVariables::V2:                label = LabelType::XML_V2; break;
    case SystemVariables::V3:                label = LabelType::XML_V3; break;
    case SystemVariables::V4:                label = LabelType::XML_V4; break;
    case SystemVariables::V5:                label = LabelType::XML_V5; break;
    case SystemVariables::V6:                label = LabelType::XML_V6; break;
    case SystemVariables::V7:                label = LabelType::XML_V7; break;
    case SystemVariables::V8:                label = LabelType::XML_V8; break;
    case SystemVariables::V9:                label = LabelType::XML_V9; break;
    case SystemVariables::V10:               label = LabelType::XML_V10; break;
    case SystemVariables::V11:               label = LabelType::XML_V11; break;
    case SystemVariables::V12:               label = LabelType::XML_V12; break;
    case SystemVariables::V13:               label = LabelType::XML_V13; break;
    case SystemVariables::V14:               label = LabelType::XML_V14; break;
    case SystemVariables::V15:               label = LabelType::XML_V15; break;
    case SystemVariables::V16:               label = LabelType::XML_V16; break;
    case SystemVariables::V17:               label = LabelType::XML_V17; break;
    case SystemVariables::V18:               label = LabelType::XML_V18; break;
    case SystemVariables::V19:               label = LabelType::XML_V19; break;
    case SystemVariables::V20:               label = LabelType::XML_V20; break;
    case SystemVariables::V21:               label = LabelType::XML_V21; break;
    case SystemVariables::V22:               label = LabelType::XML_V22; break;
    case SystemVariables::V23:               label = LabelType::XML_V23; break;
    case SystemVariables::V24:               label = LabelType::XML_V24; break;
    case SystemVariables::V25:               label = LabelType::XML_V25; break;
    case SystemVariables::V26:               label = LabelType::XML_V26; break;
    case SystemVariables::V27:               label = LabelType::XML_V27; break;
    case SystemVariables::V28:               label = LabelType::XML_V28; break;
    case SystemVariables::V29:               label = LabelType::XML_V29; break;

    case SystemVariables::MESAJ1:            label = LabelType::XML_MESAJ1; break;
    case SystemVariables::MESAJ2:            label = LabelType::XML_MESAJ2; break;
    case SystemVariables::MESAJ3:            label = LabelType::XML_MESAJ3; break;
    case SystemVariables::MESAJ4:            label = LabelType::XML_MESAJ4; break;
    case SystemVariables::MESAJ5:            label = LabelType::XML_MESAJ5; break;
    case SystemVariables::MESAJ6:            label = LabelType::XML_MESAJ6; break;
    case SystemVariables::MESAJ7:            label = LabelType::XML_MESAJ7; break;
    case SystemVariables::MESAJ8:            label = LabelType::XML_MESAJ8; break;
    case SystemVariables::MESAJ9:            label = LabelType::XML_MESAJ9; break;
    
    case SystemVariables::LOGO:              label = LabelType::LOGO; break;
    case SystemVariables::SOL:               label = LabelType::SOL; break;
    case SystemVariables::ORTA:              label = LabelType::ORTA; break;
    case SystemVariables::SAG:               label = LabelType::SAG; break;
    case SystemVariables::BEYAZ:             label = LabelType::BEYAZ; break;
    case SystemVariables::SIYAH:             label = LabelType::SIYAH; break;
    case SystemVariables::KOYU:              label = LabelType::KOYU; break;
    case SystemVariables::ACIK:              label = LabelType::ACIK; break;
    case SystemVariables::KES:               label = LabelType::KES; break;
    case SystemVariables::CEKMECE:           label = LabelType::CEKMECE; break;
    case SystemVariables::ALTCIZGIPASIF:     label = LabelType::ALTCIZGIPASIF; break;
    //case SystemVariables::QRKODBAS:          label = LabelType::QRKODBAS; break;
    //case SystemVariables::QRKODSON:          label = LabelType::QRKODSON; break;

    case SystemVariables::IRDADATA:	         label = LabelType::IRDA_DATA; break;


    default: 
      // No matching LabelType yet.
      break;
  }
  return label;
}


String SystemVariables::getSystemVariable(SystemVariables::Enum enumval) {
  const LabelType::Enum label = SystemVariables2LabelType(enumval);

  if (LabelType::MAX_LABEL != label) {
    return getValue(label);
  }

  switch (enumval)
  {
    case BOOT_CAUSE:        return String(lastBootCause);                         // Integer value to be used in rules
    case BSSID:             return String((WiFiEventData.WiFiDisconnected()) ? MAC_address().toString() : WiFi.BSSIDstr());
    case CR:                return String('\r');
    case IP4:               return String(static_cast<int>(NetworkLocalIP()[3])); // 4th IP octet
    #if FEATURE_MQTT
    case ISMQTT:            return String(MQTTclient_connected ? 1 : 0);
    #else // if FEATURE_MQTT
    case ISMQTT:            return String('0');
    #endif // if FEATURE_MQTT

    #ifdef USES_P037
    case ISMQTTIMP:         return String(P037_MQTTImport_connected ? 1 : 0);
    #else // ifdef USES_P037
    case ISMQTTIMP:         return String('0');
    #endif // USES_P037


    case ISNTP:             return String(statusNTPInitialized ? 1 : 0);
    case ISWIFI:            return String(WiFiEventData.wifiStatus); // 0=disconnected, 1=connected, 2=got ip, 4=services
    // initialized
    case TARIH:	            return node_time.getDateString('-');
    case SAAT:	            return node_time.getTimeString(':');
    case LCLTIME_AM:        return node_time.getDateTimeString_ampm('-', ':', ' ');
    case LF:                return String('\n');
    case MAC_INT:           return String(getChipId()); // Last 24 bit of MAC address as integer, to be used in rules.
    case SPACE:             return String(' ');
    case SSID:              return (WiFiEventData.WiFiDisconnected()) ? F("--") : WiFi.SSID();
    case SYSBUILD_DATE:     return get_build_date();
    case SYSBUILD_TIME:     return get_build_time();
    case SYSDAY:            return String(node_time.day());
    case SYSDAY_0:          return timeReplacement_leadZero(node_time.day());
    case SYSHEAP:           return String(ESP.getFreeHeap());
    case SYSHOUR:           return String(node_time.hour());
    case SYSHOUR_0:         return timeReplacement_leadZero(node_time.hour());
    case SYSLOAD:           return String(getCPUload(), 2);
    case SYSMIN:            return String(node_time.minute());
    case SYSMIN_0:          return timeReplacement_leadZero(node_time.minute());
    case SYSMONTH:          return String(node_time.month());
    case SYSMONTH_S:        return String(node_time.month_str());
    case SYSNAME:           return Settings.getHostname();
    case SYSSEC:            return String(node_time.second());
    case SYSSEC_0:          return timeReplacement_leadZero(node_time.second());
    case SYSSEC_D:          return String(((node_time.hour() * 60) + node_time.minute()) * 60 + node_time.second());
    case SYSTIME:           return node_time.getTimeString(':');
    case SYSTIME_AM:        return node_time.getTimeString_ampm(':');
    case SYSTIME_AM_0:      return node_time.getTimeString_ampm(':', true, '0');
    case SYSTIME_AM_SP:     return node_time.getTimeString_ampm(':', true, ' ');
    case SYSTM_HM:          return node_time.getTimeString(':', false);
    case SYSTM_HM_0:        return node_time.getTimeString(':', false, '0');
    case SYSTM_HM_SP:       return node_time.getTimeString(':', false, ' ');
    case SYSTM_HM_AM:       return node_time.getTimeString_ampm(':', false);
    case SYSTM_HM_AM_0:     return node_time.getTimeString_ampm(':', false, '0');
    case SYSTM_HM_AM_SP:    return node_time.getTimeString_ampm(':', false, ' ');
    case SYSTZOFFSET:       return node_time.getTimeZoneOffsetString();
    case SYSWEEKDAY:        return String(node_time.weekday());
    case SYSWEEKDAY_S:      return node_time.weekday_str();
    case SYSYEAR_0:
    case SYSYEAR:           return String(node_time.year());
    case SYSYEARS:          return timeReplacement_leadZero(node_time.year() % 100);
    case SYS_MONTH_0:       return timeReplacement_leadZero(node_time.month());
    case S_CR:              return F("\\r");
    case S_LF:              return F("\\n");
    case UNIXDAY:           return String(node_time.getUnixTime() / 86400);
    case UNIXDAY_SEC:       return String(node_time.getUnixTime() % 86400);
    case UNIXTIME:          return String(node_time.getUnixTime());
    case UPTIME:            return String(getUptimeMinutes());
    case UPTIME_MS:         return ull2String(getMicros64() / 1000);
    #if FEATURE_ADC_VCC
    case VCC:               return String(vcc);
    #else // if FEATURE_ADC_VCC
    case VCC:               return F("-1");
    #endif // if FEATURE_ADC_VCC
    case WI_CH:             return String((WiFiEventData.WiFiDisconnected()) ? 0 : WiFi.channel());

    default:
      // Already handled above.
      return EMPTY_STRING;
  }
  return EMPTY_STRING;
}

#define SMART_REPL_T(T, S) \
  while (s.indexOf(T) != -1) { (S((T), s, useURLencode)); }

void SystemVariables::parseSystemVariables(String& s, boolean useURLencode)
{
  START_TIMER

  if (s.indexOf('%') == -1) {
    STOP_TIMER(PARSE_SYSVAR_NOCHANGE);
    return;
  }

  SystemVariables::Enum enumval = static_cast<SystemVariables::Enum>(0);

  do {
    enumval = SystemVariables::nextReplacementEnum(s, enumval);

    switch (enumval)
    {
      case SUNRISE:           
        SMART_REPL_T(SystemVariables::toString(enumval), replSunRiseTimeString); 
        break;
      case SUNSET:            
        SMART_REPL_T(SystemVariables::toString(enumval), replSunSetTimeString); 
        break;
      case UNKNOWN:

        // Do not replace
        break;
      default:
      {
        const String value = getSystemVariable(enumval);
        repl(SystemVariables::toString(enumval), value, s, useURLencode);
        break;
      }
    }
  }
  while (enumval != SystemVariables::Enum::UNKNOWN);

  int v_index = s.indexOf(F("%v"));

  while ((v_index != -1)) {
    unsigned int i;

    if (validUIntFromString(s.substring(v_index + 2), i)) {
      String key = F("%v");
      key += i;
      key += '%';

      if (s.indexOf(key) != -1) {
        const bool trimTrailingZeros = true;
        #if FEATURE_USE_DOUBLE_AS_ESPEASY_RULES_FLOAT_TYPE
        const String value = doubleToString(getCustomFloatVar(i), 6, trimTrailingZeros);
        #else
        const String value = floatToString(getCustomFloatVar(i), 6, trimTrailingZeros);
        #endif
        repl(key, value, s, useURLencode);
      }
    }
    v_index = s.indexOf(F("%v"), v_index + 1); // Find next occurance
  }

  STOP_TIMER(PARSE_SYSVAR);
}

#undef SMART_REPL_T


SystemVariables::Enum SystemVariables::nextReplacementEnum(const String& str, SystemVariables::Enum last_tested)
{
  if (str.indexOf('%') == -1) {
    return Enum::UNKNOWN;
  }

  SystemVariables::Enum nextTested = static_cast<SystemVariables::Enum>(0);

  if (last_tested > nextTested) {
    nextTested = static_cast<SystemVariables::Enum>(last_tested + 1);
  }

  if (nextTested >= Enum::UNKNOWN) {
    return Enum::UNKNOWN;
  }

  String str_prefix        = SystemVariables::toString(nextTested).substring(0, 2);
  bool   str_prefix_exists = str.indexOf(str_prefix) != -1;

  for (int i = nextTested; i < Enum::UNKNOWN; ++i) {
    SystemVariables::Enum enumval = static_cast<SystemVariables::Enum>(i);
    const String new_str_prefix   = SystemVariables::toString(enumval).substring(0, 2);

    if ((str_prefix == new_str_prefix) && !str_prefix_exists) {
      // Just continue
    } else {
      str_prefix        = new_str_prefix;
      str_prefix_exists = str.indexOf(str_prefix) != -1;

      if (str_prefix_exists) {
        if (str.indexOf(SystemVariables::toString(enumval)) != -1) {
          return enumval;
        }
      }
    }
  }

  return Enum::UNKNOWN;
}

String SystemVariables::toString(Enum enumval)
{
  if (enumval == Enum::SUNRISE || enumval == Enum::SUNSET) {
    // These need variables, so only prepend a %, not wrap.
    return String('%') + SystemVariables::toFlashString(enumval);
  }

  return wrap_String(SystemVariables::toFlashString(enumval), '%');
}

const __FlashStringHelper * SystemVariables::toFlashString(SystemVariables::Enum enumval)
{
  switch (enumval) {
    case Enum::BOOT_CAUSE:         return F("bootcause");
    case Enum::BSSID:              return F("bssid");
    case Enum::CR:                 return F("CR");
    case Enum::IP4:                return F("ip4");
    case Enum::IP:                 return F("ip");
    case Enum::SUBNET:             return F("subnet");
    case Enum::DNS:                return F("dns");
    case Enum::DNS_1:              return F("dns1");
    case Enum::DNS_2:              return F("dns2");
    case Enum::GATEWAY:            return F("gateway");
    case Enum::CLIENTIP:           return F("clientip");
    case Enum::ISMQTT:             return F("ismqtt");
    case Enum::ISMQTTIMP:          return F("ismqttimp");
    case Enum::ISNTP:              return F("isntp");
    case Enum::ISWIFI:             return F("iswifi");
    #if FEATURE_ETHERNET
    case Enum::ETHWIFIMODE:        return F("ethwifimode");
    case Enum::ETHCONNECTED:       return F("ethconnected");
    case Enum::ETHDUPLEX:          return F("ethduplex");
    case Enum::ETHSPEED:           return F("ethspeed");
    case Enum::ETHSTATE:           return F("ethstate");
    case Enum::ETHSPEEDSTATE:      return F("ethspeedstate");
    #endif // if FEATURE_ETHERNET
    case Enum::LCLTIME:            return F("lcltime");
    case Enum::LCLTIME_AM:         return F("lcltime_am");
    case Enum::LF:                 return F("LF");
    case Enum::MAC:                return F("mac");
    case Enum::MAC_INT:            return F("mac_int");
    case Enum::RSSI:               return F("rssi");
    case Enum::SPACE:              return F("SP");
    case Enum::SSID:               return F("ssid");
    case Enum::SUNRISE:            return F("sunrise");
    case Enum::SUNSET:             return F("sunset");
    case Enum::SUNRISE_S:          return F("s_sunrise");
    case Enum::SUNSET_S:           return F("s_sunset");
    case Enum::SUNRISE_M:          return F("m_sunrise");
    case Enum::SUNSET_M:           return F("m_sunset");
    case Enum::SYSBUILD_DATE:      return F("sysbuild_date");
    case Enum::SYSBUILD_DESCR:     return F("sysbuild_desc");
    case Enum::SYSBUILD_FILENAME:  return F("sysbuild_filename");
    case Enum::SYSBUILD_GIT:       return F("sysbuild_git");
    case Enum::SYSBUILD_TIME:      return F("sysbuild_time");
    case Enum::SYSDAY:             return F("sysday");
    case Enum::SYSDAY_0:           return F("sysday_0");
    case Enum::SYSHEAP:            return F("sysheap");
    case Enum::SYSHOUR:            return F("syshour");
    case Enum::SYSHOUR_0:          return F("syshour_0");
    case Enum::SYSLOAD:            return F("sysload");
    case Enum::SYSMIN:             return F("sysmin");
    case Enum::SYSMIN_0:           return F("sysmin_0");
    case Enum::SYSMONTH:           return F("sysmonth");
    case Enum::SYSMONTH_S:         return F("sysmonth_s");
    case Enum::SYSNAME:            return F("sysname");
    case Enum::SYSSEC:             return F("syssec");
    case Enum::SYSSEC_0:           return F("syssec_0");
    case Enum::SYSSEC_D:           return F("syssec_d");
    case Enum::SYSSTACK:           return F("sysstack");
    case Enum::SYSTIME:            return F("systime");
    case Enum::SYSTIME_AM:         return F("systime_am");
    case Enum::SYSTIME_AM_0:       return F("systime_am_0");
    case Enum::SYSTIME_AM_SP:      return F("systime_am_sp");
    case Enum::SYSTM_HM:           return F("systm_hm");
    case Enum::SYSTM_HM_0:         return F("systm_hm_0");
    case Enum::SYSTM_HM_SP:        return F("systm_hm_sp");
    case Enum::SYSTM_HM_AM:        return F("systm_hm_am");
    case Enum::SYSTM_HM_AM_0:      return F("systm_hm_am_0");
    case Enum::SYSTM_HM_AM_SP:     return F("systm_hm_am_sp");
    case Enum::SYSTZOFFSET:        return F("systzoffset");
    case Enum::SYSWEEKDAY:         return F("sysweekday");
    case Enum::SYSWEEKDAY_S:       return F("sysweekday_s");
    case Enum::SYSYEAR:            return F("sysyear");
    case Enum::SYSYEARS:           return F("sysyears");
    case Enum::SYSYEAR_0:          return F("sysyear_0");
    case Enum::SYS_MONTH_0:        return F("sysmonth_0");
    case Enum::S_CR:               return F("R");
    case Enum::S_LF:               return F("N");
    case Enum::UNIT_sysvar:        return F("unit");
    #if FEATURE_ZEROFILLED_UNITNUMBER
    case Enum::UNIT_0_sysvar:      return F("unit_0");
    #endif // FEATURE_ZEROFILLED_UNITNUMBER
    case Enum::UNIXDAY:            return F("unixday");
    case Enum::UNIXDAY_SEC:        return F("unixday_sec");
    case Enum::UNIXTIME:           return F("unixtime");
    case Enum::UPTIME:             return F("uptime");
    case Enum::UPTIME_MS:          return F("uptime_ms");
    case Enum::VCC:                return F("vcc");
    case Enum::WI_CH:              return F("wi_ch");
    case Enum::FLASH_FREQ:         return F("flash_freq");
    case Enum::FLASH_SIZE:         return F("flash_size");
    case Enum::FLASH_CHIP_VENDOR:  return F("flash_chip_vendor");
    case Enum::FLASH_CHIP_MODEL:   return F("flash_chip_model");
    case Enum::FS_FREE:            return F("fs_free");
    case Enum::FS_SIZE:            return F("fs_size");
    case Enum::ESP_CHIP_ID:        return F("cpu_id");
    case Enum::ESP_CHIP_FREQ:      return F("cpu_freq");
    case Enum::ESP_CHIP_MODEL:     return F("cpu_model");
    case Enum::ESP_CHIP_REVISION:  return F("cpu_rev");
    case Enum::ESP_CHIP_CORES:     return F("cpu_cores");
    case Enum::ESP_BOARD_NAME:     return F("board_name");

    case Enum::TARIH:              return F("tarih"); 
    case Enum::SAAT:               return F("saat");
    case Enum::TARIHV:             return F("tarih_v"); 
    case Enum::SAATV:              return F("saat_v");
    case Enum::NET:                return F("net");
    case Enum::NET_S:              return F("net_s");
    case Enum::NET_V:              return F("net_v");
    case Enum::DARA:               return F("dara");
    case Enum::DARA_S:             return F("dara_s");
    case Enum::DARA_V:             return F("dara_v");
    case Enum::BRUT:               return F("brut");
    case Enum::BRUT_S:             return F("brut_s");
    case Enum::BRUT_V:             return F("brut_v");    
    case Enum::ADET:               return F("adet");
    case Enum::ADETGRAMAJ:         return F("adetgramaj");
    case Enum::PLUNO:              return F("pluno");
    case Enum::PLUADI:             return F("pluadi");
    case Enum::PLUKOD:             return F("plukod");
    case Enum::BARKOD:             return F("barkod");
    case Enum::BIRIMFIYAT:         return F("birimfiyat");
    case Enum::TUTAR:              return F("tutar");
    case Enum::SNO: 		           return F("sno");
    case Enum::SAYAC_1:            return F("sayac1");
    case Enum::SAYAC_2:            return F("sayac2");
    case Enum::SAYAC_3:            return F("sayac3");
    case Enum::SAYAC_4:            return F("sayac4");
    case Enum::SAYAC_1_C:          return F("sayac1_c");
    case Enum::SAYAC_2_C:          return F("sayac2_c");
    case Enum::SAYAC_3_C:          return F("sayac3_c");
    case Enum::SAYAC_4_C:          return F("sayac4_c");
    case Enum::SERINO:	           return F("serino");
    case Enum::FISNO:              return F("fisno");
    case Enum::TOPNET:             return F("topnet");
    case Enum::TOPDARA:            return F("topdara");
    case Enum::TOPBRUT:            return F("topbrut");
    case Enum::EAN8:               return F("EAN8barkod");
    case Enum::EAN13:              return F("EAN13barkod");
    case Enum::QRKOD:              return F("QRkod");
    case Enum::STABIL:             return F("stabil");
    case Enum::MESAJ1:             return F("msg1");
    case Enum::MESAJ2:             return F("msg2");
    case Enum::MESAJ3:             return F("msg3");
    case Enum::MESAJ4:             return F("msg4");
    case Enum::MESAJ5:             return F("msg5");
    case Enum::MESAJ6:             return F("msg6");
    case Enum::MESAJ7:             return F("msg7");
    case Enum::MESAJ8:             return F("msg8");
    case Enum::MESAJ9:             return F("msg9");

    case Enum::FISBAS1:            return F("fisbas1");
    case Enum::FISBAS2:            return F("fisbas2");
    case Enum::FISBAS3:            return F("fisbas3");
    case Enum::FISBAS4:            return F("fisbas4");

    case Enum::KFONT1:             return F("kB1");
    case Enum::KFONT2:             return F("kB2");
    case Enum::KFONT3:             return F("kB3");
    case Enum::KFONT4:             return F("kB4");
    case Enum::KFONT5:             return F("kB5");
    case Enum::KFONT6:             return F("kB6");

    case Enum::LOGO:               return F("LOGO");
    case Enum::FONT1:              return F("B1");
    case Enum::FONT2:              return F("B2");
    case Enum::FONT3:              return F("B3");
    case Enum::FONT4:              return F("B4");
    case Enum::FONT5:              return F("B5");
    case Enum::FONT6:              return F("B6");

    case Enum::SOL:                return F("SOL");
    case Enum::ORTA:               return F("ORTA");
    case Enum::SAG:                return F("SAG");

    case Enum::BEYAZ:              return F("BEYAZ");
    case Enum::SIYAH:              return F("SIYAH");    
    case Enum::KOYU:               return F("KOYU");
    case Enum::ACIK:               return F("ACIK");
    case Enum::KES:                return F("KES");    
    case Enum::CEKMECE:            return F("CEKMECE");
    case Enum::ALTCIZGIPASIF:      return F("ALTCIZGIPASIF");
    case Enum::ALTCIZGIAKTIF:      return F("ALTCIZGIAKTIF");   
    //case Enum::QRKODBAS:           return F("QRKODBAS");
    //case Enum::QRKODSON:           return F("QRKODSON");

    case Enum::FIRMAADI:           return F("firmaadi");
    case Enum::PLAKANO:            return F("plakano");
    case Enum::MUSTERIADI:         return F("musteriadi");
    case Enum::OPERATORADI:        return F("operatoradi");
    case Enum::AGIRLIK:            return F("agirlik");
   
    case Enum::V0:                 return F("V0");
    case Enum::V1:                 return F("V1");
    case Enum::V2:                 return F("V2");
    case Enum::V3:                 return F("V3");
    case Enum::V4:                 return F("V4");
    case Enum::V5:                 return F("V5");
    case Enum::V6:                 return F("V6");
    case Enum::V7:                 return F("V7");
    case Enum::V8:                 return F("V8");
    case Enum::V9:                 return F("V9");
    case Enum::V10:                return F("V10");
    case Enum::V11:                return F("V11");
    case Enum::V12:                return F("V12");
    case Enum::V13:                return F("V13");
    case Enum::V14:                return F("V14");
    case Enum::V15:                return F("V15");
    case Enum::V16:                return F("V16");
    case Enum::V17:                return F("V17");
    case Enum::V18:                return F("V18");
    case Enum::V19:                return F("V19");
    case Enum::V20:                return F("V20");
    case Enum::V21:                return F("V21");
    case Enum::V22:                return F("V22");
    case Enum::V23:                return F("V23");
    case Enum::V24:                return F("V24");
    case Enum::V25:                return F("V25");
    case Enum::V26:                return F("V26");
    case Enum::V27:                return F("V27");
    case Enum::V28:                return F("V28");
    case Enum::V29:                return F("V29");
    
    case Enum::FNET:               return F("Fnet");
    case Enum::FDARA:              return F("Fdara");
    case Enum::FBRUT:              return F("Fbrut");

    case Enum::IRDADATA:           return F("irdadata");

    case Enum::UNKNOWN: break;
  }
  return F("Unknown");
}
