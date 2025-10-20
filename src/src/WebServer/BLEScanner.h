#ifndef WEBSERVER_WEBSERVER_BLESCANNER_H
#define WEBSERVER_WEBSERVER_BLESCANNER_H

#include "../WebServer/common.h"

#if defined(HAS_BLE) || defined(HAS_BLE_CLIENT)

// ********************************************************************************
// Web Interface BLE scanner
// ********************************************************************************
void handle_blescanner();
void handle_blescanner_json();

// BLE device structure for scanning results
struct BLEDeviceInfo {
  String name;
  String address;
  int rssi;
  bool connectable;
  String serviceUUIDs;
  
  BLEDeviceInfo(const String& n = "", const String& addr = "", int r = 0, bool conn = false, const String& services = "") 
    : name(n), address(addr), rssi(r), connectable(conn), serviceUUIDs(services) {}
};

// Global BLE scan results
extern std::vector<BLEDeviceInfo> bleDeviceList;
extern bool bleScanInProgress;

// BLE scanning functions
void startBLEScan();
void stopBLEScan();
bool isBLEScanComplete();
void clearBLEScanResults();

#endif // defined(HAS_BLE) || defined(HAS_BLE_CLIENT)

#endif // WEBSERVER_WEBSERVER_BLESCANNER_H