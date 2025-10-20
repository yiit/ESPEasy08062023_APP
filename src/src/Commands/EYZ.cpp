#include "HardwareSerial.h"
#include "../Commands/EYZ.h"

#include "../../_Plugin_Helper.h"
#include "../Helpers/Misc.h"
#include "../DataTypes/TaskIndex.h"

#include "../../ESPEasy-Globals.h"
#include "../../ESPEasy_common.h"

#include "../Commands/Common.h"

#include "../ESPEasyCore/ESPEasyNetwork.h"
#include "../ESPEasyCore/Serial.h"

#include "../Globals/SecuritySettings.h"
#include "../Globals/ESPEasy_time.h"
#include "../Globals/Settings.h"

#include "../Helpers/Misc.h"
#include "../Helpers/Memory.h"
#include "../Helpers/ESPEasy_Storage.h"
#include "../Helpers/ESPEasy_time_calc.h"
#include "../Helpers/StringConverter.h"
#include "../Helpers/SystemVariables.h"

#include "../DataStructs/TimingStats.h"

#include "../Helpers/Hardware.h"

#include "../ESPEasyCore/Controller.h"

#include "../DataStructs/ExtraTaskSettingsStruct.h"

#include "../Helpers/RulesMatcher.h"
#include "../ESPEasyCore/ESPEasyRules.h"

#if defined(ESP32)
#include <HTTPClient.h>
#endif
#if defined(ESP8266)
#include <ESP8266HTTPClient.h>
#endif

#include <FS.h>

#if FEATURE_SD
#include <SD.h>
#include "../Helpers/ESPEasy_Storage.h"
#endif

#if defined(USES_P130) || defined(USES_P131) || defined(USES_P132) || defined(USES_P133) || defined(USES_140)

long control_data = 0;

void etiket_yazdir_hata(boolean aktif, int mod) {
  if (aktif) {
    Serial1.println("SOUND 100,2");
    Serial1.println("BEEP");
  }
#ifdef ESP32
#ifdef HAS_BLUETOOTH
  if (Settings.bluetooth_mod == 1)
    //SerialBT.write((const uint8_t*)hata_beep, sizeof(hata_beep));
    Serial2.write((const uint8_t*)hata_beep, sizeof(hata_beep));
#endif
#endif
#ifdef ESP32
#ifdef HAS_WIFI
  ledcWriteTone(2, 500);
  ledcWrite(2, 255);
  delay(1000);
  ledcWrite(2, 0);
#endif
#endif
}

void etiket_yaz(String ETIKET, int BTmod, int HTTPmod, int SERIALmod, bool CLImod) {
#ifdef ESP32
#ifdef HAS_WIFI
  ledcSetup(2, 1000, 8);
  ledcAttachPin(5, 2);
#endif
#endif
#ifdef ESP32
  int address = 0;
  EEPROM.writeLong(address, seri_no);
  address += sizeof(uint32_t);
  EEPROM.writeLong(address, sno);
  address += sizeof(uint32_t);
  EEPROM.writeFloat(address, top_net);
  address += sizeof(float);
  EEPROM.commit();
#else
  int address = 0;
  EEPROM.put(address, seri_no);
  address += sizeof(seri_no);
  EEPROM.put(address, top_net);
  address += sizeof(top_net);
  EEPROM.put(address, sno);
  address += sizeof(sno);
  EEPROM.commit();
#endif
  /*for (int addr = 0; addr < 9; addr++) {
    EEPROM.write(addr, (byte)(XML_SERI_NO_C[addr]));
  }
  EEPROM.commit();*/
  if (fileExists(ETIKET)) {
    fs::File form = tryOpenFile(ETIKET, "r");
    if (!ExtraTaskSettings.TaskPrintBartender) {
      String file_data = "";
      String http_data = "";
      kopya_etiket = "";
      WiFiClient client;
      if (CLImod) {
        client.setTimeout(1);
        client.connect(Settings.Cli_PrinterIP, Settings.Cli_PrinterPort, 100);
      }
      if ((Settings.Web_ServisAktif) && (HTTPmod == 1))
        client.connect(Settings.Web_ServisAdres, Settings.Web_ServisPort, 100);
      Serial1.println("SOUND 1,100");
      //Serial1.println("BEEP");
#ifdef HAS_WIFI
    ledcWriteTone(2, 2000);
    ledcWrite(2, 255);
#endif
      while (form.position() < form.size()) {
        file_data = form.readStringUntil('\n');
        file_data.replace("ENDUTEK.TTF","0");
        parseTemplate(file_data);
        parse_string_commands(file_data);
        //parseSystemVariables(file_data, false);
        file_data += '\n';
        if (HTTPmod)
          http_data += file_data;
        if ((CLImod) && (Settings.Cli_PrinterAktif)) {
          client.print(file_data);
          //String line = client.readStringUntil('\n');
          //client.println(line);
        }
        if ((CLImod) && (Settings.Web_ServisAktif)) {
          if (fileExists(Settings.sd_prn)) {
            fs::File form = tryOpenFile(Settings.sd_prn, "r");
            String file_data = "";
            while (form.position() < form.size()) {
              file_data = form.readStringUntil('\n');
              file_data.replace("ENDUTEK.TTF","0");
              parseTemplate(file_data);
              parse_string_commands(file_data);
              //file_data += '\n';
              client.print(file_data);
            }
          }
        }
        if ((Settings.Web_ServisAktif) && (HTTPmod == 1))
          client.print(file_data);
        kopya_etiket += file_data;
        //kopya_etiket += "\r\n";
        yazdir_c[0] = 0;
        strncpy(yazdir_c, file_data.c_str(), file_data.length());
#ifdef ESP32
#ifdef HAS_BLUETOOTH
        if (Settings.bluetooth_mod == BTmod)
          //SerialBT.print(file_data);
          Serial2.print(file_data);
#endif
#endif
        if (SERIALmod == 1) {
          for (int i = 0; i < file_data.length(); i++) {
            /*if (yazdir_c[i] == 94) {Serial1.write(0);}
            else if ((karakter_195) && (yazdir_c[i] == 135)) {Serial1.write(199); karakter_195 = false;} //Serial1.write(128); karakter_195 = false;} //Ç
            else if ((karakter_195) && (yazdir_c[i] == 167)) {Serial1.write(231); karakter_195 = false;} //Serial1.write(135); karakter_195 = false;} //ç
            else if ((karakter_195) && (yazdir_c[i] == 150)) {Serial1.write(214); karakter_195 = false;} //Serial1.write(153); karakter_195 = false;} //Ö
            else if ((karakter_195) && (yazdir_c[i] == 182)) {Serial1.write(246); karakter_195 = false;}//Serial1.write(148); karakter_195 = false;} //ö
            else if ((karakter_195) && (yazdir_c[i] == 156)) {Serial1.write(220); karakter_195 = false;}//Serial1.write(154); karakter_195 = false;} //Ü
            else if ((karakter_195) && (yazdir_c[i] == 188)) {Serial1.write(252); karakter_195 = false;}//Serial1.write(129); karakter_195 = false;} //ü
            else if ((karakter_196) && (yazdir_c[i] == 158)) {Serial1.write(208); karakter_196 = false;}//Serial1.write(166); karakter_196 = false;} //Ğ
            else if ((karakter_196) && (yazdir_c[i] == 159)) {Serial1.write(240); karakter_196 = false;}//Serial1.write(167); karakter_196 = false;} //ğ
            else if ((karakter_196) && (yazdir_c[i] == 176)) {Serial1.write(221); karakter_196 = false;}//Serial1.write(152); karakter_196 = false;} //İ
            else if ((karakter_196) && (yazdir_c[i] == 177)) {Serial1.write(253); karakter_196 = false;}//Serial1.write(141); karakter_196 = false;} //ı
            else if ((karakter_197) && (yazdir_c[i] == 158)) {Serial1.write(222); karakter_197 = false;}//Serial1.write(158); karakter_197 = false;} //Ş
            else if ((karakter_197) && (yazdir_c[i] == 159)) {Serial1.write(254); karakter_197 = false;}//Serial1.write(159); karakter_197 = false;} //ş
            else if (yazdir_c[i] == 195) {karakter_195 = true;}
            else if (yazdir_c[i] == 196) {karakter_196 = true;}
            else if (yazdir_c[i] == 197) {karakter_197 = true;}*/
            //else {Serial1.write(yazdir_c[i]);}
            Serial1.write(yazdir_c[i]);
#ifdef DIKOMSAN_VERSION
            Serial.write(yazdir_c[i]);
#endif
          }
        }
        //file_data.trim();
      }
      form.close();
      if ((Settings.Web_ServisAktif) && (HTTPmod == 0)) {
        HTTPClient http;
        //String ServerName = "http://";
        //ServerName = Settings.Web_ServisIP;
        //ServerName += ":";
        //ServerName += (String)Settings.Web_ServisPort;
        //ServerName += "/";
        http_data.replace("\n", "");
        //http.begin(client, http_data);
        http.begin(client, http_data);
        http.addHeader("Content-Type", "text/plain");
        //int httpCode = http.POST(http_data);   //Send the request
        http.POST(http_data);
        String payload = http.getString();  //Get the response payload
        //addLog(LOG_LEVEL_INFO, httpCode);   //Print HTTP return code
        //addLog(LOG_LEVEL_INFO, payload);    //Print request response payload
        http.end();  //Close connection
      }
    } else {
      String file_data = "";
      yazdir_c[0] = 0;
      int file_data_i = 0;
      if (SERIALmod == 1)
        Serial1.println("SOUND 1,100");
      WiFiClient client;
      if (CLImod) {
        client.setTimeout(1);
        client.connect(Settings.Cli_PrinterIP, Settings.Cli_PrinterPort, 100);
      }
      if ((Settings.Web_ServisAktif) && (HTTPmod == 1))
        client.connect(Settings.Web_ServisAdres, Settings.Web_ServisPort, 100);
      while (form.position() < form.size()) {
        char file_data_s = form.read();
        yazdir_c[file_data_i] = file_data_s;
        file_data_i++;
        if ((file_data_i > 512) || (file_data_s == '\n')) {
          if (file_data_i < 510) {
            String yazdir_s = "";
            for (int i = 0; i < file_data_i; i++)
              yazdir_s += yazdir_c[i];
            yazdir_s.replace("ENDUTEK.TTF","0");
            parseTemplate(yazdir_s);
            parse_string_commands(yazdir_s);
            //parseSystemVariables(yazdir_s, false);
            if ((CLImod) && (Settings.Cli_PrinterAktif))
              client.print(yazdir_s);
            Serial1.print(yazdir_s);
          } else {
            for (int i = 0; i < file_data_i; i++) {
            Serial1.write(yazdir_c[i]);
            if ((CLImod) && (Settings.Cli_PrinterAktif))
              client.write(yazdir_c[i]);
            control_data = control_data + int(yazdir_c[i]);
            }
            file_data = "";
          }
          yazdir_c[0] = 0;
          Serial1.flush();
          file_data_i = 0;
        }
      }
      form.close();
    }
    //Serial.print(control_data);
  }
#if FEATURE_SD
  String dosya = "/";
  dosya += node_time.getDateString('_');
  dosya += ".dat";
  //XML_NET_S.replace(".", ",");
  if (fileExists(Settings.sd_prn)) {
    fs::File form = tryOpenFile(Settings.sd_prn, "r");
    String file_data = "";
    while (form.position() < form.size()) {
      file_data = form.readStringUntil('\n');
      //file_data += '\n';
      parseSystemVariables(file_data, false);
      delay(10);
      String data = "SDdata";
      data += dosya;
      data += file_data;
      Serial1.print(data);
      delay(10);
      //Serial.print(data);
      fs::File logFile = SD.open(dosya, "a+");
      if (logFile)
        logFile.print(file_data);
      logFile.close();
    }
  }
#endif
  XML_BARKOD_S = "";
  delay(1000);
#ifdef ESP32
#ifdef HAS_WIFI
  ledcWrite(2, 0);
#endif
#endif
}

String Command_Eyz_Sensor(struct EventStruct* event, const char* Line) {
  //Serial.println("& V1 do \"calibrate_sensor\"");
  for (int i = 0; i < sizeof(etiketcal); i++) {
    if (etiketcal[i] == 94) {Serial1.write(0);}
    else {Serial1.write(etiketcal[i]);}
  }
  //return return_see_serial(event);
  return return_command_success();
}

String Command_Eyz_Test(struct EventStruct* event, const char* Line) {
  String TmpStr1 = "0";
  String TmpStr2 = "0";
  String eyz_test = "";
  dtostrf(123456.7, 7, 1, XML_NET_C);
  dtostrf(123456.7, 7, 1, XML_DARA_C);
  dtostrf(123456.7, 7, 1, XML_BRUT_C);
  dtostrf(1234567, 7, 0, XML_ADET_C);
  dtostrf(123456.7, 7, 1, XML_TOP_NET_C);
  dtostrf(123456.7, 7, 1, XML_TOP_DARA_C);
  dtostrf(123456.7, 7, 1, XML_TOP_BRUT_C);
  dtostrf(123, 3, 0, XML_SNO_C);
  dtostrf(123.4567, 7, 4, XML_ADET_GRAMAJ_C);
  XML_TARIH_S = node_time.getDateString('-');
  XML_SAAT_S = node_time.getTimeString(':');
  if (GetArgv(Line, TmpStr1, 2))
    eyz_test = TmpStr1;
  if (GetArgv(Line, TmpStr1, 3))
    eyz_test = TmpStr1;
  etiket_yaz(eyz_test, 1, Settings.Web_ServisMod, 1, Settings.Cli_PrinterAktif);
  return return_see_serial(event);
}

String Command_Eyz_Art(struct EventStruct* event, const char* Line) {
  String TmpStr1 = "0";
  String TmpStr2 = "0";
  int test_eyz_i = 0;
  if (GetArgv(Line, TmpStr1, 2))
    test_eyz_i = TmpStr1.toInt();
  if (XML_NET_S.toFloat() > 0.0001) {
    sno = sno + 1;
    XML_SNO_S = String(sno);
    String(sno).toCharArray(XML_SNO_C, 3);
    top_net = XML_NET_S.toFloat() + top_net;
    XML_TOP_NET_S = top_net;
    top_dara = XML_DARA_S.toFloat() + top_dara;
    XML_TOP_DARA_S = top_dara;
    top_brut = XML_BRUT_S.toFloat() + top_brut;
    XML_TOP_BRUT_S = top_brut;
  }
  dtostrf(sno, 3, 0, XML_SNO_C);
  if ((XML_NET_S.toFloat() > 0.0001) || (Settings.UseNegatifYaz)) {
    if (Settings.UseNegatifYaz) {
      XML_TARIH_S = node_time.getDateString('-');
      XML_SAAT_S = node_time.getTimeString(':');
    }
    etiket_yaz(Settings.art_prn, 1, Settings.Web_ServisMod, 1, Settings.Cli_PrinterAktif);
  } else etiket_yazdir_hata(true, 1);
  //return return_see_serial(event);
  return return_command_success();
}

String Command_Eyz_Tek(struct EventStruct* event, const char* Line) {
  String TmpStr1 = "0";
  String TmpStr2 = "0";
  int test_eyz_i = 0;
  if (GetArgv(Line, TmpStr1, 2))
    test_eyz_i = TmpStr1.toInt();
  dtostrf(top_net, 10, ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_TOP_NET_C);
  dtostrf(top_dara, 10, ExtraTaskSettings.TaskDeviceValueDecimals[1], XML_TOP_DARA_C);
  dtostrf(top_brut, 10, ExtraTaskSettings.TaskDeviceValueDecimals[2], XML_TOP_BRUT_C);
  //String(sno).toCharArray(XML_SNO_C, 3);
  if (sno == 0 && ((XML_NET_S.toFloat() > 0.00001) || Settings.UseNegatifYaz)) {
    if (Settings.UseNegatifYaz) {
      XML_TARIH_S = node_time.getDateString('-');
      XML_SAAT_S = node_time.getTimeString(':');
    }
    fis_no = fis_no + 1;
    XML_FIS_NO_S = String(fis_no);
    dtostrf(fis_no, 8, 0, XML_FIS_NO_C);
    seri_no = seri_no + 1;
    XML_SERI_NO_S = String(seri_no);
    dtostrf(seri_no, 8, 0, XML_SERI_NO_C);
    //etiket_yaz(ETIKET_TEK,1,Settings.Web_ServisMod,0,Settings.Cli_PrinterAktif);   //SERİAL PASİF
    etiket_yaz(Settings.tek_prn, 1, Settings.Web_ServisMod, 1, Settings.Cli_PrinterAktif);  //SERİAL AKTİF
    sendData(event);
  } else if ((top_net > 0.0001) || (Settings.UseNegatifYaz)) {
    if (Settings.UseNegatifYaz) {
      XML_TARIH_S = node_time.getDateString('-');
      XML_SAAT_S = node_time.getTimeString(':');
    }
    XML_SNO_S = String(sno);
    dtostrf(sno, 3, 0, XML_SNO_C);
    dtostrf(top_net, 8, ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_TOP_NET_C);
    dtostrf(top_dara, 8, ExtraTaskSettings.TaskDeviceValueDecimals[1], XML_TOP_DARA_C);
    dtostrf(top_brut, 8, ExtraTaskSettings.TaskDeviceValueDecimals[2], XML_TOP_BRUT_C);
    etiket_yaz(Settings.top_prn, 1, Settings.Web_ServisMod, 1, Settings.Cli_PrinterAktif);

  } else
    etiket_yazdir_hata(true, 1);
  sno = 0;
  XML_SNO_S = sno;
  top_net = 0;
  XML_TOP_NET_S = top_net;
  top_dara = 0;
  XML_TOP_DARA_S = top_net;
  top_brut = 0;
  XML_TOP_BRUT_S = top_net;
#ifdef ESP32
  int address = 0;
  EEPROM.writeLong(address, seri_no);
  address += sizeof(uint32_t);
  EEPROM.writeLong(address, sno);
  address += sizeof(uint32_t);
  EEPROM.writeFloat(address, top_net);
  address += sizeof(float);
  EEPROM.commit();
#else
  int address = 0;
  EEPROM.put(address, seri_no);
  address += sizeof(seri_no);
  EEPROM.put(address, top_net);
  address += sizeof(top_net);
  EEPROM.put(address, sno);
  address += sizeof(sno);
  EEPROM.commit();
#endif
  //return return_see_serial(event);
  return return_command_success();
}

String Command_Eyz_Plu_Art(struct EventStruct* event, const char* Line) {
  String TmpStr1 = "0";
  String TmpStr2 = "0";
  String TmpStr3 = "0";
  String TmpStr4 = "0";
  int pluno_click = 0;
  int etiket_sayisi = 0;
  int test_eyz_i = 0;
  if (GetArgv(Line, TmpStr1, 2))
    pluno_click = TmpStr1.toInt();
  if (GetArgv(Line, TmpStr2, 3))
    etiket_sayisi = TmpStr2.toInt();
  if (GetArgv(Line, TmpStr3, 4))
    test_eyz_i = TmpStr3.toInt();
  if (pluno_click == 0) {
#ifdef ESP8266
    Serial1.println(kopya_etiket);
#endif
#ifdef ESP32
    Serial1.println(kopya_etiket);
#endif
    delay(100);
  } else {
    if (fileExists(FILE_PLU)) {
      int pluno = 0;
      fs::File form = tryOpenFile(FILE_PLU, "r");
      String s = "0";
      while (form.position() < form.size()) {
        s = form.readStringUntil('\n');
        if (s.indexOf(",") > 0) {
          pluno++;
          int say = s.indexOf(",");
          if (pluno == pluno_click) {
            XML_PLU_ADI_S = s.substring(0, say);
            XML_PLU_KOD_S = s.substring((say + 1), (say + 8));
            float bol = 1;
            switch (ExtraTaskSettings.TaskDeviceValueDecimals[0]) {
              case 0: bol = 1; break;
              case 1: bol = 10; break;
              case 2: bol = 100; break;
              case 3: bol = 1000; break;
              case 4: bol = 10000; break;
            }
            dtostrf(XML_NET_S.toFloat() * bol, 5, 0, XML_BARKOD_C);
            int tartim = atoi(XML_BARKOD_C);
            Serial.println(XML_NET_S.toFloat() * bol);
            Serial.println(XML_BARKOD_C);
            char XML_NET_BARKOD_C[6];
            snprintf_P(XML_NET_BARKOD_C, sizeof(XML_NET_BARKOD_C), PSTR("%05u"), tartim);
  
            XML_EAN8_S = XML_PLU_KOD_S;
            XML_EAN13_S = XML_PLU_KOD_S + XML_NET_BARKOD_C;
          }
          s = "";
        } else
          s.trim();
      }
      form.close();
    }
    if ((XML_NET_S.toFloat() > 0.0001) || (Settings.UseNegatifYaz)) {
      if (Settings.UseNegatifYaz) {
        XML_TARIH_S = node_time.getDateString('-');
        XML_SAAT_S = node_time.getTimeString(':');
      }
      sno = sno + 1;
      XML_SNO_S = String(sno);
      dtostrf(sno, 4, 0, XML_SNO_C);
      fis_no = fis_no + 1;
      XML_FIS_NO_S = String(fis_no);
      dtostrf(fis_no, 8, 0, XML_FIS_NO_C);
      seri_no = seri_no + 1;
      XML_SERI_NO_S = String(seri_no);
      dtostrf(seri_no, 8, 0, XML_SERI_NO_C);
      etiket_yaz(Settings.srv_prn, 1, Settings.Web_ServisMod, 0, Settings.Cli_PrinterAktif);
      etiket_yaz(Settings.art_prn, 1, 3, 1, Settings.Cli_PrinterAktif);
    } else
      etiket_yazdir_hata(true, 1);
  }
/*
#if defined(ESP32)
  digitalWrite(5, HIGH);
  delay(2000);
  digitalWrite(5, LOW);
#endif
*/
  //return return_see_serial(event);
  return return_command_success();
}

//KOMUT
//eyzyaz#05/12/2024#19:45#EMRE ALKIN#0258#109#26.0#4.1#2.6#0#0.0#0.0#0#0.0\r\n
//eyzyaz#05/12/2024#19:45#EMRE ALKIN#0258#109#26.0#4.1#2.8#0#1.1#1.2#0#01.3#2.9#KG#1231#1232#1233#1234#1235#1236#1237#1238#1246#1245
//eyzyaz#ENDUTEK YAgLARI A.Ş.#BAŞLIK 2# BAŞLIK 3#19/09/2025#11:58#ADI SOYADI#123456789#1000.5#2345.6#54567.0#2.5#87.4#267TL#1#2#3#4#5##6#7#8#9#10#11#12#13#14#KG
String Command_Eyz_Yaz(struct EventStruct* event, const char* Line) {
  String TmpStr1;
  String TmpStr2;
  String TmpStr3;
  String TmpStr4;
  String TmpStr5;
  String TmpStr6;
  String TmpStr7;
  String TmpStr8;
  String TmpStr9;
  String TmpStr10;
  String TmpStr11;
  String TmpStr12;
  String TmpStr13;
  String TmpStr14;
  String TmpStr15;
  String TmpStr16;
  String TmpStr17;
  String TmpStr18;
  String TmpStr19;
  String TmpStr20;
  String TmpStr21;
  String TmpStr22;
  String TmpStr23;
  String TmpStr24;
  String TmpStr25;
  String TmpStr26;
  String TmpStr27;
  String TmpStr28;
  String TmpStr29;
  String TmpStr30;
  //String md5data;
  if (GetArgv(Line, TmpStr1, 2))
    XML_V1 = TmpStr1;
  if (GetArgv(Line, TmpStr2, 3))
    XML_V2 = TmpStr2;
  if (GetArgv(Line, TmpStr3, 4))
    XML_V3 = TmpStr3;
  if (GetArgv(Line, TmpStr4, 5))
    XML_V4 = TmpStr4;
  if (GetArgv(Line, TmpStr5, 6))
    XML_V5 = TmpStr5;
  if (GetArgv(Line, TmpStr6, 7))
    XML_V6 = TmpStr6;
  if (GetArgv(Line, TmpStr7, 8))
    XML_V7 = TmpStr7;
  if (GetArgv(Line, TmpStr8, 9))
    XML_V8 = TmpStr8;
  if (GetArgv(Line, TmpStr9, 10))
    XML_V9 = TmpStr9;
  if (GetArgv(Line, TmpStr10, 11))
    XML_V10 = TmpStr10;
  if (GetArgv(Line, TmpStr11, 12))
    XML_V11 = TmpStr11;
  if (GetArgv(Line, TmpStr12, 13))
    XML_V12 = TmpStr12;
  if (GetArgv(Line, TmpStr13, 14))
    XML_V13 = TmpStr13;
  if (GetArgv(Line, TmpStr14, 15))
    XML_V14 = TmpStr14;
  if (GetArgv(Line, TmpStr15, 16))
    XML_V15 = TmpStr15;
  if (GetArgv(Line, TmpStr16, 17))
    XML_V16 = TmpStr16;
  if (GetArgv(Line, TmpStr17, 18))
    XML_V17 = TmpStr17;
  if (GetArgv(Line, TmpStr18, 19))
    XML_V18 = TmpStr18;
  if (GetArgv(Line, TmpStr19, 20))
    XML_V19 = TmpStr19;  
  if (GetArgv(Line, TmpStr20, 21))
    XML_V20 = TmpStr20;
  if (GetArgv(Line, TmpStr21, 22))
    XML_V21 = TmpStr21;
  if (GetArgv(Line, TmpStr22, 23))
    XML_V22 = TmpStr22;
  if (GetArgv(Line, TmpStr23, 24))
    XML_V23 = TmpStr23;
  if (GetArgv(Line, TmpStr24, 25))
    XML_V24 = TmpStr24;
  if (GetArgv(Line, TmpStr25, 26))
    XML_V25 = TmpStr25;
  if (GetArgv(Line, TmpStr26, 27))
    XML_V26 = TmpStr26;
  if (GetArgv(Line, TmpStr27, 28))
    XML_V27 = TmpStr27;
  if (GetArgv(Line, TmpStr28, 29))
    XML_V28 = TmpStr28;
  if (GetArgv(Line, TmpStr29, 30))
    XML_V29 = TmpStr29;
  /*if (GetArgv(Line, TmpStr1, 10)) {
    md5data = TmpStr1;
    String md5chck = "eyzyaz";
    md5chck += "#";
    md5chck += XML_FIRMA_ADI_S;
    md5chck += "#";
    md5chck += XML_TARIH_V;
    md5chck += "#";
    md5chck += XML_SAAT_V;
    md5chck += "#";
    md5chck += XML_PLAKA_NO_S;
    md5chck += "#";
    md5chck += XML_OPERATOR_ADI_S;
    md5chck += "#";
    md5chck += XML_MUSTERI_ADI_S;
    md5chck += "#";
    md5chck += XML_PLU_ADI_S;
    md5chck += "#";
    md5chck += XML_NET_V;
    md5chck += "#";
    //Serial.println(md5chck);
    _md5.begin();
    _md5.add(md5chck);
    _md5.calculate();
    _md5.toString();*/
    //Serial.println(_md5.toString());
    //Serial.println(md5data);
  //}
  //if (_md5.toString() == md5data) {
    etiket_yaz(Settings.tek_prn, 1, Settings.Web_ServisMod, 1, Settings.Cli_PrinterAktif);  //SERİAL AKTİF
  /*} else {
    etiket_yazdir_hata(true, 1);
    return return_command_failed();
  }*/
  //return return_command_yazdir();
  return return_command_success();
}

String Command_Eyz_Yaz_Satir(struct EventStruct* event, const char* Line) {
  String TmpStr1;
  String message;
  if (GetArgv(Line, TmpStr1, 2)) {
    message = TmpStr1;
    message += "\r\n";
  }
  if (message.length() > 0) {
    Serial1.println("BEEP");
    yazdir_c[0] = 0;
    strncpy(yazdir_c, message.c_str(), message.length());
    for (int i = 0; i < message.length(); i++) {
      if (yazdir_c[i] == 94) {Serial1.write(0);}        
      else if ((karakter_195) && (yazdir_c[i] == 135)) { Serial1.write(128); karakter_195 = false;} //Ç
      else if ((karakter_195) && (yazdir_c[i] == 167)) { Serial1.write(135); karakter_195 = false;} //ç
      else if ((karakter_195) && (yazdir_c[i] == 150)) { Serial1.write(153); karakter_195 = false;} //Ö
      else if ((karakter_195) && (yazdir_c[i] == 182)) { Serial1.write(148); karakter_195 = false;} //ö
      else if ((karakter_195) && (yazdir_c[i] == 156)) { Serial1.write(154); karakter_195 = false;} //Ü
      else if ((karakter_195) && (yazdir_c[i] == 188)) { Serial1.write(129); karakter_195 = false;} //ü
      else if ((karakter_196) && (yazdir_c[i] == 158)) { Serial1.write(166); karakter_196 = false;} //Ğ
      else if ((karakter_196) && (yazdir_c[i] == 159)) { Serial1.write(167); karakter_196 = false;} //ğ
      else if ((karakter_196) && (yazdir_c[i] == 176)) { Serial1.write(152); karakter_196 = false;} //İ
      else if ((karakter_196) && (yazdir_c[i] == 177)) { Serial1.write(141); karakter_196 = false;} //ı
      else if ((karakter_197) && (yazdir_c[i] == 158)) { Serial1.write(158); karakter_197 = false;} //Ş
      else if ((karakter_197) && (yazdir_c[i] == 159)) { Serial1.write(159); karakter_197 = false;} //ş
      else if (yazdir_c[i] == 195) {karakter_195 = true;}
      else if (yazdir_c[i] == 196) {karakter_196 = true;}
      else if (yazdir_c[i] == 197) {karakter_197 = true;}
      else {Serial1.write(yazdir_c[i]);}
    }
  } else {
    etiket_yazdir_hata(true, 1);
    return return_command_failed();
  }
  //return return_command_yazdir();
  return return_command_success();
}

String Command_Eyz_tspl(struct EventStruct* event, const char* Line) {
  String TmpStr1;
  int time = 5;
  if (GetArgv(Line, TmpStr1, 2))
    time = TmpStr1.toInt();
#ifdef ESP32
  Serial1.println("BEEP");
#endif
  tspl_mod = true;
  tspl_time = millis() + (time * 1000);
  //return return_command_yazdir();
  return return_command_success();
}

String Command_Eyz_Net(struct EventStruct* event, const char* Line) {
#ifdef ESP32
  Serial1.println("BEEP");
#endif
#if defined(ESP32)
#ifdef HAS_BLUETOOTH
  if (Settings.bluetooth_mod == 2)
    //SerialBT.println("TARTIM:" + XML_NET_S);
    Serial2.println("TARTIM:" + XML_NET_S);
#endif
#endif
  return return_command_success();
}

String Command_Eyz_Serino(struct EventStruct* event, const char* Line) {
  String TmpStr1 = "0";
  if (GetArgv(Line, TmpStr1, 2))
    seri_no = TmpStr1.toInt();
  XML_SERI_NO_S = String(seri_no);
  dtostrf(seri_no, 8, 0, XML_SERI_NO_C);
  int address = 0;
  EEPROM.writeLong(address, seri_no);
  address += sizeof(uint32_t);
  EEPROM.commit();
  /*for (int addr = 0; addr < 9; addr++) {
    EEPROM.write(addr, (byte)(XML_SERI_NO_C[addr]));
  }*/
  return return_command_success();
}
#endif