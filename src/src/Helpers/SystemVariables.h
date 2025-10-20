#ifndef HELPERS_SYSTEMVARIABLES_H
#define HELPERS_SYSTEMVARIABLES_H

#include "../../ESPEasy_common.h"

class SystemVariables {

public:

  enum Enum : uint8_t {
    // For optmization, keep enums sorted alfabetically
    BOOT_CAUSE,
    BSSID,
    CR,
    IP,
    IP4,  // 4th IP octet
    SUBNET,
    GATEWAY,
    DNS,
    DNS_1,
    DNS_2,
    CLIENTIP,
    ISMQTT,
    ISMQTTIMP,
    ISNTP,
    ISWIFI,
    #if FEATURE_ETHERNET
    ETHWIFIMODE,
    ETHCONNECTED,
    ETHDUPLEX,
    ETHSPEED,
    ETHSTATE,
    ETHSPEEDSTATE,
    #endif // if FEATURE_ETHERNET
    LCLTIME,
    LCLTIME_AM,
    LF,
    MAC,
    MAC_INT,
    RSSI,
    SPACE,
    SSID,
    SUNRISE,
    SUNSET,
    SUNRISE_S,
    SUNSET_S,
    SUNRISE_M,
    SUNSET_M,
    SYSBUILD_DATE,
    SYSBUILD_DESCR,
    SYSBUILD_FILENAME,
    SYSBUILD_GIT,
    SYSBUILD_TIME,
    SYSDAY,
    SYSDAY_0,
    SYSHEAP,
    SYSHOUR,
    SYSHOUR_0,
    SYSLOAD,
    SYSMIN,
    SYSMIN_0,
    SYSMONTH,
    SYSMONTH_S,
    SYSNAME,
    SYSSEC,
    SYSSEC_0,
    SYSSEC_D,
    SYSSTACK,
    SYSTIME,
    SYSTIME_AM,
    SYSTIME_AM_0,
    SYSTIME_AM_SP,
    SYSTM_HM,
    SYSTM_HM_0,
    SYSTM_HM_SP,
    SYSTM_HM_AM,
    SYSTM_HM_AM_0,
    SYSTM_HM_AM_SP,
    SYSTZOFFSET,
    SYSWEEKDAY,
    SYSWEEKDAY_S,
    SYSYEAR,
    SYSYEARS,
    SYSYEAR_0,
    SYS_MONTH_0,
    S_CR,
    S_LF,
    UNIT_sysvar,   // We already use UNIT as define.
    #if FEATURE_ZEROFILLED_UNITNUMBER
    UNIT_0_sysvar,
    #endif // FEATURE_ZEROFILLED_UNITNUMBER
    UNIXDAY,
    UNIXDAY_SEC,
    UNIXTIME,
    UPTIME,
    UPTIME_MS,
    VCC,
    WI_CH,
    FLASH_FREQ,    // Frequency of the flash chip
    FLASH_SIZE,    // Real size of the flash chip
    FLASH_CHIP_VENDOR,
    FLASH_CHIP_MODEL,
    FS_SIZE,       // Size of the file system
    FS_FREE,       // Free space (in bytes) on the file system

    ESP_CHIP_ID,
    ESP_CHIP_FREQ,
    ESP_CHIP_MODEL,
    ESP_CHIP_REVISION,
    ESP_CHIP_CORES,
    ESP_BOARD_NAME,

    TARIH,
    SAAT,
    TARIHV,
    SAATV,
    STABIL,
    NET,
    NET_S,
    NET_V,
    DARA,
    DARA_S,
    DARA_V,
    BRUT,
    BRUT_S,
    BRUT_V,
    ADET,
    ADETGRAMAJ,
    PLUNO,
    PLUADI,
    PLUKOD,
    BARKOD,
    BIRIMFIYAT,
    TUTAR,
    SNO,
    SAYAC_1,
    SAYAC_2,
    SAYAC_3,
    SAYAC_4,
    SAYAC_1_C,
    SAYAC_2_C,
    SAYAC_3_C,
    SAYAC_4_C,
    SERINO,
    FISNO,
    TOPNET,
    TOPDARA,
    TOPBRUT,
    EAN8,
    EAN13,
    QRKOD,
    MESAJ1,MESAJ2,MESAJ3,MESAJ4,MESAJ5,MESAJ6,MESAJ7,MESAJ8,MESAJ9,
    FISBAS1,FISBAS2,FISBAS3,FISBAS4,
    KFONT1,KFONT2,KFONT3,KFONT4,KFONT5,KFONT6,
    LOGO,
    FONT1,FONT2,FONT3,FONT4,FONT5,FONT6,
    SOL,ORTA,SAG,
    BEYAZ,SIYAH,KOYU,ACIK,
    KES,CEKMECE,ALTCIZGIPASIF,ALTCIZGIAKTIF,
    //QRKODBAS,QRKODSON,
    FIRMAADI,PLAKANO,MUSTERIADI,OPERATORADI,AGIRLIK,
    V0,V1,V2,V3,V4,V5,V6,V7,V8,V9,V10,V11,V12,V13,V14,V15,V16,V17,V18,V19,V20,V21,V22,V23,V24,V25,V26,V27,V28,V29,
    FNET,FDARA,FBRUT,
    IRDADATA,

    // Keep UNKNOWN as last
    UNKNOWN
  };

  // Find the next thing to replace.
  // Return UNKNOWN when nothing needs to be replaced.
  static SystemVariables::Enum nextReplacementEnum(const String& str, SystemVariables::Enum last_tested);

  static String toString(SystemVariables::Enum enumval);
  static const __FlashStringHelper * toFlashString(SystemVariables::Enum enumval);

  static String getSystemVariable(SystemVariables::Enum enumval);

  static void parseSystemVariables(String& s, boolean useURLencode);

};




#endif // HELPERS_SYSTEMVARIABLES_H
