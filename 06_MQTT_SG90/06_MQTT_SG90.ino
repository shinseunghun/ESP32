#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>

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

// ====== 서보모터 설정 ======
Servo myServo;
const int SERVO_PIN = 13; // SG90 신호선 연결 핀

// ====== MQTT 토픽 ======
const char* topic_sub = "servo/angle";  // 명령 수신 토픽

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

  if (String(topic) == "servo/angle") {
    int angle = msg.toInt();  // 문자열 → 정수 변환
    angle = constrain(angle, 0, 180);  // 각도 제한
    Serial.printf("서보 각도 설정: %d°\n", angle);

    myServo.write(angle);
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
  pinMode(ledPin, OUTPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(90); // 초기 위치 (중앙)

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
