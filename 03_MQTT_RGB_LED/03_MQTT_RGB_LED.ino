#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Arduino.h>
#include "driver/ledc.h"

// ===== WiFi 정보 =====
const char* ssid = "SSID";
const char* password = "PASSWORD";

// ===== HiveMQ Cloud 정보 =====
const char* mqtt_server = "445bb19566794dd1b1f1ad41488a1838.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "dohasiha";
const char* mqtt_password = "DOHAsiha00!";

// ===== LED 핀 =====
const int R_LED_Pin = 16;
const int G_LED_Pin = 4;
const int B_LED_Pin = 17;

const int Freq_LED = 5000;
const int PWM_Channel_0 = 0; 
const int PWM_Channel_1 = 1; 
const int PWM_Channel_2 = 2; 
const int Resolution_LED = 8;

unsigned int R_LED_Duty = 0; // 8 bits
unsigned int G_LED_Duty = 0; // 8 bits
unsigned int B_LED_Duty = 0; // 8 bits

const char* topic_sub = "led/rgb";

// TLS 클라이언트
WiFiClientSecure espClient;
PubSubClient client(espClient);

// 콜백 함수: 메시지를 수신할 때 실행
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("메시지 수신 [");
  Serial.print(topic);
  Serial.print("] ");

  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  Serial.println(msg);

  // 메시지 형식: "R,G,B" (예: "255,100,50")
  int r, g, b;
  if (sscanf(msg.c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
    Serial.printf("설정된 RGB: R=%d, G=%d, B=%d\n", r, g, b);

    // 값 범위 제한 (0~255)
    R_LED_Duty = constrain(r, 0, 255);
    G_LED_Duty = constrain(g, 0, 255);
    B_LED_Duty = constrain(b, 0, 255);

    Serial.printf("출력할 RGB: R=%d, G=%d, B=%d\n", R_LED_Duty, G_LED_Duty, B_LED_Duty);

    // LED 출력 (PWM 제어)
    ledcWrite(R_LED_Pin, R_LED_Duty); // R_LED, Values are PWM_Channel & Duty
    ledcWrite(G_LED_Pin, G_LED_Duty); // G_LED, Values are PWM_Channel & Duty
    ledcWrite(B_LED_Pin, B_LED_Duty); // B_LED, Values are PWM_Channel & Duty


  } else {
    Serial.println("잘못된 메시지 형식입니다. 형식: R,G,B (예: 255,0,128)");
  }
}

// MQTT 연결 함수
void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT 연결 중...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("연결 성공");
      client.subscribe(topic_sub);
    } else {
      Serial.print("실패, rc=");
      Serial.print(client.state());
      Serial.println(" 5초 후 재시도");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  // LED 핀 설정
    
  ledcAttachChannel(R_LED_Pin, Freq_LED, Resolution_LED, PWM_Channel_0); // R_LED, Values are Pin & PWM_Channel
  ledcAttachChannel(G_LED_Pin, Freq_LED, Resolution_LED, PWM_Channel_1); // G_LED, Values are Pin & PWM_Channel
  ledcAttachChannel(B_LED_Pin, Freq_LED, Resolution_LED, PWM_Channel_2); // B_LED, Values are Pin & PWM_Channel

  ledcWrite(R_LED_Pin, 0); // R_LED, Values are PWM_Channel & Duty
  ledcWrite(G_LED_Pin, 0); // G_LED, Values are PWM_Channel & Duty
  ledcWrite(B_LED_Pin, 0); // B_LED, Values are PWM_Channel & Duty

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi 연결 중...");
  }
  Serial.println("WiFi 연결 완료");

  // TLS 인증서 검증 생략 (필요 시 인증서 직접 설정 가능)
  espClient.setInsecure();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
