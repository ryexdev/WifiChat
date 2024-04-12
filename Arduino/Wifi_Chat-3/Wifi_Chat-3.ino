#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer_Generic.h>

#define CHATNAME "ChatRoom"
#define HTTP_CODE 200 
#define DNS_PORT 53  

IPAddress APIP(10, 10, 10, 1);
String allMsgs = "<i>*system restarted*</i>";
DNSServer dnsServer;
ESP8266WebServer webServer(80);
WebSocketsServer webSocket = WebSocketsServer(81); 

void emit(String s) {
  Serial.println(s);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      webSocket.broadcastTXT(payload);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  emit("setup");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(CHATNAME);
  
  dnsServer.start(DNS_PORT, "*", APIP);
  webServer.onNotFound([]() {
    String page = R"=====(
    <!DOCTYPE html>
    <html>
    <head>
        <title>)=====" + String(CHATNAME) + R"=====(
        </title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
            body {
                color: #333;
                font-family: Century Gothic, sans-serif;
                font-size: 18px;
                line-height: 24px;
                margin: 0;
                padding: 0;
            }
            nav {
                background: #566656;
                color: #fff;
                padding: 1em;
                font-size: 1.3em;
            }
            input, textarea {
                font-size: 16px;
            }
            label, h1 {
                display: block;
                font-style: italic;
                font-weight: bold;
            }
            textarea {
                width: 100%;
            }
        </style>
        <script>
            document.addEventListener('DOMContentLoaded', function() {
                const ws = new WebSocket('ws://' + location.hostname + ':81/');
                ws.addEventListener('message', function(event) {
                    document.querySelector('ol').innerHTML = '<li>' + event.data + '</li>' + document.querySelector('ol').innerHTML;
                });
                document.querySelector('form').addEventListener('submit', function(e) {
                    e.preventDefault();
                    const username = document.getElementById('username').value;
                    const message = document.querySelector('textarea').value;
                    ws.send(username + ': ' + message);
                    document.querySelector('textarea').value = '';
                });
            });
        </script>
    </head>
    <body>
        <nav>
            <b>)=====" + String(CHATNAME) + R"=====(
            </b>
        </nav>
        <div>
            <label>Your name:</label>
            <input type="text" id="username" name="username">
        </div>
        <div>
            <label>Messages:</label>
            <ol>)=====" + allMsgs + R"=====(
            </ol>
        </div>
        <div>
            <form action="/post" method="post">
                <label>Post new message:</label>
                <textarea name="m"></textarea><br/>
                <input type="submit" value="send">
            </form>
        </div>
    </body>
    </html>
    )=====";
        webServer.send(HTTP_CODE, "text/html", page);
    });

    webServer.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    }

    void loop() {
        dnsServer.processNextRequest();
        webServer.handleClient();
        webSocket.loop();
    }
