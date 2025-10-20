#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include "ESPEasy-Globals.h"
#include "src/Commands/InternalCommands.h"
#include "src/Globals/Settings.h"

#define TCP_CMD_PORT 9101
#define TCP_IDLE_TIMEOUT_MS (5UL * 60UL * 1000UL)  // 5 dk (isteğe bağlı)

WiFiServer tcpCmdServer(TCP_CMD_PORT);

// Kalıcı istemci ve buffer
static WiFiClient tcpClient;
static String     cmdBuf;
static unsigned long lastActivityMs = 0;

void Plugin_999_Init()
{
  tcpCmdServer.begin();
  tcpCmdServer.setNoDelay(true); // küçük paketlerde gecikmeyi azalt
  addLog(LOG_LEVEL_INFO, F("TCPCommand: server started on port 8383"));
}

static void handleCommandLine(const String& line)
{
  if (line.isEmpty()) return;

  addLog(LOG_LEVEL_INFO, String(F("TCPCommand: ")) + line);

  String reply;
  // SADECE internal komut; gerçek status döner
  ExecuteCommand_internal_withReply(EventValueSource::Enum::VALUE_SOURCE_HTTP,
                                    line.c_str(),
                                    reply,
                                    true);

  if (tcpClient.connected()) {
    tcpClient.println(reply);   // "OK" değil, komutun cevabı / unknown mesajı
    //tcpClient.print(F("> "));
  }
}



void Plugin_999_Loop()
{
  // 1) Yeni bağlantı varsa ve şu an bağlı değilsek kabul et
  if ((!tcpClient) || (!tcpClient.connected())) {
    WiFiClient newClient = tcpCmdServer.available();
    if (newClient) {
      // İsteğe bağlı: mevcut bağlıyı kapat
      if (tcpClient && tcpClient.connected()) {
        tcpClient.stop();
      }
      tcpClient = newClient;
      tcpClient.setTimeout(1); // readString* kullanmıyoruz, ama dursun
      lastActivityMs = millis();

      //tcpClient.println(F("Welcome to ESPEasy TCP command port"));
      //tcpClient.print(F("> "));
      addLog(LOG_LEVEL_INFO, F("TCPCommand: client connected"));
    }
    return; // Bağlı değilsek daha ileri gitme
  }

  // 2) Bağlıyken non-blocking oku
  bool gotData = false;
  while (tcpClient.connected() && tcpClient.available() > 0) {
    int b = tcpClient.read();
    if (b < 0) break;
    gotData = true;

    char c = static_cast<char>(b);

    // CR/LF normalize: '\n' veya '\r' gördüğünde komutu işle
    if (c == '\n' || c == '\r') {
      if (cmdBuf.length() > 0) {
        String line = cmdBuf;
        cmdBuf = "";
        line.trim();
        handleCommandLine(line);
      }
      // birden çok CR/LF gelirse boş satırı yok say
    } else {
      // Basit koruma: aşırı uzun satırı kes
      if (cmdBuf.length() < 1024) {
        cmdBuf += c;
      } else {
        cmdBuf = "";
        tcpClient.println(F("ERR Line too long"));
        tcpClient.print(F("> "));
      }
    }
  }

  if (gotData) {
    lastActivityMs = millis();
  }

  // 3) İsteğe bağlı idle timeout
  if (TCP_IDLE_TIMEOUT_MS > 0 && (millis() - lastActivityMs) > TCP_IDLE_TIMEOUT_MS) {
    tcpClient.println(F("Idle timeout, bye."));
    tcpClient.stop();
    addLog(LOG_LEVEL_INFO, F("TCPCommand: client disconnected (idle)"));
  }
}
