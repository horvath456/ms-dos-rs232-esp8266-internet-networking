#include <ESP8266WiFi.h>
#include <time.h>

// #define DEBUG


BearSSL::WiFiClientSecure client;
BearSSL::Session session;

char readbuffer[256];
char bigbuf1[256];
char smallbuf1[64];
char smallbuf2[64];


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  client.setInsecure();
}


void loop() {
  if (Serial.available()) {

    size_t len = Serial.readBytesUntil('\n', readbuffer, 255);
    readbuffer[len] = 0; // null-terminate c-string

    if(len > 0) {
      if (strcmp(readbuffer, "AT") == 0) {
        Serial.println("OK");
      }
      else if (strcmp(readbuffer, "AT+INFO") == 0) {
        Serial.println("REAL_ESP8266");
      }
      else if (strcmp(readbuffer, "AT+RESET") == 0) {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        Serial.println("OK");
      }
      else if (strcmp(readbuffer, "AT+GET_MAC") == 0) {
        uint8_t macAddr[6];
        WiFi.macAddress(macAddr);
        Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
      }
      else if (strcmp(readbuffer, "AT+LIST_AP") == 0) {
        int networksFound = WiFi.scanNetworks();
  
        for (int i = 0; i < networksFound; i++)
        {
          Serial.printf("%d,%s,%d,%d,%d\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i));
        }
  
        if (networksFound == 0) {
          Serial.println("NO NETWORKS FOUND");
        }
      }
      else if (strncmp(readbuffer, "AT+CONNECT_TO_AP", 16) == 0) {
        bool startreadSSID = false;
        bool readSSID = false;
        bool startreadPass = false;
        bool readPass = false;
        size_t smallbuf1_cnt = 0;
        size_t smallbuf2_cnt = 0;
  
        for (int i = 10; i < len; i++) {
          if (i == 10) continue; // = char
          char c = readbuffer[i];
          if (c == '"') {
            if (!startreadSSID && !readSSID) startreadSSID = true;
            else if (startreadSSID && !readSSID) readSSID = true;
            else if (startreadSSID && readSSID && !startreadPass && !readPass) startreadPass = true;
            else if (startreadSSID && readSSID && startreadPass && !readPass) readPass = true;
          } else {
            if (startreadSSID && !readSSID) {
              smallbuf1[smallbuf1_cnt] = c;
              smallbuf1_cnt++;
            }
            else if (startreadPass && !readPass) {
              smallbuf2[smallbuf2_cnt] = c;
              smallbuf2_cnt++;
            } else {
              // ignore char (e.g. ,)
            }
          }
        }
        smallbuf1[smallbuf1_cnt] = 0; // null-terminate c-string
        smallbuf2[smallbuf2_cnt] = 0; // null-terminate c-string
  
        WiFi.begin(smallbuf1, smallbuf2);

        delay(2000); // add a delay
        
        Serial.println("OK");
      }
      else if (strcmp(readbuffer, "AT+DISCONNECT") == 0) {
        WiFi.disconnect();
        Serial.println("OK");
      }
      else if (strcmp(readbuffer, "AT+IS_CONNECTED_TO_AP") == 0) {
        bool b = WiFi.isConnected();
        if (b) Serial.println("YES");
        else Serial.println("NO");
      }
      else if (strcmp(readbuffer, "AT+CONNECTED_AP") == 0) {
        Serial.printf("%s\n", WiFi.SSID().c_str());
      }
      else if (strcmp(readbuffer, "AT+GET_IP") == 0) {
        if (WiFi.status() == WL_CONNECTED)
          Serial.println(WiFi.localIP());
        else Serial.println("NOT CONNECTED");
      }
      else if (strcmp(readbuffer, "AT+INIT_TLS") == 0) {
        configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  
        #ifdef DEBUG
        Serial.print("Waiting for NTP time sync: ");
        #endif
        
        time_t now = time(nullptr);
        while (now < 8 * 3600 * 2) {
          delay(500);
          
          #ifdef DEBUG
          Serial.print(".");
          #endif
          
          now = time(nullptr);
        }
        
        struct tm timeinfo;
        gmtime_r(&now, &timeinfo);

        delay(2000); // add a delay
        
        #ifdef DEBUG
        Serial.println("");
        Serial.print("Current time: ");
        Serial.print(asctime(&timeinfo));
        #endif
        Serial.println("OK");
      }
      else if (strncmp(readbuffer, "AT+TLS_START_HOSTNAME=", 21) == 0) {
        bool startreadHostname = false;
        bool readHostname = false;
        bool startreadPort = false;
        bool doneReadingPort = false;
        size_t bigbuf1_cnt = 0;
        size_t smallbuf1_cnt = 0;
  
        for (int i = 20; i < len; i++) {
          if (i == 20) continue; // = char
          char c = readbuffer[i];
  
          if (c == '"') {
            if (!startreadHostname && !readHostname) startreadHostname = true;
            else if (startreadHostname && !readHostname) readHostname = true;
            else if (startreadHostname && readHostname && !startreadPort && !doneReadingPort)
              startreadPort = true;
            else if (startreadHostname && readHostname && startreadPort && !doneReadingPort)
              doneReadingPort = true;
          } else {
            if (startreadHostname && !readHostname) {
              bigbuf1[bigbuf1_cnt] = c;
              bigbuf1_cnt++;
            }
            else if (startreadPort && !doneReadingPort) {
              smallbuf1[smallbuf1_cnt] = c;
              smallbuf1_cnt++;
            } else {
              // ignore char (e.g. ,)
            }
          }
        }
        bigbuf1[bigbuf1_cnt] = 0; // null-terminate c-string
        smallbuf1[smallbuf1_cnt] = 0; // null-terminate c-string
  
        uint16_t port = atoi(smallbuf1);

        delay(2000); // add a delay
  
        client.setSession(&session);
        client.connect(bigbuf1, port);

        delay(1000); // add a delay
        
        if (!client.connected())  Serial.printf("*** Can't connect to %s:%d ***\n---\n", bigbuf1, port);
        else Serial.printf("CONNECTED\n---\n");
      }
      else if (strncmp(readbuffer, "AT+TLS_START_IP=", 16) == 0) {
        bool startreadHostname = false;
        bool readHostname = false;
        bool startreadPort = false;
        bool doneReadingPort = false;
        size_t smallbuf1_cnt = 0;
        size_t smallbuf2_cnt = 0;
  
        for (int i = 15; i < len; i++) {
          if (i == 15) continue; // = char
          char c = readbuffer[i];
  
          if (c == '"') {
            if (!startreadHostname && !readHostname) startreadHostname = true;
            else if (startreadHostname && !readHostname) readHostname = true;
            else if (startreadHostname && readHostname && !startreadPort && !doneReadingPort)
              startreadPort = true;
            else if (startreadHostname && readHostname && startreadPort && !doneReadingPort)
              doneReadingPort = true;
          } else {
            if (startreadHostname && !readHostname) {
              smallbuf1[smallbuf1_cnt] = c;
              smallbuf1_cnt++;
            }
            else if (startreadPort && !doneReadingPort) {
              smallbuf2[smallbuf2_cnt] = c;
              smallbuf2_cnt++;
            } else {
              // ignore char (e.g. ,)
            }
          }
        }
        smallbuf1[smallbuf1_cnt] = 0; // null-terminate c-string
        smallbuf2[smallbuf2_cnt] = 0; // null-terminate c-string
  
        // now smallbuf1 contains the ip, it needs to be parsed
        // smallbuf2 contains the port
  
        uint8_t ip_array[4];
        uint8_t ip_array_cnt = 0;
        char ip_part_str[4];
        uint8_t ip_part_str_cnt = 0;
  
        for (int i = 0; i < smallbuf1_cnt; i++) {
          char c = smallbuf1[i];
          if (c == '.') {
            ip_part_str[ip_part_str_cnt] = 0;
            ip_array[ip_array_cnt] = atoi(ip_part_str);
            ip_array_cnt++;
            ip_part_str_cnt = 0;
          } else {
            ip_part_str[ip_part_str_cnt] = c;
            ip_part_str_cnt++;
          }
        }
        ip_part_str[ip_part_str_cnt] = 0;
        ip_array[ip_array_cnt] = atoi(ip_part_str);
  
        IPAddress ip(ip_array[0], ip_array[1], ip_array[2], ip_array[3]);
        uint16_t port = atoi(smallbuf2);

        delay(2000); // add a delay
  
        client.setSession(&session);
        client.connect(ip, port);

        delay(1000); // add a delay
        
        if (!client.connected())  Serial.printf("*** Can't connect to %d.%d.%d.%d:%d ***\n---\n", ip_array[0], ip_array[1], ip_array[2], ip_array[3], port);
        else Serial.printf("CONNECTED\n---\n");
      }
      else if (strncmp(readbuffer, "AT+SEND=", 8) == 0) {
          size_t strlen_readbuffer = strlen(readbuffer);
          size_t bytes_to_copy = strlen_readbuffer - 8;
          strncpy(smallbuf1, readbuffer+8, bytes_to_copy);
          smallbuf1[bytes_to_copy] = 0; // null-termianate c-string
          size_t bytes_to_send = atoi(smallbuf1);
  
          if(bytes_to_send > 255) bytes_to_send = 255; // buffer size is 256, dont want overflow
  
          #ifdef DEBUG
          Serial.printf("Sending %d bytes..\n", bytes_to_send);
          #endif
          
          size_t bytes_sent = Serial.readBytes(bigbuf1, bytes_to_send);
          bigbuf1[bytes_sent] = 0; // null-termianate c-string
          client.write(bigbuf1);
  
          #ifdef DEBUG
          Serial.printf("Sent %d bytes..\n", bytes_sent);
          #endif
      }
      else if (strcmp(readbuffer, "AT+TLS_CLOSE") == 0) {
        client.stop();
        Serial.println("OK");
      }
      else {
        Serial.println("*** UNSUPPORTED COMMAND ***");
      }
    }
  }

  while (client.available()) {
    char reply = client.read();
    Serial.print(reply);
  }

  delay(100);
}
