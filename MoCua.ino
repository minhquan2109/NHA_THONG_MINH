#include <Arduino.h>  // Thư viện Arduino cơ bản.
int ls1 = 8;   // Định nghĩa chân cảm biến ls1.
int ls2 = 9;   // Định nghĩa chân cảm biến ls2.
int S1 = 41;   // Định nghĩa chân cảm biến S1.
int S2 = 43;   // Định nghĩa chân cảm biến S2.
int e1 = 36;   // Định nghĩa chân điều khiển tốc độ động cơ 1.
int e2 = 38;   // Định nghĩa chân điều khiển tốc độ động cơ 2.
int i1 = 12;   // Định nghĩa chân điều khiển hướng động cơ 1.
int i2 = 11;   // Định nghĩa chân điều khiển hướng động cơ 2.
int H = 180;   // Định nghĩa tốc độ động cơ.
void forward() {
  digitalWrite(i1, HIGH);    // Đặt hướng động cơ 1 là tiến tới.
  digitalWrite(i2, LOW);     // Đặt hướng động cơ 2 là lùi lại.
  analogWrite(e1, H);        // Thiết lập tốc độ động cơ 1.
  analogWrite(e2, H);        // Thiết lập tốc độ động cơ 2.
}
void goback() {
  digitalWrite(i1, LOW);     // Đặt hướng động cơ 1 là lùi lại.
  digitalWrite(i2, HIGH);    // Đặt hướng động cơ 2 là tiến tới.
  analogWrite(e1, H);        // Thiết lập tốc độ động cơ 1.
  analogWrite(e2, H);        // Thiết lập tốc độ động cơ 2.
}
void stop() {
  digitalWrite(i1, LOW);     // Tắt động cơ 1.
  digitalWrite(i2, LOW);     // Tắt động cơ 2.
  analogWrite(e1, 0);        // Thiết lập tốc độ động cơ 1 bằng 0.
  analogWrite(e2, 0);        // Thiết lập tốc độ động cơ 2 bằng 0.
}
void setup() {
  Serial.begin(9600);              // Khởi động Serial với tốc độ 9600.
  pinMode(ls1, INPUT_PULLUP);      // Thiết lập chân ls1 là đầu vào với điện trở kéo lên.
  pinMode(ls2, INPUT_PULLUP);      // Thiết lập chân ls2 là đầu vào với điện trở kéo lên.
  pinMode(S1, INPUT);              // Thiết lập chân S1 là đầu vào.
  pinMode(S2, INPUT);              // Thiết lập chân S2 là đầu vào.
  pinMode(i1, OUTPUT);             // Thiết lập chân i1 là đầu ra.
  pinMode(i2, OUTPUT);             // Thiết lập chân i2 là đầu ra.
  pinMode(e1, OUTPUT);             // Thiết lập chân e1 là đầu ra.
  pinMode(e2, OUTPUT);             // Thiết lập chân e2 là đầu ra.
}
void loop() {
  int rls1 = digitalRead(ls1);   // Đọc giá trị từ cảm biến ls1.
  int rls2 = digitalRead(ls2);   // Đọc giá trị từ cảm biến ls2.
  int rS1 = digitalRead(S1);     // Đọc giá trị từ cảm biến S1.
  int rS2 = digitalRead(S2);     // Đọc giá trị từ cảm biến S2.

  // Điều kiện kiểm tra và hành động tương ứng
  if (rls1 == 1 && rls2 == 1 && rS1 == 1 && rS2 == 1) {
    forward();    // Tiến tới khi tất cả cảm biến đều kích hoạt.
  } else if (rls1 == 1 && rls2 == 1 && rS1 == 1 && rS2 == 0) {
    forward();    // Tiến tới khi các cảm biến trừ S2 kích hoạt.
  } else if (rls1 == 1 && rls2 == 1 && rS1 == 0 && rS2 == 0) {
    goback();     // Lùi lại khi chỉ các cảm biến ls1 và ls2 kích hoạt.
  } else if (rls1 == 1 && rls2 == 0 && rS1 == 0 && rS2 == 0) {
    goback();     // Lùi lại khi chỉ cảm biến ls1 kích hoạt.
  } else if (rls1 == 0 && rls2 == 1 && rS1 == 1 && rS2 == 1) {
    forward();    // Tiến tới khi chỉ cảm biến ls2 kích hoạt.
  } else if (rls1 == 0 && rls2 == 1 && rS1 == 0 && rS2 == 0) {
    stop();       // Dừng lại khi chỉ cảm biến ls2 và S2 không kích hoạt.
  } else if (rls1 == 0 && rls2 == 1 && rS1 == 1 && rS2 == 0) {
    forward();    // Tiến tới khi chỉ cảm biến ls2 và S1 kích hoạt.
  } else if (rls1 == 0 && rls2 == 1 && rS1 == 0 && rS2 == 1) {
    forward();    // Tiến tới khi chỉ cảm biến ls2 và S2 kích hoạt.
  } else if (rls1 == 1 && rls2 == 0 && rS1 == 1 && rS2 == 0) {
    stop();       // Dừng lại khi chỉ cảm biến ls1 và S1 kích hoạt.
  } else if (rls1 == 1 && rls2 == 0 && rS1 == 0 && rS2 == 1) {
    stop();       // Dừng lại khi chỉ cảm biến ls1 và S2 kích hoạt.
  } else if (rls1 == 1 && rls2 == 0 && rS1 == 1 && rS2 == 1) {
    stop();       // Dừng lại khi chỉ cảm biến ls1 kích hoạt.
  } else if (rls1 == 0 && rls2 == 0 && rS1 == 1 && rS2 == 1) {
    stop();       // Dừng lại khi tất cả cảm biến đều không kích hoạt.
  }

  // In giá trị các cảm biến lên Serial monitor
  Serial.print(rls1);
  Serial.print(rls2);
  Serial.print(rS1);
  Serial.println(rS2);
  delay(100);    // Trì hoãn 100ms.
}
