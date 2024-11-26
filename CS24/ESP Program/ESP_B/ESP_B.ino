#include <esp_now.h>
#include <WiFi.h>

const int relayPin = 15;  // GPIO pin connected to the SSR
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  Serial.print("Received from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", recv_info->src_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  Serial.print("Bytes received: ");
  Serial.println(len);
  char incomingMessage[len + 1];
  memcpy(incomingMessage, incomingData, len);
  incomingMessage[len] = '\0';
  int Value = atoi(incomingMessage);
  Serial.println("Value");
  Serial.println(Value);

  if(Value == 1){
  // Turn the SSR on
  digitalWrite(relayPin, HIGH);
  Serial.println("pompaNyala");
  delay(1000);  // Wait for 3 seconds
  }else{
  // Turn the SSR off
  digitalWrite(relayPin, LOW);
  Serial.println("pompaMati");
  delay(1000);  // Wait for 3 seconds
  }
}

void setup() {
  Serial.begin(115200);

  // Connect to the AP created by Device 1
  WiFi.begin("ESP32-AP", "12345678");
  // Initialize the relay pin as an output
  pinMode(relayPin, OUTPUT);

  // Start with the relay turned off
  digitalWrite(relayPin, LOW);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register receive callback
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // The loop can be used for other tasks; ESP-NOW handling is done via the callback
}