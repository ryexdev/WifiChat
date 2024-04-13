#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "ChatHTML.h"

const char* ssid = "Chat Room";
DNSServer dnsServer;
AsyncWebServer server(80);

String chatData = "";

String previousIP = "";

void setup() {
  pinMode(25, OUTPUT);
  digitalWrite(25, LOW);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);

  dnsServer.start(53, "*", WiFi.softAPIP());

  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(200, "text/html", chatHTML);
  });

  server.on("/chat", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", chatData);
  });

server.on("/send", HTTP_GET, [](AsyncWebServerRequest *request){
    String message = request->getParam("msg")->value();
    String ip = request->client()->remoteIP().toString();
    // Keep the original IP for other uses

    // Create a new variable for the modified IP, removing dots
    String modifiedIP = ip;
    modifiedIP.replace(".", "");

    // Simple hash-like operation to ensure uniqueness
    long ipNum = modifiedIP.toInt();
    unsigned long hash = (ipNum * 2654435761) % 4294967296; // Example simple hash function

    String username = "user" + String(hash);
    
    if (ip != previousIP)
        chatData += "--------------------------------\n";
    chatData += username + ": " + message + "\n";
    previousIP = ip;
    
    request->send(200, "text/plain", "OK");
});


  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
}
