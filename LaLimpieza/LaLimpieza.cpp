#include "Arduino.h"
#include "LaLimpieza.h"

AsyncWebServer server(80); // Creating WebServer at port 80
WebSocketsServer webSocket = WebSocketsServer(81); // Creating WebSocket Server at port81

//
// This is the webpage which is hosted on your ESP board 
// and you can access this webpage by going to this address
//               
//                      192.168.4.1
//

char _webpage[] PROGMEM = R"=====(
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LA LIMPIEZA PORTAL</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">

    <script>
        var connection = new WebSocket('ws://' + location.hostname + ':81/');
        connection.onopen = function () {
            connection.send('Connect ' + new Date());
        };
        connection.onerror = function (error) {
            console.log('WebSocket Error ', error);
        };
        connection.onmessage = function (e) {
            console.log('Server: ', e.data);
        };

        function togglePassword() {
            var passField = document.getElementById('pass_cred');
            var showPassIcon = document.getElementById('showPassIcon');
            if (passField.type === 'password') {
                passField.type = 'text';
                showPassIcon.classList.remove('fa-eye');
                showPassIcon.classList.add('fa-eye-slash');
            } else {
                passField.type = 'password';
                showPassIcon.classList.remove('fa-eye-slash');
                showPassIcon.classList.add('fa-eye');
            }
        }

        function credentials_rec() {
            var ssid = document.getElementById('ssid_cred').value;
            var pass = document.getElementById('pass_cred').value;
            var auth = document.getElementById('auth_token').value;
            var full_command = '#{"ssid":"' + ssid + '", "pass":"' + pass + '","auth":"' + auth + '"}';
            console.log(full_command);
            connection.send(full_command);
            location.replace('http://' + location.hostname + '/submit');
        }
    </script>

    <style>
        body {
          font-family: Arial, sans-serif;
          background: 
            radial-gradient(farthest-side at -33.33% 50%,#0000 52%,#2ff7e8 54% 57%,#0000 59%) 0 calc(128px/2),
            radial-gradient(farthest-side at 50% 133.33%,#0000 52%,#2ff7e8 54% 57%,#0000 59%) calc(128px/2) 0,
            radial-gradient(farthest-side at 133.33% 50%,#0000 52%,#2ff7e8 54% 57%,#0000 59%),
            radial-gradient(farthest-side at 50% -33.33%,#0000 52%,#2ff7e8 54% 57%,#0000 59%),
            #5de7d9;
          background-size: calc(128px/4.667) 128px,128px calc(128px/4.667);
          margin: 0;
          display: flex;
          justify-content: center;
          align-items: center;
          min-height: 100vh;
        }
    
        form {
          background-color: #fff;
          border-radius: 2px;
          padding: 20px;
          box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
          width: 80%;
          max-width: 400px;
          text-align: center;
        }
    
        label {
          display: block;
          font-weight: bold;
          margin-bottom: 10px;
          font-size: 16px;
        }
    
        input[type="text"] {
          width: calc(100% - 20px);
          padding: 10px;
          margin-bottom: 20px;
          border: 1px solid #ccc;
          border-radius: 4px;
          box-sizing: border-box;
          font-size: 16px;
          transition: border-color 0.3s ease;
        }
    
        input[type="text"]:focus {
          border-color:    #5de7d9;
          outline: none;
        }

        input[type="password"] {
            width: calc(100% - 20px);
            padding: 10px;
            margin-bottom: 20px;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 16px;
            transition: border-color 0.3s ease;
          }
      
          input[type="password"]:focus {
            border-color:    #5de7d9;
            outline: none;
          }
    
        button {
          background-color:    #5de7d9;
          color: white;
          padding: 10px;
          width: calc(100% - 20px);
          border: none;
          border-radius: 4px;
          cursor: pointer;
          font-size: 16px;
          transition: background-color 0.3s ease;
        }
    
        button:hover {
          background-color:    #41a59b;
        }

    /* Additional style for eye icon */
    .fa-eye {
        position: absolute;
        top: 50%;
        right: 10px;
        transform: translateY(-50%);
        cursor: pointer;
        padding-bottom: 20px;
        padding-right: 10px;
        color: #65cbc2;
    }
    .fa-eye-slash {
        position: absolute;
        top: 50%;
        right: 10px;
        transform: translateY(-50%);
        cursor: pointer;
        padding-bottom: 20px;
        padding-right: 10px;
        color: #65cbc2;
    }

</style>
</head>
<body>
    <form>
        <h2 style="margin-bottom: 20px; color: #5de7d9;">LA LIMPIEZA</h2>
        <input type="text" id="auth_token" placeholder="Enter Authentication Token">
        <input type="text" id="ssid_cred" placeholder="Enter WiFi Name">
        <div style="position: relative;">
            <input type="password" id="pass_cred" placeholder="Enter WiFi Password">
            <i id="showPassIcon" class="fas fa-eye fa-lg" onclick="togglePassword()"></i>
        </div>
        <button type="button" onclick="credentials_rec()">Submit</button>
    </form>

    <script src="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/js/all.min.js"></script>
</body>
</html>

)=====";





void _webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{

  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);

      if (payload[0] == '#') 
      {
        String message = String((char*)( payload));
        message = message.substring(1);
        Serial.println(message);

        //JSON part
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, message);

        String ssid = doc["ssid"];
        String pass = doc["pass"];
        String auth = doc["auth"];
        Serial.println(ssid); Serial.println(pass);


        // Clearing EEPROM
        if (ssid.length() > 0 && pass.length() > 0) {
          Serial.println("clearing eeprom");
          for (int i = 0; i < 100; ++i) {
            EEPROM.write(i, 0);
          }


          // Storing in EEPROM
          Serial.println("writing eeprom ssid:");
          for (int i = 0; i < ssid.length(); ++i)
          {
            EEPROM.write(i, ssid[i]);
            Serial.print("Wrote: ");
            Serial.println(ssid[i]);
          }
          Serial.println("writing eeprom pass:");
          for (int i = 0; i < pass.length(); ++i)
          {
            EEPROM.write(32 + i, pass[i]);
            Serial.print("Wrote: ");
            Serial.println(pass[i]);
          }
          Serial.println("writing eeprom auth token:");
          for (int i = 0; i < auth.length(); ++i)
          {
            EEPROM.write(64 + i, auth[i]);
            Serial.print("Wrote: ");
            Serial.println(auth[i]);
          }
          EEPROM.commit();
          delay(2000);

          //Restarting ESP board
          ESP.restart();
          break;
        }
     }
  }
}



void credentials::Erase_eeprom()
{
  EEPROM.begin(512); //Initialasing EEPROM
  Serial.println("Erasing...");
  Serial.println("clearing eeprom");
    for (int i = 0; i < 100; ++i) 
    {
      EEPROM.write(i, 0);
    }
   EEPROM.commit();
}



String credentials::EEPROM_Config()
{
 EEPROM.begin(512); //Initialasing EEPROM
  Serial.println();
  Serial.println();

  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM");


  for (int i = 0; i < 32; ++i)
  {
    ssid += char(EEPROM.read(i));
  }
  Serial.print("SSID: ");
  Serial.println(ssid);

  for (int i = 32; i < 64; ++i)
  {
    pass += char(EEPROM.read(i));
  }

  Serial.print("Password: ");
  Serial.println(pass);
  String auth_token = "";
  for (int i = 64; i < 100; ++i)
  {
    auth_token += char(EEPROM.read(i));
  }
  Serial.print("Auth Token: ");
  Serial.println(auth_token);
  return auth_token;
}



bool credentials::credentials_get()
{
  if (_testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    return true;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    return false;
  }
}



void credentials::setupAP(char* softap_ssid, char* softap_pass)
{

  WiFi.disconnect();
  delay(100);
  WiFi.softAP(softap_ssid,softap_pass);
  Serial.println("softap");
  _launchWeb();
  Serial.println("Server Started");
   webSocket.begin();
    webSocket.onEvent(_webSocketEvent);
}



bool credentials::_testWifi()
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  char* my_ssid = &ssid[0];
  char* my_pass = &pass[0];

  WiFi.begin(my_ssid, my_pass);
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}



// This is the function which will be called when an invalid page is called from client
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}


void credentials::_createWebServer()
{
server.on("/", [](AsyncWebServerRequest * request) {
  request->send_P(200, "text/html", _webpage);
});

// Send a GET request to <IP>/get?message=<message>
server.on("/submit", HTTP_GET, [] (AsyncWebServerRequest * request) {
  String message;
    message = "Credentials received by ESP board!!!";
   request->send(200, "text/plain", message);
});

server.onNotFound(notFound);
server.begin();

}

void credentials::_launchWeb()
{
  Serial.println("");
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  _createWebServer();
  // Start the server

}

void credentials::server_loops()
{ 
     webSocket.loop();
}
