#include <Arduino.h>

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
  delay(timeDelay);
  stop();
  
}

void moveBackward() {
  digitalWrite(rightIN1, HIGH);
  digitalWrite(rightIN2, LOW);
  digitalWrite(leftIN3, HIGH);
  digitalWrite(leftIN4, LOW);
  ledcWrite(pwmChannelRight, dutyCycle);
  ledcWrite(pwmChannelLeft, dutyCycle);
  Serial.println("⬅️ Di chuyển lùi");
  delay(timeDelay);
  stop();
}

void turnLeft() {
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, HIGH);
  digitalWrite(leftIN3, HIGH);
  digitalWrite(leftIN4, LOW);
  ledcWrite(pwmChannelRight, dutyCycle);
  ledcWrite(pwmChannelLeft, dutyCycle);
  Serial.println("↪️ Rẽ trái");
  delay(timeDelay);
  stop();
}

void turnRight() {
  digitalWrite(rightIN1, HIGH);
  digitalWrite(rightIN2, LOW);
  digitalWrite(leftIN3, LOW);
  digitalWrite(leftIN4, HIGH);
  ledcWrite(pwmChannelRight, dutyCycle);
  ledcWrite(pwmChannelLeft, dutyCycle);
  Serial.println("↩️ Rẽ phải");
  delay(timeDelay);
  stop();
}

void moveLeft() {
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, HIGH);
  digitalWrite(leftIN3, LOW);
  digitalWrite(leftIN4, HIGH);
  ledcWrite(pwmChannelRight, dutyCycle);
  ledcWrite(pwmChannelLeft, dutyCycle / 2);
  Serial.println("⬅️ Dịch chuyển sang trái");
  delay(timeDelay);
  stop();
}

void moveRight() {
  digitalWrite(rightIN1, LOW);
  digitalWrite(rightIN2, HIGH);
  digitalWrite(leftIN3, LOW);
  digitalWrite(leftIN4, HIGH);
  ledcWrite(pwmChannelRight, dutyCycle / 2);
  ledcWrite(pwmChannelLeft, dutyCycle);
  Serial.println("➡️ Dịch chuyển sang phải");
  delay(timeDelay);
  stop();
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

void printMenu() {
  Serial.println("===== MENU ĐIỀU KHIỂN XE =====");
  Serial.println("F: Tiến | B: Lùi | L: Trái | R: Phải");
  Serial.println("Q: Dịch trái | E: Dịch phải | S: Dừng");
  Serial.println("1-9: Điều chỉnh tốc độ (1: chậm nhất, 9: nhanh nhất)");
  Serial.println("H: Hiển thị menu");
  Serial.println("===============================");
}

void setup() {
  Serial.begin(9600);

  pinMode(rightIN1, OUTPUT);
  pinMode(rightIN2, OUTPUT);
  pinMode(leftIN3, OUTPUT);
  pinMode(leftIN4, OUTPUT);
  pinMode(rightEnable, OUTPUT);
  pinMode(leftEnable, OUTPUT);

  ledcSetup(pwmChannelRight, freq, resolution);
  ledcAttachPin(rightEnable, pwmChannelRight);
  ledcSetup(pwmChannelLeft, freq, resolution);
  ledcAttachPin(leftEnable, pwmChannelLeft);

  stop();
  Serial.println("\n🚗 Xe Điều Khiển Bằng Serial");
  Serial.println("Sử dụng Serial Monitor (9600 Baud)");
  printMenu();
}

void loop() {
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
        case 'h': printMenu(); break;
        default:
          Serial.print("❓ Lệnh không hợp lệ: ");
          Serial.println(command);
          break;
      }
    }
  }

  delay(10);
}