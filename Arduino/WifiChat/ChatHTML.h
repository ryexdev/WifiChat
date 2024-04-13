String chatHTML = R"rawliteral(

<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=0">
  <title>Chat Room</title>
  <style>
    body, html {
      height: 100%;
      margin: 0;
      display: flex;
      flex-direction: column; /* Change to column for stacking */
      justify-content: center;
      align-items: center;
      background-color: #f0f0f0;
      font-family: Arial, sans-serif;
      font-size: 14px;
    }
    #intro {
      text-align: center; /* Center the intro text */
      margin-bottom: 20px; /* Spacing between intro and chat box */
      width: 90%;
    }
    #intro h2 {
      margin: 0; /* Remove default margins for consistency */
      font-size: 20px; /* Slightly larger for the title */
    }
    #intro p {
      margin: 5px 0 0; /* Small space above, none below */
      font-size: 14px; /* Keep the explanation text modest */
    }
    #chatBox {
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      width: 90%;
      max-width: 600px;
      height: 70vh;
      max-height: 500px;
      background-color: #fff;
      box-shadow: 0 4px 8px rgba(0,0,0,0.2);
      border-radius: 10px;
      padding: 20px 30px
      overflow: hidden;
    }
    #messages {
      height: calc(100% - 60px);
      overflow-y: auto;
      padding: 10px;
      border-bottom: 1px solid #ccc;
      margin-bottom: 20px;
      word-wrap: break-word;
    }
    #newMessage {
      width: 100%;
      display: flex;
    }
    #messageInput {
      flex-grow: 1;
      padding: 10px;
      margin-right: 10px;
      border: 1px solid #ccc;
      border-radius: 5px;
      font-size: 16px;
    }
    #sendButton {
      padding: 10px 15px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 5px;
      cursor: pointer;
    }
    #sendButton:hover {
      background-color: #45a049;
    }
  </style>
</head>
<body>
  <div id="intro">
    <h2>Chat Room</h2>
    <p>This is a pop-up chat room. It's hosted on someone's ESP32, so messages are not stored and will disappear when the device is reset. Have fun!</p>
  </div>
  <div id="chatBox">
    <div id="messages"></div>
    <form id="newMessage" action="#" onsubmit="submitMessage(); return false;">
      <input type="text" id="messageInput" placeholder="Type a message...">
      <button type="submit" id="sendButton">Send</button>
    </form>
  </div>
  <script>
    function submitMessage() {
      var message = document.getElementById("messageInput").value;

      // Check if the message is too long or empty
      if (message.length === 0) {
        alert("Please type a message.");
        return false;
      } else if (message.length > 255) {
        alert("Message too long. Please limit to 255 characters.");
        return false;
      }

      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          updateMessages(); // Call to update messages immediately after sending
        }
      };
      xhr.open("GET", "/send?msg=" + encodeURIComponent(message), true);
      xhr.send();
      document.getElementById("messageInput").value = ''; // Clear input after sending
    }

    function updateMessages() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var messagesDiv = document.getElementById("messages");
          messagesDiv.innerHTML = this.responseText.replace(/\n/g, "<br>");
          // Scroll to the bottom of the messages div
          messagesDiv.scrollTop = messagesDiv.scrollHeight;
        }
      };
      xhr.open("GET", "/chat", true);
      xhr.send();
    }

    setInterval(updateMessages, 500); // Update messages every second
  </script>
</body>
</html>

)rawliteral";
