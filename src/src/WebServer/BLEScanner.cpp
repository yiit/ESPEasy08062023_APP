#include "../WebServer/BLEScanner.h"

#if defined(HAS_BLE) || defined(HAS_BLE_CLIENT)

#include "../WebServer/JSON.h"
#include "../WebServer/ESPEasy_WebServer.h"
#include "../WebServer/AccessControl.h"
#include "../WebServer/HTML_wrappers.h"
#include "../WebServer/Markup_Forms.h"
#include "../WebServer/Markup.h"
#include "../WebServer/Markup_Buttons.h"
#include "../Globals/Settings.h"
#include "../Helpers/ESPEasy_Storage.h"

#include <vector>
#include <NimBLEDevice.h>

// Global variables for BLE scanning
std::vector<BLEDeviceInfo> bleDeviceList;
bool bleScanInProgress = false;

// BLE scan completed synchronously, no callback needed

void startBLEScan() {
  if (bleScanInProgress) return;
  
  clearBLEScanResults();
  bleScanInProgress = true;
  
  // Initialize BLE if not already done
  if (!NimBLEDevice::getInitialized()) {
    NimBLEDevice::init("");
  }
  
  NimBLEScan* pBLEScan = NimBLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  pBLEScan->setDuplicateFilter(false);
  
  // Start scan for 5 seconds and process results
  NimBLEScanResults scanResults = pBLEScan->start(5, true);
  
  // Process scan results
  for (int i = 0; i < scanResults.getCount(); i++) {
    NimBLEAdvertisedDevice advertisedDevice = scanResults.getDevice(i);
    
    BLEDeviceInfo device;
    device.address = String(advertisedDevice.getAddress().toString().c_str());
    device.name = advertisedDevice.haveName() ? String(advertisedDevice.getName().c_str()) : "Unnamed Device";
    device.rssi = advertisedDevice.getRSSI();
    device.connectable = advertisedDevice.isConnectable();
    
    // Get service UUIDs
    String services = "";
    if (advertisedDevice.haveServiceUUID()) {
      services = String(advertisedDevice.getServiceUUID().toString().c_str());
    }
    device.serviceUUIDs = services;
    
    // Check if device already exists in list
    bool found = false;
    for (auto& existing : bleDeviceList) {
      if (existing.address.equals(device.address)) {
        // Update existing device info
        existing.name = device.name;
        existing.rssi = device.rssi;
        existing.connectable = device.connectable;
        existing.serviceUUIDs = device.serviceUUIDs;
        found = true;
        break;
      }
    }
    
    if (!found) {
      bleDeviceList.push_back(device);
    }
  }
  
  bleScanInProgress = false;
}

void stopBLEScan() {
  if (bleScanInProgress) {
    NimBLEDevice::getScan()->stop();
    bleScanInProgress = false;
  }
}

bool isBLEScanComplete() {
  return !bleScanInProgress;
}

void clearBLEScanResults() {
  bleDeviceList.clear();
}

// ********************************************************************************
// Web Interface BLE scanner JSON
// ********************************************************************************
void handle_blescanner_json() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_blescanner"));
  #endif

  if (!isLoggedIn()) { return; }
  if (!isLisansIn()) { return; }
  
  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startJsonStream();
  addHtml('[');
  
  bool firstEntry = true;
  for (const auto& device : bleDeviceList) {
    if (firstEntry) { 
      firstEntry = false; 
      addHtml('{');
    } else { 
      addHtml(',', '{'); 
    }
    
    stream_next_json_object_value(F("name"), device.name);
    stream_next_json_object_value(F("address"), device.address);
    stream_next_json_object_value(F("rssi"), String(device.rssi));
    stream_next_json_object_value(F("connectable"), device.connectable ? F("Yes") : F("No"));
    stream_last_json_object_value(F("services"), device.serviceUUIDs.length() > 0 ? device.serviceUUIDs : F("None"));
  }
  
  if (firstEntry) {
    addHtml('}');
  }
  
  addHtml(']');
  TXBuffer.endStream();
}

// ********************************************************************************
// Web Interface BLE scanner
// ********************************************************************************
void handle_blescanner() {
  #ifndef BUILD_NO_RAM_TRACKER
  checkRAM(F("handle_blescanner"));
  #endif

  if (!isLoggedIn()) { return; }
  if (!isLisansIn()) { return; }

  // Handle form submission for connecting to selected device
  if (hasArg(F("connect"))) {
    String selectedMAC = webArg(F("selectedmac"));
    if (selectedMAC.length() > 0) {
      // Save selected MAC to settings
      selectedMAC.toCharArray(Settings.bluetooth_mac_address, sizeof(Settings.bluetooth_mac_address));
      SaveSettings();
      
      // Show success message without reboot
      addLog(LOG_LEVEL_INFO, String(F("BLE: Device MAC address saved: ")) + selectedMAC);
    }
  }
  
  // Handle manual MAC address input
  if (hasArg(F("manual_connect"))) {
    String manualMAC = webArg(F("manual_mac"));
    if (manualMAC.length() > 0) {
      // Validate MAC format (basic check)
      if (manualMAC.length() == 17 && manualMAC.indexOf(':') > 0) {
        manualMAC.toCharArray(Settings.bluetooth_mac_address, sizeof(Settings.bluetooth_mac_address));
        SaveSettings();
        
        // Show success message without reboot
        addLog(LOG_LEVEL_INFO, String(F("BLE: Manual MAC address saved: ")) + manualMAC);
      }
    }
  }

  // Start scan if requested
  if (hasArg(F("scan"))) {
    startBLEScan();
  }
  
  // Stop scan if requested
  if (hasArg(F("stop_scan"))) {
    bleScanInProgress = false;
    addLog(LOG_LEVEL_INFO, F("BLE: Scan stopped by user"));
  }

  navMenuIndex = MENU_INDEX_TOOLS;
  TXBuffer.startStream();
  sendHeadandTail_stdtemplate(_HEAD);
  
  addHtml(F("<h2>BLE Device Scanner</h2>"));
  
  // Current connected device info
  addFormSubHeader(F("Current BLE Device"));
  if (strlen(Settings.bluetooth_mac_address) > 0) {
    addHtml(F("<p><b>Connected MAC:</b> "));
    addHtml(Settings.bluetooth_mac_address);
    addHtml(F("</p>"));
  } else {
    addHtml(F("<p><i>No BLE device configured</i></p>"));
  }
  
  // Manual MAC input section
  addFormSubHeader(F("Manual MAC Address"));
  addHtml(F("<form method='post'>"));
  addHtml(F("<table><tr><td style='padding: 5px;'>MAC Address:</td><td style='padding: 5px;'>"));
  addHtml(F("<input type='text' name='manual_mac' value='"));
  addHtml(Settings.bluetooth_mac_address);
  addHtml(F("' placeholder='DC:0D:30:E3:12:6C' maxlength='17' pattern='[0-9A-Fa-f:]{17}' style='width: 150px;'>"));
  addHtml(F("</td><td style='padding: 5px;'>"));
  addSubmitButton(F("Save MAC"), F("manual_connect"));
  addHtml(F("</td></tr></table></form>"));
  addHtml(F("<small><i>Format: XX:XX:XX:XX:XX:XX (with colons)</i></small>"));
  
  // Scan controls
  addFormSubHeader(F("BLE Device Scan"));
  
  addHtml(F("<form method='post'>"));
  if (bleScanInProgress) {
    addHtml(F("<p><i>Scanning in progress... Please wait.</i></p>"));
    addHtml(F("<input type='submit' name='stop_scan' value='Stop Scan'>"));
  } else {
    addSubmitButton(F("Start Scan"), F("scan"));
  }
  addHtml(F("</form>"));
  
  // Display scan results
  if (!bleDeviceList.empty()) {
    addFormSubHeader(F("Found BLE Devices"));
    addHtml(F("<form method='post'>"));
    html_table_class_multirow();
    html_TR();
    html_table_header(F("Select"));
    html_table_header(F("Device Name"));
    html_table_header(F("MAC Address"));
    html_table_header(F("RSSI"));
    html_table_header(F("Connectable"));
    html_table_header(F("Services"));

    for (const auto& device : bleDeviceList) {
      html_TR_TD();
      
      // Radio button for selection
      addHtml(F("<input type='radio' name='selectedmac' value='"));
      addHtml(device.address);
      addHtml(F("'>"));
      
      html_TD();
      addHtml(device.name);
      
      html_TD();
      addHtml(device.address);
      
      html_TD();
      addHtml(String(device.rssi));
      addHtml(F(" dBm"));
      
      html_TD();
      addHtml(device.connectable ? F("Yes") : F("No"));
      
      html_TD();
      addHtml(device.serviceUUIDs.length() > 0 ? device.serviceUUIDs : F("None"));
    }
    
    html_end_table();
    addHtml(F("<br>"));
    addSubmitButton(F("Connect to Selected Device"), F("connect"));
    addHtml(F("</form>"));
  } else if (!bleScanInProgress) {
    addHtml(F("<p><i>No devices found. Click 'Start Scan' to search for BLE devices.</i></p>"));
  }
  
  // Success/Error messages
  if (hasArg(F("connect")) || hasArg(F("manual_connect"))) {
    addHtml(F("<div style='background-color: #d4edda; border: 1px solid #c3e6cb; color: #155724; padding: 10px; margin: 10px 0; border-radius: 4px;'>"));
    addHtml(F("✓ BLE MAC address saved successfully! Current device: "));
    addHtml(Settings.bluetooth_mac_address);
    addHtml(F("</div>"));
  }

  sendHeadandTail_stdtemplate(_TAIL);
  TXBuffer.endStream();
}

#endif // defined(HAS_BLE) || defined(HAS_BLE_CLIENT)