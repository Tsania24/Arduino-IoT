#include <esp_now.h>
#include <WiFi.h>
#include <WebServer.h>

#define BUTTON_PIN 13  // Pin yang digunakan untuk tombol

int ledPin = 2;
int count = 0;
bool pumpState = false;
volatile bool buttonPressed = false;  // Variabel untuk menyimpan status tombol

uint8_t receiverMacAddress[] = {0xCC, 0x8D, 0xA2, 0x0C, 0xD9, 0x28}; // Replace with receiver's MAC address

String htmlON = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Interactive Switch Button - On</title>
    <style>
    body {
        background: rgb(7, 22, 60);
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        height: 100vh;
        overflow: hidden;
        color: #fff;
        text-align: center;
    }

    .background-text {
        font-size: 2.5rem;
        font-weight: bold;
        opacity: 0.5;
        z-index: -1;
        pointer-events: none;
        width: 90%;
        max-width: 800px;
        margin-bottom: 10px;
    }
 
    .sub-text {
        font-size: 1.2rem;
        opacity: 0.7;
        z-index: 1;
        pointer-events: none;
        width: 90%;
        max-width: 800px;
        margin-bottom: 30px;
    }
    
    .form-box {
        position: relative;
        margin-top: 50px;
    }
  
    .button-box {
        width: 220px;
        position: relative;
        border-radius: 30px;
        background: #fff;
        box-shadow: 0 0 15px rgba(0, 0, 0, 0.3);
        display: flex;
        justify-content: space-between;
    }
    
    .toggle-label {
        padding: 10px 40px;
        cursor: pointer;
        background: transparent;
        border: 0;
        outline: none;
        position: relative;
        text-align: center;
        z-index: 1;
        font-size: 1rem;
        color: #000;
    }
    
    #btn {
        position: absolute;
        top: 0;
        left: 0;
        width: 110px;
        height: 100%;
        background: #e6bc22;
        border-radius: 30px;
        transition: .5s;
        z-index: 0;
    }
    
    .description {
        margin-top: 30px;
        font-size: 1.5rem;
        opacity: 0.8;
    }
    </style>
</head>
<body>
    <div class="background-text">IoT Based Automatic Pressure Control for Biogas System</div>
    <div class="sub-text">(IoT-Based Automatic Pressure Control for Biogas System)</div>

    <div class="form-box">
        <div class="button-box">
            <div id="btn"></div>
            <label class="toggle-label" onclick="location.href='/off'">Off</label>
            <label class="toggle-label" onclick="location.href='/on'">On</label>
        </div>
    </div>
    
    <div class="description">Biogas Pump On.</div>
 </body>
 </html>
)rawliteral";
    
String htmlOFF = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Interactive Switch Button - Off</title>
    <style>
    body {
        background: rgb(7, 22, 60);
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 0;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        height: 100vh;
        overflow: hidden;
        color: #fff;
        text-align: center;
    }
 
    .background-text {
        font-size: 2.5rem;
        font-weight: bold;
        opacity: 0.5;
        z-index: -1;
        pointer-events: none;
        width: 90%;
        max-width: 800px;
        margin-bottom: 10px;
    }
    
    .sub-text {
        font-size: 1.2rem;
        opacity: 0.7;
        z-index: 1;
        pointer-events: none;
        width: 90%;
        max-width: 800px;
        margin-bottom: 30px;
    }
    
    .form-box {
        position: relative;
        margin-top: 50px;
    }
    
    .button-box {
        width: 220px;
        position: relative;
        border-radius: 30px;
        background: #fff;
        box-shadow: 0 0 15px rgba(0, 0, 0, 0.3);
        display: flex;
        justify-content: space-between;
    }
    
    .toggle-label {
        padding: 10px 40px;
        cursor: pointer;
        background: transparent;
        border: 0;
        outline: none;
        position: relative;
        text-align: center;
        z-index: 1;
        font-size: 1rem;
        color: #000;
    }
    
    #btn {
        position: absolute;
        top: 0;
        left: 0;
        width: 110px;
        height: 100%;
        background: #e6bc22;
        border-radius: 30px;
        transition: .5s;
        z-index: 0;
    }
    
    .description {
        margin-top: 30px;
        font-size: 1.5rem;
        opacity: 0.8;
    }
    </style>
</head>
<body>
    <div class="background-text">IoT Based Automatic Pressure Control for Biogas System</div>
    <div class="sub-text">(IoT-Based Automatic Pressure Control for Biogas System)</div>
 
    <div class="form-box">
        <div class="button-box">
            <div id="btn"></div>
            <label class="toggle-label" onclick="location.href='/off'">Off</label>
            <label class="toggle-label" onclick="location.href='/on'">On</label>
        </div>
    </div>
    
    <div class="description">Biogas Pump Off.</div>
</body>
</html>
)rawliteral";
    
// Create a WebServer object that listens on port 80
WebServer server(80);
 
void sendData(const char *message){
    esp_err_t result = esp_now_send(receiverMacAddress, (uint8_t *)message, strlen(message));

    if (result == ESP_OK) {
      Serial.println("Message sent successfully");
    } else {
      Serial.println("Error sending the message");
    }
}  

// Function to handle the root URL "/"
void handleRoot() {
    server.send(200, "text/html", htmlOFF);
}

// Function to handle the "On" button press
void handleOn() {
    /*pumpState = true; // Set pump state to on
    Serial2.write(pumpState ? '1' : '0'); // Use updated port*/
    count = 1;
    Serial.println("Pump state set to ON");
    server.send(200, "text/html", htmlON);
}
    
// Function to handle the "Off" button press
void handleOff() {
    /*pumpState = false; // Set pump state to off
    Serial2.write(pumpState ? '1' : '0'); // Use updated port*/
    count = 0;
    Serial.println("Pump state set to OFF");
    server.send(200, "text/html", htmlOFF);
}

//Function Switch
void IRAM_ATTR handleButtonPress() {
  buttonPressed = true;  // Set flag ketika interrupt terjadi
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT); // Set the GPIO pin as an output
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Menggunakan internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, CHANGE);  // Mengatur interrupt pada pin, dan trigger saat tombol ditekan
  // Setup Wi-Fi in Access Point mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ESP32-AP", "12345678");
  Serial.println("WiFi Access Point Started");
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");

    return;
  }

Serial.println(WiFi.softAPIP());

  // Add peer (receiver)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverMacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
    // Define the root URL handler
    server.on("/", handleRoot);
  
    // Define the handlers for the buttons
    server.on("/on", handleOn);
    server.on("/off", handleOff);
   
    // Start the web server
    server.begin();
    Serial.println("Web server started");
}

void loop() {
  // Continuously handle client requests
  server.handleClient();
    if (buttonPressed) {
    Serial.println(count++);
    buttonPressed = false;  // Reset flag setelah interrupt diproses
    
  }
  if ((count%2)==1){
      pumpState = false;   
      Serial.println("Nyala");
      sendData("1");
      digitalWrite(ledPin, HIGH); // Turn the LED on
    }else{
      count = 0;
      pumpState = true;
      Serial.println("Mati");
      sendData("0");
      digitalWrite(ledPin, LOW);
    }
    Serial.println("count");
    Serial.println(count);
  delay(1000);
}