#include <Arduino.h>

// Định nghĩa chân encoder
const int ENCODER_PIN = 18; // Chân OUT của encoder H206 kết nối với GPIO18 của ESP32

// Thông số encoder
const int PULSES_PER_ROTATION = 20; // Số khe (xung) trong một vòng quay

// Biến lưu số đếm
volatile long pulseCount = 0;
volatile long lastPulseCount = 0;
volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 5; // Thời gian debounce (ms)

// Biến cho tính toán tốc độ và vòng quay
unsigned long previousMillis = 0;
const long interval = 1000; // Thời gian cập nhật (ms)
float totalRotations = 0.0; // Tổng số vòng quay
float lastRotations = 0.0;  // Số vòng quay tại lần cập nhật trước

// Hàm ngắt xử lý khi có xung từ encoder
void IRAM_ATTR handlePulse() {
  unsigned long currentTime = millis();
  // Kiểm tra thời gian debounce để tránh đếm nhiễu
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    pulseCount++;
    lastDebounceTime = currentTime;
  }
}

void setup() {
  // Khởi tạo Serial để hiển thị kết quả
  Serial.begin(9600);
  Serial.println("ESP32 Encoder Rotation Counter");
  Serial.println("Encoder: 20 khe/vong");
  
  // Thiết lập chân encoder là INPUT_PULLUP để đảm bảo trạng thái xác định khi không có tín hiệu
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  
  // Đính kèm ngắt vào chân encoder, kích hoạt khi có cạnh lên (RISING)
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), handlePulse, RISING);
  
  Serial.println("Đang đếm vòng quay từ encoder...");
}

void loop() {
  // Kiểm tra xem đã đến lúc cập nhật số liệu chưa
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    // Lưu thời điểm cập nhật
    previousMillis = currentMillis;
    
    // Tính toán số xung mới trong khoảng thời gian
    long newPulses = pulseCount - lastPulseCount;
    lastPulseCount = pulseCount;
    
    // Tính số vòng quay
    totalRotations = (float)pulseCount / PULSES_PER_ROTATION;
    
    // Tính số vòng quay mới trong khoảng thời gian này
    float newRotations = totalRotations - lastRotations;
    lastRotations = totalRotations;
    
    // Tính RPM (vòng/phút)
    float rpm = newRotations * 60.0; // newRotations là số vòng/giây, nhân với 60 để có vòng/phút
    
    // Hiển thị thông tin
    Serial.print("Tong so xung: ");
    Serial.print(pulseCount);
    Serial.print(" | Xung/giay: ");
    Serial.print(newPulses);
    Serial.print(" | So vong: ");
    Serial.print(totalRotations, 2); // Hiển thị 2 chữ số thập phân
    Serial.print(" | RPM: ");
    Serial.println(rpm, 1);  // Hiển thị 1 chữ số thập phân
    
    // Nếu muốn tính toán thêm như quãng đường, tốc độ, v.v. có thể thêm vào đây
    // Ví dụ: distance = totalRotations * wheelCircumference
  }
}