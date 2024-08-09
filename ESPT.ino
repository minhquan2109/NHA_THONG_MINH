#include <WiFi.h>                      // Thư viện kết nối WiFi.
#include <HTTPClient.h>                // Thư viện để tạo yêu cầu HTTP.
#include <WiFiClientSecure.h>          // Thư viện kết nối client bảo mật (SSL/TLS).
#include "freertos/FreeRTOS.h"         // Thư viện FreeRTOS cho đa nhiệm.
#include "freertos/task.h"             // Thư viện quản lý tác vụ của FreeRTOS.
#include <Arduino.h>                   // Thư viện Arduino cơ bản.
#include "FreeRTOSConfig.h"            // Tập tin cấu hình FreeRTOS.
#include <esp_task_wdt.h>              // Thư viện quản lý watchdog timer.
#define BLYNK_TEMPLATE_ID "TMPL6d3nOXz76"       // ID mẫu Blynk.
#define BLYNK_TEMPLATE_NAME "ESP32"             // Tên mẫu Blynk.
#define BLYNK_AUTH_TOKEN "-fUaRpB_YBcjXbGdDhmWzrGAR27sULlU" // Mã thông báo xác thực Blynk.
#define BLYNK_PRINT Serial                      // Cấu hình để in ra Serial monitor.
#include <BlynkSimpleEsp32.h>                   // Thư viện Blynk cho ESP32.
// Định nghĩa chân nút nhấn và led
#define bt0 15
#define led0 25
#define bt1 17
#define led1 26
#define bt2 16
#define led2 27
#define bt3 18
#define led3 14
#define bt4 19  
#define led4 12
//Định nghĩa chân ảo Blynk
#define VIRTUAL_PIN0 V0
#define VIRTUAL_PIN1 V1
#define VIRTUAL_PIN2 V2
#define VIRTUAL_PIN3 V3
#define VIRTUAL_PIN4 V4
//Biến trạng thái nút nhấn và đèn LED
int buttonState0 = HIGH;
int ledState0 = LOW;
int buttonState1 = HIGH;
int ledState1 = LOW;
int buttonState2 = HIGH;
int ledState2 = LOW;
int buttonState3 = HIGH;
int ledState3 = LOW;
int buttonState4 = HIGH;
int ledState4 = LOW;
//Biến cảm biến
float t = 0.0;
float h = 0.0;
int flameLevel = 0;
int gasLevel = 0;
//Thông tin kết nối WiFi và ThingSpeak
char auth[] = "-fUaRpB_YBcjXbGdDhmWzrGAR27sULlU";      // Mã thông báo xác thực Blynk.
char ssid[] = "NHOM THEM TRA SUA 5G";                 // SSID WiFi.
char pass[] = "0337027822";                           // Mật khẩu WiFi.
const int myChannelNumber = 2555593;                  // Số kênh ThingSpeak.
const char* writeAPIKey = "UUX4MOG2DG484L7S";         // Khóa API ThingSpeak.
const char* server = "api.thingspeak.com";            // Máy chủ ThingSpeak.
//Hàm điều khiển Blynk cho đèn LED
BLYNK_WRITE(V0) {
  int pinValue0 = param.asInt();    // Đọc giá trị từ chân ảo V0.
  ledState0 = pinValue0;            // Cập nhật trạng thái đèn LED 0.
  digitalWrite(led0, ledState0);    // Điều khiển đèn LED 0.
}
BLYNK_WRITE(V1) {
  int pinValue1 = param.asInt();    // Đọc giá trị từ chân ảo V1.
  ledState1 = pinValue1;            // Cập nhật trạng thái đèn LED 1.
  digitalWrite(led1, ledState1);    // Điều khiển đèn LED 1.
}
BLYNK_WRITE(V2) {
  int pinValue2 = param.asInt();    // Đọc giá trị từ chân ảo V2.
  ledState2 = pinValue2;            // Cập nhật trạng thái đèn LED 2.
  digitalWrite(led2, ledState2);    // Điều khiển đèn LED 2.
}
BLYNK_WRITE(V3) {
  int pinValue3 = param.asInt();    // Đọc giá trị từ chân ảo V3.
  ledState3 = pinValue3;            // Cập nhật trạng thái đèn LED 3.
  digitalWrite(led3, ledState3);    // Điều khiển đèn LED 3.
}
BLYNK_WRITE(V4) {
  int pinValue4 = param.asInt();    // Đọc giá trị từ chân ảo V4.
  ledState4 = pinValue4;            // Cập nhật trạng thái đèn LED 4.
  digitalWrite(led4, ledState4);    // Điều khiển đèn LED 4.
}
//Hàm kiểm tra nút nhấn và điều khiển đèn LED
void checkButton0() {
  if (digitalRead(bt0) == LOW) {            // Nếu nút nhấn bt0 được nhấn.
    if (buttonState0 == HIGH) {             // Nếu trạng thái nút trước đó là HIGH.
      ledState0 = !ledState0;               // Đổi trạng thái đèn LED 0.
      Blynk.virtualWrite(V0, ledState0);    // Cập nhật trạng thái đèn LED 0 trên Blynk.
      digitalWrite(led0, ledState0);        // Điều khiển đèn LED 0.
      delay(200);                           // Trì hoãn để tránh nhiễu.
    }
    buttonState0 = LOW;                     // Cập nhật trạng thái nút hiện tại là LOW.
  } else {
    buttonState0 = HIGH;                    // Nếu nút không được nhấn, trạng thái nút là HIGH.
  }
}
void checkButton1() {
  if (digitalRead(bt1) == LOW) {            // Nếu nút nhấn bt1 được nhấn.
    if (buttonState1 == HIGH) {             // Nếu trạng thái nút trước đó là HIGH.
      ledState1 = !ledState1;               // Đổi trạng thái đèn LED 1.
      Blynk.virtualWrite(V1, ledState1);    // Cập nhật trạng thái đèn LED 1 trên Blynk.
      digitalWrite(led1, ledState1);        // Điều khiển đèn LED 1.
      delay(200);                           // Trì hoãn để tránh nhiễu.
    }
    buttonState1 = LOW;                     // Cập nhật trạng thái nút hiện tại là LOW.
  } else {
    buttonState1 = HIGH;                    // Nếu nút không được nhấn, trạng thái nút là HIGH.
  }
}
void checkButton2() {                       
  if (digitalRead(bt2) == LOW) {            // Nếu nút nhấn bt2 được nhấn.
    if (buttonState2 == HIGH) {             // Nếu trạng thái nút trước đó là HIGH.
      ledState2 = !ledState2;               // Đổi trạng thái đèn LED 2. 
      Blynk.virtualWrite(V2, ledState2);    // Cập nhật trạng thái đèn LED 2 trên Blynk.
      digitalWrite(led2, ledState2);        // Điều khiển đèn LED 2.
      delay(200);                           // Trì hoãn để tránh nhiễu. 
    }
    buttonState2 = LOW;
  } else {
    buttonState2 = HIGH;
  }
}
void checkButton3() {
  if (digitalRead(bt3) == LOW) {            // Nếu nút nhấn bt3 được nhấn.
    if (buttonState3 == HIGH) {             // Nếu trạng thái nút trước đó là HIGH.
      ledState3 = !ledState3;               // Đổi trạng thái đèn LED 3. 
      Blynk.virtualWrite(V3, ledState3);    // Cập nhật trạng thái đèn LED 3 trên Blynk.
      digitalWrite(led3, ledState3);        // Điều khiển đèn LED 3.
      delay(200);                           // Trì hoãn để tránh nhiễu.
    }
    buttonState3 = LOW;
  } else {
    buttonState3 = HIGH;
  }
}
void checkButton4() {
  if (digitalRead(bt4) == LOW) {            // Nếu nút nhấn bt4 được nhấn.
    if (buttonState4 == HIGH) {             // Nếu trạng thái nút trước đó là HIGH.
      ledState4 = !ledState4;               // Đổi trạng thái đèn LED 4. 
      Blynk.virtualWrite(V4, ledState4);    // Cập nhật trạng thái đèn LED 4 trên Blynk.
      digitalWrite(led4, ledState4);        // Điều khiển đèn LED 4.
      delay(200);                           // Trì hoãn để tránh nhiễu.
    }
    buttonState4 = LOW;
  } else {
    buttonState4 = HIGH;
  }
}
void setup() {
  Serial.begin(9600); // Khởi động giao tiếp Serial với tốc độ truyền 9600 bps.
  WiFi.disconnect(); // Ngắt kết nối WiFi hiện tại (nếu có).
  delay(2000); // Chờ 2 giây để đảm bảo WiFi đã ngắt kết nối hoàn toàn.
  Serial.println("Connecting to "); // In ra Serial Monitor thông báo "Connecting to".
  Serial.println(ssid); // In ra tên mạng WiFi mà thiết bị sẽ kết nối.
  WiFi.begin(ssid, pass); // Bắt đầu kết nối WiFi với SSID và mật khẩu đã định nghĩa trước đó.
  xTaskCreate(readData,"ReadData",4000,NULL,1,NULL); // Tạo task "readData" với stack size 4000 bytes, priority 1.
  xTaskCreate(sendData,"SendData",4000,NULL,1,NULL); // Tạo task "sendData" với stack size 4000 bytes, priority 1.
  xTaskCreate(Task_Control,"Task_Control",8000,NULL,1,NULL); // Tạo task "Task_Control" với stack size 8000 bytes, priority 1.
  xTaskCreate(Task_Blynk_Loop,"loop1",6000,NULL,1,NULL); // Tạo task "Task_Blynk_Loop" với stack size 6000 bytes, priority 1.
}
void loop() {
  // Hàm loop() trống vì các tác vụ đã được xử lý bởi FreeRTOS tasks.
}
void Task_Control(void *pvParameters) {
  (void)pvParameters; // Không sử dụng tham số này, chỉ để tránh cảnh báo compiler.
  
  // Thiết lập chế độ cho các chân nút nhấn và đèn LED
  pinMode(bt0, INPUT_PULLUP);
  pinMode(bt1, INPUT_PULLUP);
  pinMode(bt2, INPUT_PULLUP);
  pinMode(bt3, INPUT_PULLUP);
  pinMode(bt4, INPUT_PULLUP);
  pinMode(led0, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  
  while (1) { // Vòng lặp vô tận để task luôn chạy.
    checkButton0(); // Kiểm tra trạng thái của nút nhấn bt0 và thực hiện hành động tương ứng.
    checkButton1(); // Kiểm tra trạng thái của nút nhấn bt1 và thực hiện hành động tương ứng.
    checkButton2(); // Kiểm tra trạng thái của nút nhấn bt2 và thực hiện hành động tương ứng.
    checkButton3(); // Kiểm tra trạng thái của nút nhấn bt3 và thực hiện hành động tương ứng.
    checkButton4(); // Kiểm tra trạng thái của nút nhấn bt4 và thực hiện hành động tương ứng.
  }
}
void readData(void* parameter) {
    while (1) { // Vòng lặp vô tận để task luôn chạy.
      if (Serial.available()) { // Kiểm tra xem có dữ liệu nào đang chờ đọc từ Serial không.
        String data = Serial.readStringUntil('\n'); // Đọc một dòng dữ liệu từ Serial cho đến khi gặp ký tự '\n'.
        
        if (data.startsWith("H:")) { // Kiểm tra nếu dữ liệu bắt đầu với "H:".
            int endIndex = data.indexOf(","); // Tìm vị trí của dấu phẩy đầu tiên.
            String humidityData = data.substring(2, endIndex); // Lấy dữ liệu độ ẩm từ vị trí 2 đến vị trí dấu phẩy.
            h = humidityData.toFloat(); // Chuyển đổi dữ liệu độ ẩm sang kiểu float và lưu vào biến h.
            data = data.substring(endIndex + 1); // Cắt bỏ phần dữ liệu đã xử lý.
        }
        
        if (data.startsWith("T:")) { // Kiểm tra nếu dữ liệu bắt đầu với "T:".
            int endIndex = data.indexOf(","); // Tìm vị trí của dấu phẩy đầu tiên.
            String tempData = data.substring(2, endIndex); // Lấy dữ liệu nhiệt độ từ vị trí 2 đến vị trí dấu phẩy.
            t = tempData.toFloat(); // Chuyển đổi dữ liệu nhiệt độ sang kiểu float và lưu vào biến t.
            data = data.substring(endIndex + 1); // Cắt bỏ phần dữ liệu đã xử lý.
        }
        
        if (data.startsWith("Gas:")) { // Kiểm tra nếu dữ liệu bắt đầu với "Gas:".
            int endIndex = data.indexOf(","); // Tìm vị trí của dấu phẩy đầu tiên.
            String gasData = data.substring(4, endIndex); // Lấy dữ liệu khí gas từ vị trí 4 đến vị trí dấu phẩy.
            gasLevel = gasData.toInt(); // Chuyển đổi dữ liệu khí gas sang kiểu int và lưu vào biến gasLevel.
            data = data.substring(endIndex + 1); // Cắt bỏ phần dữ liệu đã xử lý.
        }
        
        if (data.startsWith("Flame:")) { // Kiểm tra nếu dữ liệu bắt đầu với "Flame:".
            String flameData = data.substring(6, data.length()); // Lấy dữ liệu ngọn lửa từ vị trí 6 đến hết chuỗi.
            flameLevel = flameData.toInt(); // Chuyển đổi dữ liệu ngọn lửa sang kiểu int và lưu vào biến flameLevel.
        }
      }
      delay(100); // Chờ 100ms trước khi kiểm tra lần tiếp theo.
    }
}
void sendData(void* parameter) {
  while (1) { // Vòng lặp vô tận để task luôn chạy.
    if (WiFi.status() == WL_CONNECTED) { // Kiểm tra xem WiFi có kết nối không.
      WiFiClient client; // Tạo một đối tượng WiFiClient.
      HTTPClient http; // Tạo một đối tượng HTTPClient.
      String url = "http://" + String(server) + "/update?api_key=" + writeAPIKey; // Tạo URL để gửi dữ liệu.
      url += "&field1=" + String(t); // Thêm dữ liệu nhiệt độ vào URL.
      url += "&field2=" + String(h); // Thêm dữ liệu độ ẩm vào URL.
      url += "&field3=" + String(flameLevel); // Thêm dữ liệu mức độ lửa vào URL.
      url += "&field4=" + String(gasLevel); // Thêm dữ liệu mức độ khí gas vào URL.
      
      // In dữ liệu ra Serial Monitor để debug
      Serial.print("Temperature: ");
      Serial.println(t);
      Serial.print("Humidity: ");
      Serial.println(h);
      Serial.print("Flame level: ");
      Serial.println(flameLevel);
      Serial.print("Gas level: ");
      Serial.println(gasLevel);
      
      http.begin(client, url); // Bắt đầu kết nối HTTP với URL.
      int httpResponseCode = http.GET(); // Gửi yêu cầu GET và nhận mã phản hồi.
      
      if (httpResponseCode > 0) { // Nếu mã phản hồi lớn hơn 0, tức là yêu cầu thành công.
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode); // In mã phản hồi ra Serial Monitor.
      } else { // Nếu mã phản hồi nhỏ hơn hoặc bằng 0, tức là yêu cầu thất bại.
        Serial.print("Error code: ");
        Serial.println(httpResponseCode); // In mã lỗi ra Serial Monitor.
      }
      http.end(); // Kết thúc kết nối HTTP.
    } else {
      Serial.println("WiFi Disconnected"); // In ra Serial Monitor nếu WiFi bị ngắt kết nối.
    }
    delay(1000);  // Chờ 1 giây trước khi gửi dữ liệu lần tiếp theo.
  }
}
void Task_Blynk_Loop(void *pvParameters) {
  (void)pvParameters; // Không sử dụng tham số này, chỉ để tránh cảnh báo compiler.

  Blynk.begin(auth, ssid, pass); // Kết nối Blynk với thông tin xác thực và WiFi.
  while (Blynk.connected() == false) { // Chờ cho đến khi kết nối Blynk thành công.
  }
  Serial.println();
  Serial.println("Blynk Connected"); // In ra Serial Monitor khi kết nối thành công.
  Serial.println("Blynk Loop Task Started"); // In ra Serial Monitor khi task bắt đầu.
  while (1) { // Vòng lặp vô tận để task luôn chạy.
    Blynk.run(); // Chạy Blynk.
    delay(1); // Chờ 1ms trước khi lặp lại.
  }
}

