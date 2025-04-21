#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Thông tin WiFi
const char* ssid = "KawaII";
const char* password = "dmcayvcl";

// Thông tin MQTT Broker
const char* mqtt_server = "172.20.10.5"; // Địa chỉ IP của Mosquitto broker
const int mqtt_port = 1883;
const char* mqtt_topic = "car/command";
const char* mqtt_client_id = "ESP32Car";

// Chân điều khiển L298N
int rightIN1 = 27;
int rightIN2 = 26;
int leftIN3 = 25;
int leftIN4 = 33;
int rightEnable = 14;
int leftEnable = 12;

// PWM properties
const int freq = 1000;
const int pwmChannelRight = 0;
const int pwmChannelLeft = 1;
const int resolution = 8;
int dutyCycle = 200; // Tốc độ mặc định (0-255)
int timeDelay = 400;

// Khai báo client WiFi và MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Hàm điều khiển xe
void stop() {
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, LOW);
  digitalWrite(leftIN3, LOW);
  digitalWrite(leftIN4, LOW);
  Serial.println("⛔ Đã dừng");
}

void moveForward() {
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, HIGH);
  digitalWrite(leftIN3, LOW);
  digitalWrite(leftIN4, HIGH);
  ledcWrite(pwmChannelRight, dutyCycle);
  ledcWrite(pwmChannelLeft, dutyCycle);
  Serial.println("➡️ Di chuyển tiến");
  // delay(timeDelay);
  // stop();
}

void moveBackward() {
  digitalWrite(rightIN1, HIGH);
  digitalWrite(rightIN2, LOW);
  digitalWrite(leftIN3, HIGH);
  digitalWrite(leftIN4, LOW);
  ledcWrite(pwmChannelRight, dutyCycle);
  ledcWrite(pwmChannelLeft, dutyCycle);
  Serial.println("⬅️ Di chuyển lùi");
  // delay(timeDelay);
  // stop();
}

void turnLeft() {
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, HIGH);
  digitalWrite(leftIN3, HIGH);
  digitalWrite(leftIN4, LOW);
  ledcWrite(pwmChannelRight, dutyCycle);
  ledcWrite(pwmChannelLeft, dutyCycle);
  Serial.println("↪️ Rẽ trái");
  // delay(timeDelay);
  // stop();
}

void turnRight() {
  digitalWrite(rightIN1, HIGH);
  digitalWrite(rightIN2, LOW);
  digitalWrite(leftIN3, LOW);
  digitalWrite(leftIN4, HIGH);
  ledcWrite(pwmChannelRight, dutyCycle);
  ledcWrite(pwmChannelLeft, dutyCycle);
  Serial.println("↩️ Rẽ phải");
  // delay(timeDelay);
  // stop();
}

void moveLeft() {
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, HIGH);
  digitalWrite(leftIN3, LOW);
  digitalWrite(leftIN4, HIGH);
  ledcWrite(pwmChannelRight, dutyCycle);
  ledcWrite(pwmChannelLeft, dutyCycle / 2);
  Serial.println("⬅️ Dịch chuyển sang trái");
  // delay(timeDelay);
  // stop();
}

void moveRight() {
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, HIGH);
  digitalWrite(leftIN3, LOW);
  digitalWrite(leftIN4, HIGH);
  ledcWrite(pwmChannelRight, dutyCycle / 2);
  ledcWrite(pwmChannelLeft, dutyCycle);
  Serial.println("➡️ Dịch chuyển sang phải");
  // delay(timeDelay);
  // stop();
}

void setSpeed(int speed) {
  if (speed >= 0 && speed <= 255) {
    dutyCycle = speed;
    ledcWrite(pwmChannelRight, dutyCycle);
    ledcWrite(pwmChannelLeft, dutyCycle);
    Serial.print("⚙️ Đã đặt tốc độ: ");
    Serial.println(dutyCycle);
  }
}

// Kết nối WiFi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Đang kết nối với WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi đã kết nối");
  Serial.println("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}

// Callback khi nhận được thông điệp MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Đã nhận thông điệp từ: ");
  Serial.println(topic);
  
  // Chuyển payload thành chuỗi
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  
  Serial.print("Thông điệp: ");
  Serial.println(message);

  // Xử lý lệnh
  if (strcmp(message, "F") == 0 || strcmp(message, "f") == 0) {
    moveForward();
  } else if (strcmp(message, "B") == 0 || strcmp(message, "b") == 0) {
    moveBackward();
  } else if (strcmp(message, "L") == 0 || strcmp(message, "l") == 0) {
    turnLeft();
  } else if (strcmp(message, "R") == 0 || strcmp(message, "r") == 0) {
    turnRight();
  } else if (strcmp(message, "Q") == 0 || strcmp(message, "q") == 0) {
    moveLeft();
  } else if (strcmp(message, "E") == 0 || strcmp(message, "e") == 0) {
    moveRight();
  } else if (strcmp(message, "S") == 0 || strcmp(message, "s") == 0) {
    stop();
  } else if (message[0] >= '1' && message[0] <= '9') {
    int speedLevel = (message[0] - '0') * 28; // Tốc độ tương ứng 1~252
    setSpeed(speedLevel);
  }
}

// Kết nối lại MQTT
void reconnect() {
  // Lặp lại cho đến khi kết nối được
  while (!client.connected()) {
    Serial.print("Đang kết nối MQTT...");
    // Thử kết nối
    if (client.connect(mqtt_client_id)) {
      Serial.println("đã kết nối");
      // Đăng ký nhận thông điệp từ topic
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("lỗi, rc=");
      Serial.print(client.state());
      Serial.println(" thử lại sau 5 giây");
      // Đợi 5 giây trước khi thử lại
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Cài đặt chân pin
  pinMode(rightIN1, OUTPUT);
  pinMode(rightIN2, OUTPUT);
  pinMode(leftIN3, OUTPUT);
  pinMode(leftIN4, OUTPUT);
  pinMode(rightEnable, OUTPUT);
  pinMode(leftEnable, OUTPUT);

  // Cấu hình PWM
  pinMode(rightEnable, OUTPUT);
  analogWrite(rightEnable, dutyCycle);
  pinMode(leftEnable, OUTPUT);
  analogWrite(leftEnable, dutyCycle);

  // Dừng xe khi khởi động
  stop();
  
  // Kết nối WiFi
  setup_wifi();
  
  // Cài đặt MQTT server và callback
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("\n🚗 Xe Điều Khiển Bằng MQTT và Hand Detection");
  Serial.println("Đang lắng nghe lệnh từ MQTT topic: " + String(mqtt_topic));
}

void loop() {
  // Kiểm tra và duy trì kết nối MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Vẫn giữ lại chức năng điều khiển qua Serial để kiểm tra
  if (Serial.available() > 0) {
    char command = Serial.read();

    // Loại bỏ ký tự không mong muốn
    if (command == '\n' || command == '\r') return;

    if (command >= '1' && command <= '9') {
      int speedLevel = (command - '0') * 28; // Tốc độ tương ứng 1~252
      setSpeed(speedLevel);
    } else {
      switch (tolower(command)) {
        case 'f': moveForward(); break;
        case 'b': moveBackward(); break;
        case 'l': turnLeft(); break;
        case 'r': turnRight(); break;
        case 'q': moveLeft(); break;
        case 'e': moveRight(); break;
        case 's': stop(); break;
        case 'h': 
          // In menu
          Serial.println("===== MENU ĐIỀU KHIỂN XE =====");
          Serial.println("F: Tiến | B: Lùi | L: Trái | R: Phải");
          Serial.println("Q: Dịch trái | E: Dịch phải | S: Dừng");
          Serial.println("1-9: Điều chỉnh tốc độ (1: chậm nhất, 9: nhanh nhất)");
          Serial.println("===============================");
          break;
        default:
          Serial.print("❓ Lệnh không hợp lệ: ");
          Serial.println(command);
          break;
      }
    }
  }

  delay(10);
}