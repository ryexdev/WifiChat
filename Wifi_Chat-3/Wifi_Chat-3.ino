#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>


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

String input(String argName) {
  String a = webServer.arg(argName);
  a.replace("<", "&lt;");
  a.replace(">", "&gt;");
  a.substring(0, 200); 
  return a;
}

String posted() {
    String msg = input("m");
    String user = input("u");  // Get username from POST data
    allMsgs = "<li>" + user + ": " + msg + "</li>" + allMsgs;  // Attach username to message
    emit("posted: " + user + ": " + msg); 
    return user + ": " + msg;
}

void setup() {
  Serial.begin(115200); 
  emit("setup"); 
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(APIP, APIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(CHATNAME);
  
  dnsServer.start(DNS_PORT, "*", APIP);
  
  webServer.on("/post", []() {
    webServer.send(HTTP_CODE, "text/plain", posted());
  });
  
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
              const usernameInput = document.getElementById('username');
              const storedUsername = localStorage.getItem('username');

              if (storedUsername) {
                  usernameInput.value = storedUsername;
              }

              usernameInput.addEventListener('input', function(e) {
                  localStorage.setItem('username', e.target.value);
              });

              document.querySelector('form').addEventListener('submit', function(e) {
                e.preventDefault();

                const username = document.getElementById('username').value;
                const formData = new FormData(this);
                formData.append('u', username);

                fetch('/post', {
                  method: 'POST',
                  body: formData
                })
                .then(response => {
                  if (!response.ok) {
                    throw new Error('Network response was not ok' + response.statusText);
                  }
                  return response.text();
                })
                .then(text => {
                  document.querySelector('ol').innerHTML = '<li>' + text + '</li>' + document.querySelector('ol').innerHTML;
                  document.querySelector('textarea').value = '';
                })
                .catch(error => {
                  console.error('Fetch error: ', error);
                });
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
              <textarea name="m"></textarea>
              <br/>
              <input type="submit" value="send">
            </form>
          </div>
        </body>
      </html>
    )=====";
    webServer.send(HTTP_CODE, "text/html", page);
  });


  webServer.begin();
}

void loop() { 
  dnsServer.processNextRequest(); 
  webServer.handleClient(); 
}
