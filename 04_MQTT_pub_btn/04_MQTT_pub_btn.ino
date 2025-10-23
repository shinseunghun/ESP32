#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// ===== WiFi 정보 =====
const char* ssid = "SSID";
const char* password = "PASSWORD";

// ===== HiveMQ Cloud 정보 =====
const char* mqtt_server = "445bb19566794dd1b1f1ad41488a1838.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "dohasiha";
const char* mqtt_password = "DOHAsiha00!";

// ===== LED 핀 =====
const int ledPin = 2;

// ===== 버튼 핀 설정 =====
const int BUTTON_PIN = 15;

// ===== MQTT 토픽 =====
const char* topic_pub = "button/state";

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

  if (msg == "on") {
    digitalWrite(ledPin, HIGH);
  } else if (msg == "off") {
    digitalWrite(ledPin, LOW);
  }
  else if (msg == "read") {    
    int sensorValue = digitalRead(ledPin);
    Serial.print("LED value: ");
    Serial.println(sensorValue);

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", sensorValue);
    client.publish("esp32/led/value", buffer);
  }
}

// MQTT 연결 함수
void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT 연결 중...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("연결 성공");
      client.subscribe("esp32/led");
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
  pinMode(ledPin, OUTPUT);
  pinMode(BUTTON_PIN, INPUT); // 버튼 핀 입력

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

  // 버튼 상태 읽기
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == HIGH) {
    Serial.println("버튼 눌림! 메시지 전송");
    client.publish(topic_pub, "pressed");
    delay(500); // 중복 발행 방지
  }
}
