import cv2
import mediapipe as mp
import math
import paho.mqtt.client as mqtt
import time

# Cấu hình MQTT
MQTT_BROKER = "localhost"  # Địa chỉ IP của Mosquitto broker
MQTT_PORT = 1883
MQTT_TOPIC = "car/command"

# Khai báo biến toàn cục
new_state = -1
old_state = -1

# Khởi tạo MQTT client
client = mqtt.Client()
client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_start()

# Khởi tạo MediaPipe Hands
mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
hands = mp_hands.Hands(
    static_image_mode=False,
    max_num_hands=1,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5
)

# Cấu hình camera
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

# Cấu hình thời gian để kiểm tra độ ổn định
STABILITY_TIME = 0.1  # Thời gian cử chỉ cần ổn định (giây)
COMMAND_COOLDOWN = 0.05  # Thời gian giữa các lệnh
last_command_time = time.time() - COMMAND_COOLDOWN

# Biến theo dõi độ ổn định của cử chỉ
current_gesture = "NONE"
last_gesture = "NONE"
gesture_start_time = 0
stable_gesture = "NONE"
gesture_confirmed = False

def calculate_finger_angles(landmarks):
    # Tính góc giữa các khớp ngón tay
    # Trả về danh sách góc của 5 ngón tay
    angles = []
    
    # Ngón cái (Thumb)
    thumb_angle = calculate_angle(landmarks[0], landmarks[2], landmarks[4])
    angles.append(thumb_angle)
    
    # Ngón trỏ (Index)
    index_angle = calculate_angle(landmarks[0], landmarks[5], landmarks[8])
    angles.append(index_angle)
    
    # Ngón giữa (Middle)
    middle_angle = calculate_angle(landmarks[0], landmarks[9], landmarks[12])
    angles.append(middle_angle)
    
    # Ngón áp út (Ring)
    ring_angle = calculate_angle(landmarks[0], landmarks[13], landmarks[16])
    angles.append(ring_angle)
    
    # Ngón út (Pinky)
    pinky_angle = calculate_angle(landmarks[0], landmarks[17], landmarks[20])
    angles.append(pinky_angle)
    
    return angles

def calculate_angle(point1, point2, point3):
    # Tính góc giữa ba điểm
    x1, y1 = point1.x, point1.y
    x2, y2 = point2.x, point2.y
    x3, y3 = point3.x, point3.y
    
    angle = math.degrees(math.atan2(y3 - y2, x3 - x2) - math.atan2(y1 - y2, x1 - x2))
    angle = abs(angle)
    if angle > 180:
        angle = 360 - angle
    return angle

def detect_gesture(landmarks):
    global new_state, old_state
    
    # Tính góc các ngón tay
    angles = calculate_finger_angles(landmarks)
    
    # Kiểm tra các ngón tay có duỗi ra không (ngón tay được coi là duỗi nếu góc > 150 độ)
    extended_fingers = [1 if angle > 150 else 0 for angle in angles]
    
    # Đếm số ngón tay được duỗi ra
    num_extended = sum(extended_fingers)
    
    # Xác định cử chỉ dựa trên số ngón tay duỗi
    if num_extended == 0:  # Nắm tay - dừng
        old_state = new_state
        new_state = 0
        return "S"  # Stop
    
    elif num_extended == 1:  # 1 ngón - Tiến
        old_state = new_state
        new_state = 1
        return "F"  # Forward
    
    elif num_extended == 2:  # 2 ngón - Lùi
        old_state = new_state
        new_state = 2
        return "B"  # Backward
    
    elif num_extended == 3:  # 3 ngón - Rẽ trái
        old_state = new_state
        new_state = 3
        return "L"  # Left
    
    elif num_extended == 4:  # 4 ngón - Rẽ phải
        old_state = new_state
        new_state = 4
        return "R"  # Right
    
    elif num_extended == 5:  # 5 ngón - Tốc độ cao
        old_state = new_state
        new_state = 5
        return "9"  # Tốc độ cao
    
    return "NONE"

print("Đang khởi động hệ thống nhận dạng cử chỉ tay...")

try:
    while cap.isOpened():
        success, image = cap.read()
        if not success:
            print("khong the")
            break
        
        # Lật ảnh để dễ sử dụng
        image = cv2.flip(image, 1)
        
        # Chuyển đổi màu từ BGR sang RGB
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        
        # Xử lý ảnh với MediaPipe
        results = hands.process(image_rgb)
        
        # Tạo biến để hiển thị trạng thái ổn định
        stability_status = "dang cho cu chi"
        stability_color = (255, 0, 0)  # Màu đỏ khi chưa ổn định
        
        # Vẽ kết quả
        if results.multi_hand_landmarks:
            for hand_landmarks in results.multi_hand_landmarks:
                # Vẽ các điểm và đường kết nối
                mp_drawing.draw_landmarks(
                    image, hand_landmarks, mp_hands.HAND_CONNECTIONS)
                
                # Lấy danh sách các điểm trên bàn tay
                landmarks = [lm for lm in hand_landmarks.landmark]
                
                # Phát hiện cử chỉ
                detected_gesture = detect_gesture(landmarks)
                
                # Kiểm tra độ ổn định của cử chỉ
                current_time = time.time()
                if detected_gesture != current_gesture:
                    # Cử chỉ đã thay đổi, đặt lại thời gian bắt đầu
                    current_gesture = detected_gesture
                    gesture_start_time = current_time
                    gesture_confirmed = False
                    stability_status = f"da phat hien: {current_gesture}"
                elif not gesture_confirmed and (current_time - gesture_start_time) >= STABILITY_TIME:
                    # Cử chỉ đã ổn định đủ lâu
                    stable_gesture = current_gesture
                    gesture_confirmed = True
                    stability_status = f"da xac nhan: {stable_gesture}"
                    stability_color = (0, 255, 0)  # Chuyển sang màu xanh khi ổn định
                    
                    # Gửi lệnh qua MQTT nếu đã ổn định và khác với lệnh trước đó
                    if (stable_gesture != last_gesture or stable_gesture in ["F", "B", "L", "R"]) and \
                       stable_gesture != "NONE" and \
                       current_time - last_command_time > COMMAND_COOLDOWN:
                        print(f"Gửi lệnh: {stable_gesture}")
                        client.publish(MQTT_TOPIC, stable_gesture)
                        last_command_time = current_time
                        last_gesture = stable_gesture
                
                # Hiển thị thời gian ổn định
                if not gesture_confirmed and current_gesture != "NONE":
                    stability_progress = min(100, int((current_time - gesture_start_time) / STABILITY_TIME * 100))
                    cv2.rectangle(image, (10, 60), (10 + stability_progress * 2, 80), (0, 255, 0), -1)
                    cv2.rectangle(image, (10, 60), (210, 80), (255, 255, 255), 2)
                
                # Hiển thị cử chỉ hiện tại trên màn hình
                cv2.putText(image, f"Cử chỉ: {detected_gesture}", (10, 30),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                cv2.putText(image, stability_status, (10, 110),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, stability_color, 2)
        else:
            # Không phát hiện bàn tay, đặt lại các biến
            current_gesture = "NONE"
            gesture_confirmed = False
        
        # Hiển thị hướng dẫn
        cv2.putText(image, "0 ngon: Dung (S)", (10, image.shape[0] - 140), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
        cv2.putText(image, "1 ngon: Tien (F)", (10, image.shape[0] - 120), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
        cv2.putText(image, "2 ngon: Lui (B)", (10, image.shape[0] - 100), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
        cv2.putText(image, "3 ngon: Re trai (L)", (10, image.shape[0] - 80), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
        cv2.putText(image, "4 ngon: Re phai (R)", (10, image.shape[0] - 60), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
        cv2.putText(image, "5 ngon: Tang toc (9)", (10, image.shape[0] - 40), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
        cv2.putText(image, "Nhan 'q' de thoat", (10, image.shape[0] - 20), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
        
        # Hiển thị kết quả
        cv2.imshow('Hand Detection', image)
        
        # Thoát nếu nhấn phím 'q'
        if cv2.waitKey(5) & 0xFF == ord('q'):
            break

finally:
    # Giải phóng tài nguyên
    hands.close()
    cap.release()
    cv2.destroyAllWindows()
    client.loop_stop()
    client.disconnect()
    print("Đã đóng hệ thống.")