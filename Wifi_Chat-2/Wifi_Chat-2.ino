#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WebSocketsServer_Generic.h>

const byte DNS_PORT = 53;
ESP8266WebServer server(80);
DNSServer dnsServer;
WebSocketsServer webSocket = WebSocketsServer(81);

const int maxMessages = 5;
String messages[maxMessages];

String page = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>Chat Room</title>
<style>
  body {
    font-family: Arial, sans-serif;
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    min-height: 100vh;
    margin: 0;
    padding: 0;
  }
  ul {
    max-width: 400px;
    width: 100%;
    list-style-type: none;
    padding: 0;
    overflow-y: auto;
    max-height: 400px;
    margin-bottom: 10px;
  }
  li {
    border: 1px solid #ccc;
    margin: 8px 0;
    padding: 8px;
    border-radius: 5px;
  }
  button {
    background-color: #4CAF50; /* Green */
    border: none;
    color: white;
    padding: 15px 32px;
    text-align: center;
    text-decoration: none;
    display: inline-block;
    font-size: 16px;
    margin: 4px 2px;
    transition-duration: 0.4s;
    cursor: pointer;
  }
  button:hover {
    background-color: white;
    color: black;
    border: 1px solid #4CAF50;
  }
  input {
    padding: 10px;
    width: 80%;
    max-width: 300px;
    font-size: 16px;
  }
  @media (max-width: 600px) {
    ul {
      max-height: 300px;
    }
    input {
      width: calc(100% - 20px);
    }
  }
</style>
</head>
<body>
<ul id="chat"></ul>
<input id="msg" type="text" placeholder="Type your message here..." />
<button onclick="send()">Send</button>
<script>
  var ws = new WebSocket('ws://' + location.hostname + ':81/');
  var userName = "";

  ws.onmessage = function(evt) {
    var chat = document.getElementById('chat');
    var msg = document.createElement('li');
    
    var data = evt.data.split('###');
    var sender = data[0];
    var message = data[1];
    
    msg.innerHTML = "<b>" + sender + ":</b> " + message;

    if(sender === userName) {
      msg.style.backgroundColor = "blue";
      msg.style.color = "white";
    } else {
      msg.style.backgroundColor = "grey";
      msg.style.color = "white";
    }
    chat.appendChild(msg);
    chat.scrollTop = chat.scrollHeight;
  };
  
  ws.onopen = function() {
    userName = prompt("Enter your name:");
    ws.send("SET_NAME###" + userName);
  };
  
  function send() {
    var msg = document.getElementById('msg').value;
    ws.send("MSG###" + msg);
    document.getElementById('msg').value = '';
  }
</script>
</body>
</html>
)=====";

String clientNames[5]; // An array to store client names.

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_TEXT: {
            String msg = String((char *)payload);
            if (msg.startsWith("SET_NAME###")) {
                clientNames[num] = msg.substring(12); // Store the name.
            } else if (msg.startsWith("MSG###")) {
                String actualMessage = clientNames[num] + "###" + msg.substring(6);

                for (int i = maxMessages - 1; i > 0; i--) {
                    messages[i] = messages[i - 1];
                }
                messages[0] = actualMessage;

                for (int i = 0; i < maxMessages; i++) {
                    if (messages[i].length() > 0) {
                        webSocket.broadcastTXT(messages[i]);
                    }
                }
            }
            break;
        }
        case WStype_CONNECTED: {
            clientNames[num] = "Client" + String(num); // Default name.
            for (int i = 0; i < maxMessages; i++) {
                if (messages[i].length() > 0) {
                    webSocket.sendTXT(num, messages[i]);
                }
            }
            break;
        }
        case WStype_DISCONNECTED: {
            clientNames[num] = "";
            break;
        }
        // ... [Handle other event types as needed]
    }
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP("ChatRoom");

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
  server.onNotFound([]() {
    server.send(200, "text/html", page);
  });
  
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  webSocket.loop();
}
