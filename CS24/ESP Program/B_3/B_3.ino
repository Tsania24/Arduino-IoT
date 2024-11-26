#include <esp_now.h>
#include <WiFi.h>

#define LED_PIN 40 // LED 핀 번호

// 일정한 간격으로 메시지를 전송하기 위한 타이머 값
unsigned long t = 0;

// 송신할 데이터 (bool 값)
bool sendData1 = false; // ESP NOW를 통해 송수신되는 신호
bool sendData2 = false; // Serial을 통해 송수신되는 신호

// 상대방의 MAC 주소
byte peerMAC[] = {0xB8, 0xD6, 0x1A, 0xA7, 0x2F, 0xD0};
esp_now_peer_info_t dest;

// 송수신 성공 여부를 추적하는 변수
bool sendSuccess = false;
bool receiveSuccess = false;

// 송신 콜백 함수
/*void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("성공적으로 송신했음!");
    sendSuccess = true; // 송신 성공
  } else {
    Serial.println("송신 실패!");
    sendSuccess = false; // 송신 실패
  }
}

// 수신 콜백 함수
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len == sizeof(bool)) {
    bool receivedData = *(bool *)data;
    Serial.print("수신된 데이터: ");
    Serial.println(receivedData ? "true" : "false");
    receiveSuccess = true; // 수신 성공

    Serial2.write(sendData2 ? '1' : '0'); // 수정된 포트 사용
  } 
  else {
    Serial.println("수신 데이터 길이 오류!");
    receiveSuccess = false; // 수신 실패
  }
}*/

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // Serial2 포트 (TX=17, RX=16)
  pinMode(LED_PIN, OUTPUT);

  // Wi-Fi 모드를 스테이션 모드로 설정
  WiFi.mode(WIFI_STA);

  // ESP-NOW 기능을 활성화
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESPNOW 초기화 실패!");
    return;
  }

  // 상대방 보드 주소 설정
  memcpy(dest.peer_addr, peerMAC, sizeof(peerMAC));
  dest.channel = 1; // 현재 Wi-Fi 채널 사용, 두 장치가 동일 채널을 사용해야 합니다
  dest.encrypt = false;

  // 상대방 보드를 피어로 등록
  if (esp_now_add_peer(&dest) != ESP_OK) {
    Serial.println("피어 등록 실패!");
    return;
  }

  // 송신 완료 콜백 함수 등록
  esp_now_register_send_cb(OnDataSent);

  // 수신 콜백 함수 등록
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // 2초마다 데이터를 송신
  if (millis() - t > 2000) {
    t = millis();
    sendSuccess = false; // 송신 성공 여부 초기화
    esp_now_send(dest.peer_addr, (uint8_t *)&sendData1, sizeof(sendData1));
    Serial.println("송신 데이터: " + String(sendData1 ? "true" : "false"));
  }

  if (Serial2.available()) {
    char receivedChar = Serial2.read();
    bool receivedData = (receivedChar == '1');
    sendData1 = receivedData;
  }
  
  // 송수신이 모두 성공하면 LED 켜기
  if (sendSuccess && receiveSuccess) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW); 
  }
}


