import network
import socket
import json
import esp32
import mdns

# Configure ESP32 as an Access Point
ssid = 'Navigate to esp32.local'

ap = network.WLAN(network.AP_IF)
ap.active(True)
ap.config(essid=ssid, authmode=network.AUTH_OPEN)

messages = []  # Store messages along with usernames

# Initialize mDNS
mdns = network.mDNS()
mdns.start('esp32', 'ESP32 Web Server')
mdns.addService('_http', '_tcp', 80)

# Start the Web Server
def start_server():
    addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1]
    s = socket.socket()
    s.bind(addr)
    s.listen(5)
    print('Listening on', addr)
    
    while True:
        cl, addr = s.accept()
        client_ip = str(addr[0])  # Extract client IP
        request = cl.recv(1024).decode('utf-8')
        
        if 'GET /messages' in request:
            cl.send('HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n')
            cl.sendall(json.dumps(messages))
        elif 'POST /message' in request:
            content_length = int(request.split('Content-Length: ')[1].split('\r\n')[0])
            post_body_raw = request.split('\r\n\r\n')[1][:content_length]
            post_body = json.loads(post_body_raw)
            message_content = post_body['message']
            messages.append(client_ip + ": " + message_content)
            cl.send('HTTP/1.1 200 OK\r\n\r\n')
        else:
            serve_chat_room_page(cl)
        
        cl.close()

def serve_chat_room_page(client):
    chat_room_html = """
<!DOCTYPE html>
<html>
<head>
<title>ESP32 Chat Room</title>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 20px; background-color: #f0f0f0; }
#chat { background-color: #e6e6e6; color: #333; border-radius: 8px; padding: 10px; width: 100%; max-width: 600px; height: 300px; overflow-y: auto; margin-bottom: 20px; }
input[type=text], button { border-radius: 5px; padding: 10px; font-size: 16px; }
input[type=text] { border: 1px solid #ccc; width: calc(100% - 120px); margin-right: 10px; }
button { background-color: #007bff; color: white; border: none; cursor: pointer; }
button:hover { background-color: #0056b3; }
@media (max-width: 600px) {
    body { padding: 15px; }
    input[type=text], button { width: 100%; margin-top: 5px; }
}
</style>
</head>
<body>
<h2>Temporary Chat</h2>
<div id="chat"></div>
<input type="text" id="message" placeholder="Type a message...">
<button onclick="sendMessage()">Send</button>
<script>
function updateScroll(){
    var element = document.getElementById("chat");
    element.scrollTop = element.scrollHeight;
}

setInterval(function() {
    fetch('/messages').then(response => response.json()).then(data => {
        var formattedMessages = data.map(function(message) {
            return '<p>' + message.replace(' - ', ': ') + '</p>';
        });
        document.getElementById('chat').innerHTML = formattedMessages.join('');
        updateScroll();
    });
}, 1000);

function sendMessage() {
    var message = document.getElementById('message').value;
    fetch('/message', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ message: message })
    }).then(() => {
        document.getElementById('message').value = ''; // Clear input field after sending
    });
}
</script>
</body>
</html>
"""
    client.send('HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n')
    client.sendall(chat_room_html)

start_server()
