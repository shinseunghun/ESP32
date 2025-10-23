#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

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

// ====== DHT22 설정 ======
#define DHTPIN 18       // DHT 데이터 핀
#define DHTTYPE DHT22  // 또는 DHT22 / AM2302
DHT dht(DHTPIN, DHTTYPE);

// ====== MQTT 토픽 ======
const char* topic_sub = "sensor/dht/read";
const char* topic_pub = "sensor/dht/value";

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

  if (msg == "read") {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("센서 읽기 오류!");
      client.publish(topic_pub, "error: failed to read from DHT sensor");
      return;
    }

    Serial.printf("온도: %.1f °C, 습도: %.1f %%\n", temperature, humidity);

    // JSON 형태로 데이터 전송
    char payload[100];
    snprintf(payload, sizeof(payload), "{\"temperature\":%.1f,\"humidity\":%.1f}", temperature, humidity);
    client.publish(topic_pub, payload);
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

  dht.begin();
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
