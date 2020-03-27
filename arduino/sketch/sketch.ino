#include <ESP8266WiFi.h>
#include <time.h>


String split(String s, char parser, int index) {
  String rs="";
  int parserIndex = index;
  int parserCnt=0;
  int rFromIndex=0, rToIndex=-1;
  while (index >= parserCnt) {
    rFromIndex = rToIndex+1;
    rToIndex = s.indexOf(parser,rFromIndex);
    if (index == parserCnt) {
      if (rToIndex == 0 || rToIndex == -1) return "";
      return s.substring(rFromIndex,rToIndex);
    } else parserCnt++;
  }
  return rs;
}


BearSSL::WiFiClientSecure client;
BearSSL::Session session;


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  client.setInsecure();
}


void loop() {
  if(Serial.available()) {
    String s = Serial.readStringUntil(10);
    if(s == "AT") {
      Serial.println("OK");
    }
    if(s == "AT+RESET") {
       WiFi.mode(WIFI_STA);
       WiFi.disconnect();
    }
    if(s == "AT+LIST_AP") {
      int networksFound = WiFi.scanNetworks();

      for (int i = 0; i < networksFound; i++)
      {
        Serial.printf("%d,\"%s\",%d,%d,\"%s\"\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "encrypted");
      }
    }
    if(s.startsWith("AT+CONNECT=")) {
      String ssid = split(s, '"', 1);
      String pass = split(s, '"', 3);
      WiFi.begin(ssid, pass);
    }
    if(s == "AT+DISCONNECT") {
      WiFi.disconnect();
    }
    if(s.startsWith("AT+TLS_START=")) {
      String conStr = s.substring(13);
      String hostStr = conStr.substring(0, conStr.indexOf(','));
      String portStr = conStr.substring(conStr.indexOf(',')+1);
      uint16_t port = portStr.toInt();
      client.setSession(&session);
      client.connect(hostStr.c_str(), port);
      if (!client.connected())  Serial.printf("*** Can't connect to %s:%d ***\n-------\n", hostStr.c_str(), port);
      else Serial.printf("Connected!\n-------\n");
    }
    if(s.startsWith("AT+SEND=")) {
      String numBytesStr = s.substring(8);
      int numBytes = numBytesStr.toInt();
      char* buf = new char[numBytes];
      Serial.readBytes(buf, numBytes);
      client.write(buf);
      delete[] buf;
    }
    if(s == "AT+TLS_CLOSE") {
      client.stop();
    }
    if(s == "AT+INIT_TLS") {
      configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

      Serial.print("Waiting for NTP time sync: ");
      time_t now = time(nullptr);
      while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
      }
      Serial.println("");
      struct tm timeinfo;
      gmtime_r(&now, &timeinfo);
      Serial.print("Current time: ");
      Serial.print(asctime(&timeinfo));
    }
    if(s == "AT+IS_CONNECTED_TO_AP") {
      bool b = WiFi.isConnected();
      if(b) Serial.println("YES");
      else Serial.println("NO");
    }
    if(s == "AT+GET_IP") {
      if (WiFi.status() == WL_CONNECTED)
      Serial.println(WiFi.localIP());
      else Serial.println("");
    }
    if(s == "AT+GET_MAC") {
      uint8_t macAddr[6];
      WiFi.macAddress(macAddr);
      Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
    }
    if(s == "AT+CONNECTED_AP") {
      Serial.printf("%s\n", WiFi.SSID().c_str());
    }
  }

  while(client.available()) {
    String reply = client.readStringUntil('\r');
    Serial.print(reply);
  }

  delay(100);
}
