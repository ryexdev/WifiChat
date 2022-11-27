#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

String responseHTML = ""
"<!DOCTYPE html><html><body>"
"<h2>Someone is Curious</h2>"
//*************Form*************
"<form  name='frm' id='frm' method='get'>"
"Go Ahead. Say Anything."
"<input type='text' name='InputText' value='No Ones Watching....'><br><input type='submit' value='Submit'></form>"
"<SCRIPT TYPE='text/JavaScript'>.function mf(){document.getElementById('frm').submit;}</SCRIPT>"
//*************Results*************
"<div><p>"
"Test Message"
"</p></div>"
//
"</body></html>"
;

#define MaxHeaderLength 16
String HttpHeader = String(MaxHeaderLength);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("cOmE oN iN");
  dnsServer.start(DNS_PORT, "*", apIP);
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", responseHTML);
  });
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
  delay(100);
}
