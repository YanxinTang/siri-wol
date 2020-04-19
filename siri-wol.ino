#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "Your_SSID"
#define STAPSK  "Your_Password"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

const char *remoteIP = "Your_PC_IP";
const unsigned int remotePort = 7;
// Your_PC_MAC_Address
const unsigned char MACAddress[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


WiFiUDP UDP;

unsigned char packet[102];
void makeWOLPacket(unsigned char* packet);
void sendWOLPacket();
void handleWOL();
void handleNotFound();

void setup(void) {
  makeWOLPacket(packet);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/wol", handleWOL);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}

void makeWOLPacket(unsigned char* packet) {
  int i, j;
  for (i = 0; i < 6; i++) {
    packet[i] = 0xFF;  
  }
  for (i = 1; i < 17; i++) {
    for (j = 0; j < 6; j++) {
      packet[i*6+j] = MACAddress[j];
    } 
  }
}

void sendWOLPacket() {
  UDP.beginPacket(remoteIP, remotePort);
  UDP.write(packet, sizeof packet);
  UDP.endPacket();
}

void handleWOL() {
  digitalWrite(LED_BUILTIN, LOW);
  sendWOLPacket();
  char result[400] = "{ \"code\": 0, \"message\": \"Hi, 已帮您唤醒电脑\" }";
  server.send(200, "application/json", result);
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleNotFound() {
  digitalWrite(LED_BUILTIN, LOW);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, HIGH);
}
