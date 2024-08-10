#include <Wire.h> // Thư viện giao tiếp I2C
#include <Keypad.h> // Thư viện điều khiển bàn phím ma trận
#include <LiquidCrystal_I2C.h> // Thư viện điều khiển LCD qua giao tiếp I2C
#include <Servo.h> // Thư viện điều khiển servo
#include <SDHT.h> // Thư viện cảm biến DHT
#include <Arduino_FreeRTOS.h> // Thư viện FreeRTOS cho Arduino
#include <EEPROM.h> // Thư viện EEPROM để lưu trữ dữ liệu không bay hơi
#include <SPI.h> // Thư viện giao tiếp SPI
#include <MFRC522.h> // Thư viện điều khiển RFID
//////////////////////////////
#define COMMON_ANODE // Định nghĩa loại LED là anode chung

#ifdef COMMON_ANODE
#define LED_ON LOW // Nếu là anode chung, LED bật khi ở mức LOW
#define LED_OFF HIGH // LED tắt khi ở mức HIGH
#else
#define LED_ON HIGH // Nếu là cathode chung, LED bật khi ở mức HIGH
#define LED_OFF LOW // LED tắt khi ở mức LOW


#define redLed 7       
#define greenLed 6   // Chân kết nối LED xanh lá
#define blueLed 5    // Chân kết nối LED xanh dương
#define relay 13     // Chân kết nối relay
#define buzzer 33    // Chân kết nối buzzer
#define LED_PW 4     // Chân kết nối LED nguồn
#define Analog_gas A4 // Chân analog kết nối cảm biến khí gas
#define Analog_lua A3 // Chân analog kết nối cảm biến lửa
#define led 34        // Chân kết nối LED
#define pin_dht22 48  // Chân kết nối cảm biến DHT22
/////////////////////
#define FILTER_SIZE 26 // Kích thước bộ lọc
#define LED_DELAY 500 // Độ trễ của LED
/////////////////////

Servo myServo; // Khai báo đối tượng servo
int servoPin = 10; // Chân kết nối servo
#define trig A0            // Chân trig của cảm biến siêu âm HC-SR04
#define echo A1            / /Chân echo của cảm biến siêu âm HC-SR04

boolean match = false; // Biến kiểm tra thẻ RFID có khớp không
boolean programMode = false; // Biến kiểm tra chế độ lập trình
int successRead; // Biến kiểm tra đọc thẻ thành công

byte storedCard[4]; // Mảng lưu trữ thẻ đã lưu
byte readCard[4]; // Mảng lưu trữ thẻ đang đọc
byte masterCard[4]; // Mảng lưu trữ thẻ chủ

#define SS_PIN 53 // Chân kết nối SS của RFID
#define RST_PIN 49 // Chân kết nối RST của RFID
MFRC522 mfrc522(SS_PIN, RST_PIN); // Khai báo đối tượng MFRC522 cho RFID

//--------------------------------------------
LiquidCrystal_I2C lcd2(0X27, 20, 4);
LiquidCrystal_I2C lcd(0x26, 16, 2);
SDHT dht;
//................................................
char password[4];
char initial_password[4],new_password[4];
int vcc=11;
int i=0;
//int relay_pin = 11;
char key_pressed=0;
const byte rows = 4; 
const byte columns = 4; 
char hexaKeys[rows][columns] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
byte row_pins[rows]={A8,A9,A10,A11};
byte column_pins[columns]={A12,A13,A14,A15};
Keypad keypad_key=Keypad( makeKeymap(hexaKeys),row_pins,column_pins,rows,columns);
//----------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);
  xTaskCreate(Humi_Temp_function, "Humi_Temp", 200, NULL, 2, NULL);
  xTaskCreate(RFID_CLOCK_function, "RFID_CLOCK", 500, NULL, 1, NULL);
  xTaskCreate(keypad_password_function, "keypad", 500, NULL,1, NULL);
  xTaskCreate(distance_hcsr04_function, "hcsr04", 500, NULL, 1, NULL);
}
//......................................................
float GetDistance()
{
    unsigned long duration; // biến đo thời gian
    int distanceCm;         // biến lưu khoảng cách
    /* Phát xung từ chân trig */
    digitalWrite(trig,0);   // tắt chân trig
    delayMicroseconds(2);
    digitalWrite(trig,1);   // phát xung từ chân trig
    delayMicroseconds(5);   // xung có độ dài 5 microSeconds
    digitalWrite(trig,0);   // tắt chân trig
    /* Tính toán thời gian */
    // Đo độ rộng xung HIGH ở chân echo. 
    duration = pulseIn(echo,HIGH);  
    // Tính khoảng cách đến vật.
    distanceCm = int(duration/2/29.412);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  return distanceCm;
}

void distance_function()
{
  long distance = GetDistance();
  if (distance <= 0)
  {
    Serial.println("Quá thời gian đo khoảng cách !!");
  }
  else
  {   
    Serial.print("Khoảng cách (cm): ");
    Serial.println(distance);
  }
  if (distance < 20) 
  {
    runServo();
  }
}

void runServo() {
  myServo.write(90);  // Cho servo quay một góc 90 độ
  //delay(3000);
  vTaskDelay(3000 / portTICK_PERIOD_MS);
  myServo.write(0); // Cho servo trở về vị trí ban đầu
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  //delay(1000);
}
void lockDoor() {
  digitalWrite(relay, LOW);
  digitalWrite(LED_PW, LOW);
}
void unlockDoor() {
  digitalWrite(relay, HIGH);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  digitalWrite(relay, LOW); 
  digitalWrite(LED_PW, HIGH);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  digitalWrite(LED_PW, LOW);
}
long measurements2[FILTER_SIZE];
int measurementIndex2 = 0;

void addMeasurement2(long measurement) {
  measurements2[measurementIndex2] = measurement;
  measurementIndex2 = (measurementIndex2 + 1) % FILTER_SIZE;
}

long getFilteredMeasurement2() {
  long sum = 0;
  for (int i = 0; i < FILTER_SIZE; i++) {
    sum += measurements2[i];
  }
  return sum / FILTER_SIZE;
}
//-----------------------------------------------------------
void keypad_function()
{
  //digitalWrite(relay,LOW);
  key_pressed = keypad_key.getKey();
  if(key_pressed=='#')
  change();
  if(key_pressed)
  {
    password[i++]=key_pressed;
    lcd.print(key_pressed);
  }
  if(i==4)
  {
    vTaskDelay(200 / portTICK_PERIOD_MS);
    for(int j=0;j<4;j++)
    initial_password[j]=EEPROM.read(j);
    if(!(strncmp(password, initial_password,4)))
    {
      lcd.clear();
      lcd.print("Pass Accepted");
      //digitalWrite(relay,LOW);
      unlockDoor();
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      lcd.setCursor(0,0);
      lcd.print("Pres >START< to");
      lcd.setCursor(0,1);
      lcd.print("change the pass");
      vTaskDelay(3000 / portTICK_PERIOD_MS);
      lcd.clear();
      lcd.print("Enter Password:");
      lcd.setCursor(0,1);
      i=0;
    }
    else
    {
      //digitalWrite(relay, HIGH);
      lockDoor();
      lcd.clear();
      lcd.print("Wrong Password");
      lcd.setCursor(0,0);
      lcd.print("Pres >#< to");
      lcd.setCursor(0,1);
      lcd.print("change the pass");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      lcd.clear();
      lcd.print("Enter Password");
      lcd.setCursor(0,1);
      i=0;
    }
  }
}
void change()
{
  int j=0;
  lcd.clear();
  lcd.print("Current Password");
  lcd.setCursor(0,1);
  while(j<4)
  {
    char key=keypad_key.getKey();
    if(key)
    {
      new_password[j++]=key;
      lcd.print(key);
    }
    key=0;
  }
  vTaskDelay(500 / portTICK_PERIOD_MS);
  if((strncmp(new_password, initial_password, 4)))
  {
    lcd.clear();
    lcd.print("Wrong Password");
    lcd.setCursor(0,1);
    lcd.print("Try Again");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  else
  {
    j=0;
    lcd.clear();
    lcd.print("New Password:");
    lcd.setCursor(0,1);
    while(j<4)
    {
      char key=keypad_key.getKey();
      if(key)
      {
        initial_password[j]=key;
        lcd.print(key);
        EEPROM.write(j,key);
        j++;
      }
    }
    lcd.print("Pass Changed");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
  lcd.clear();
  lcd.print("Enter Password");
  lcd.setCursor(0,1);
  key_pressed=0;
}
void initialpassword()
{
  int j;
  for(j=0;j<4;j++)
  EEPROM.write(j,j+49);
  for(j=0;j<4;j++)
  initial_password[j]=EEPROM.read(j);
}

//------------------------------------------------------------------
void humid_function() {
  if (dht.read(DHT11, pin_dht22)) {
    Serial.print("H:");
    Serial.print(double(dht.humidity) / 10, 1);
    Serial.print(",T:");
    Serial.println(double(dht.celsius) / 10, 1);
    lcd2.setCursor(7, 0);
    lcd2.print(double(dht.humidity) / 10, 1);
    lcd2.setCursor(7, 1);
    lcd2.print(double(dht.celsius) / 10, 1);

    // Đọc giá trị từ cảm biến khí GAS
    int gasValue = analogRead(Analog_gas);
    Serial.print("Gas:");
    Serial.println(gasValue);
    lcd2.setCursor(7, 2);
    lcd2.print(gasValue);

    // Đọc giá trị từ cảm biến lửa (flame sensor)
    int flameValue = analogRead(Analog_lua);
    Serial.print("Flame:");
    Serial.println(flameValue);
    if (flameValue < 490) {
      lcd2.setCursor(0, 3);
      lcd2.print("                ");
      lcd2.setCursor(0, 3);
      lcd2.print("    CANH BAO CHAY");
      digitalWrite(buzzer, HIGH);
    } 
    else if (flameValue > 500) {
      lcd2.setCursor(0, 3);
      lcd2.print("                ");
      lcd2.setCursor(0, 3);
      lcd2.print("       AN TOAN    ");
      digitalWrite(buzzer, LOW);
    } 

    // Chuỗi dữ liệu để gửi
    String dataToSend = "H:" + String(double(dht.humidity) / 10, 1) + ",T:" + String(double(dht.celsius) / 10, 1) + ",Gas:" + String(gasValue) + ",Flame:" + String(flameValue) + "\n";
    Serial.println(dataToSend);
  }
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}
//------------------------------------------------------------------
void loop() 
{}

void Enter_Pass_Word(void *pvParameters) {
  Serial.print("Serial ON");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  lockDoor();
  pinMode(LED_PW, OUTPUT);
  while (1) {
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}
//---------------------------------------------
void keypad_password_function(void *pvParameters)
{
  lcd.init();
  lcd.begin(16,2);
  lcd.backlight();
  //pinMode(relay, OUTPUT);
  pinMode(vcc, OUTPUT);
  lcd.print(" MOUNT DYNAMICS ");
  lcd.setCursor(0,1);
  lcd.print("Electronic Lock ");
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter Password");
  lcd.setCursor(0,1);
  initialpassword();
  while(1)
  {
    keypad_function();
  }
} 
//-----------------------------------------------
void Humi_Temp_function(void *pvParameters) {
  pinMode(pin_dht22, INPUT);
  lcd2.init();
  lcd2.backlight();
  lcd2.begin(20, 4);
  lcd2.setCursor(0, 0);
  lcd2.print("HUMI:  ");
  lcd2.setCursor(0, 1);
  lcd2.print("TEMP: ");
  lcd2.setCursor(11, 0);
  lcd2.print(" %");
  lcd2.setCursor(11, 1);
  lcd2.print(" C");
  lcd2.setCursor(0, 2);
  lcd2.print("GAS:  ");

  while (1) {
    humid_function();
  }
}
/////////////////////////////////////////

void RFID_CLOCK() {
  do {
    successRead = getID();  // Cố gắng đọc ID của thẻ RFID
    if (programMode) {
      cycleLeds();  // Chạy các LED nếu đang ở chế độ lập trình
    } else {
      normalModeOn();  // Bật chế độ bình thường nếu không ở chế độ lập trình
    }
  } while (!successRead);  // Lặp lại cho đến khi đọc thành công một thẻ
  
  if (programMode) {
    if (isMaster(readCard)) {
      Serial.println("This is Master Card");  // Đây là thẻ master
      Serial.println("Exiting Program Mode");  // Thoát khỏi chế độ lập trình
      Serial.println("-----------------------------");
      programMode = false;  // Đặt chế độ lập trình thành false
      return;
    } else {
      if (findID(readCard)) {
        Serial.println("I know this PICC, so removing");  // Thẻ đã được biết, loại bỏ nó
        deleteID(readCard);  // Xóa ID của thẻ khỏi danh sách
        Serial.println("-----------------------------");
      } else {
        Serial.println("I do not know this PICC, adding...");  // Thẻ chưa được biết, thêm nó vào
        writeID(readCard);  // Ghi ID của thẻ vào danh sách
        Serial.println("-----------------------------");
      }
    }
  } else {
    if (isMaster(readCard)) {
      programMode = true;  // Đặt chế độ lập trình thành true
      Serial.println("Hello Master - Entered Program Mode");  // Chào master, đã vào chế độ lập trình
      int count = EEPROM.read(0);  // Đọc số lượng bản ghi từ EEPROM
      Serial.print("I have ");
      Serial.print(count);
      Serial.print(" record(s) on EEPROM");
      Serial.println("");
      Serial.println("Scan a PICC to ADD or REMOVE");  // Quét thẻ để thêm hoặc loại bỏ
      Serial.println("-----------------------------");
    } else {
      if (findID(readCard)) {
        Serial.println("Welcome, You shall pass");  // Chào mừng, bạn được phép qua
        digitalWrite(greenLed, HIGH);  // Bật LED xanh lá
        digitalWrite(relay, HIGH);  // Bật relay
        digitalWrite(redLed, LOW);  // Tắt LED đỏ
        vTaskDelay(1500 / portTICK_PERIOD_MS);  // Đợi 1500 mili giây
        digitalWrite(greenLed, LOW);  // Tắt LED xanh lá
        digitalWrite(relay, LOW);  // Tắt relay
      } else {
        Serial.println("You shall not pass");  // Bạn không được phép qua
        failed();  // Gọi hàm failed để xử lý trường hợp không thành công
      }
    }
  }
}

//////////////////////////////////////////////////////
void RFID_CLOCK_function(void *pvParameters) {
  // Thiết lập chế độ cho các chân LED và còi (buzzer)
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  // Tắt tất cả các LED ban đầu
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  digitalWrite(blueLed, LED_OFF);
  
  // Khởi tạo giao tiếp SPI và đầu đọc RFID MFRC522
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Thiết lập độ nhạy của ăng-ten RFID lên mức tối đa
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  
  // Kiểm tra xem thẻ master đã được định nghĩa trong EEPROM chưa
  if (EEPROM.read(1) != 143) {
    Serial.println("No Master Card Defined");
    Serial.println("Scan A PICC to Define as Master Card");
    
    // Vòng lặp đợi cho đến khi một thẻ RFID hợp lệ được quét
    do {
      successRead = getID();  // Hàm đọc ID của thẻ RFID
      digitalWrite(blueLed, LED_ON);
      vTaskDelay(200 / portTICK_PERIOD_MS);
      digitalWrite(blueLed, LED_OFF);
      vTaskDelay(200 / portTICK_PERIOD_MS);
    } while (!successRead);
    
    // Lưu ID của thẻ đã quét vào EEPROM làm thẻ master
    for (int j = 0; j < 4; j++) {
      EEPROM.write(2 + j, readCard[j]);
    }
    EEPROM.write(1, 143);
    Serial.println("Master Card Defined");
  }
  
  // In UID của thẻ master ra màn hình serial
  Serial.println("Master Card's UID");
  for (int i = 0; i < 4; i++) {
    masterCard[i] = EEPROM.read(2 + i);
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");  // In một dòng mới
  Serial.println("Waiting PICCs to be scanned :)");
  
  // Chạy các LED để báo hiệu hệ thống đã sẵn sàng
  cycleLeds();
  
  // Vòng lặp chính để liên tục kiểm tra các thẻ RFID
  while (1) {
    RFID_CLOCK();  // Hàm xử lý logic cho đồng hồ RFID
    // vTaskDelay(500/portTICK_PERIOD_MS);  // Độ trễ tùy chọn để làm chậm vòng lặp
  }
}
/////////////////////////////////////////////////////////////
void distance_hcsr04_function(void *pvParameters) {
  myServo.attach(servoPin); // Gắn servo vào chân điều khiển được chỉ định bởi servoPin
  myServo.write(0); // Đặt servo ở vị trí 0 độ
  pinMode(trig, OUTPUT); // Cấu hình chân trig làm chân xuất tín hiệu
  pinMode(echo, INPUT);  // Cấu hình chân echo làm chân nhập tín hiệu
  while(1) { // Vòng lặp vô tận
    distance_function(); // Gọi hàm đo khoảng cách
  }
}
//////////////////////////////////////////////////////////////

int getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0; // Nếu không có thẻ mới được đưa vào, trả về 0
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return 0; // Nếu không đọc được UID của thẻ, trả về 0
  }

  Serial.println("Scanned PICC's UID:");
  for (int i = 0; i < 4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i]; // Lưu UID của thẻ vào mảng readCard
    Serial.print(readCard[i], HEX); // In từng byte của UID dưới dạng hệ 16
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); // Dừng giao tiếp với thẻ
  return 1; // Trả về 1 khi đã đọc được thẻ thành công
}


void cycleLeds() {
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_ON);
  digitalWrite(blueLed, LED_OFF);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
  digitalWrite(blueLed, LED_ON);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  digitalWrite(redLed, LED_ON);
  digitalWrite(greenLed, LED_OFF);
  digitalWrite(blueLed, LED_OFF);
  vTaskDelay(200 / portTICK_PERIOD_MS);
}

void normalModeOn() {
  digitalWrite(blueLed, LED_ON);
  digitalWrite(redLed, LED_OFF);
  digitalWrite(greenLed, LED_OFF);
}

void readID(int number) {
  int start = (number * 4) + 2;
  for (int i = 0; i < 4; i++) {
    storedCard[i] = EEPROM.read(start + i);
  }
}


void writeID(byte a[]) {
  if (!findID(a)) {
    int num = EEPROM.read(0);
    int start = (num * 4) + 6;
    num++;
    EEPROM.write(0, num);
    for (int j = 0; j < 4; j++) {
      EEPROM.write(start + j, a[j]);
    }
    successWrite();
  } else {
    failedWrite();
  }
}


void deleteID(byte a[]) {
  if (!findID(a)) {
    failedWrite();
  } else {
    int num = EEPROM.read(0);
    int slot;
    int start;
    int looping;
    int j;
    int count = EEPROM.read(0);
    slot = findIDSLOT(a);
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;
    EEPROM.write(0, num);
    for (j = 0; j < looping; j++) {
      EEPROM.write(start + j, EEPROM.read(start + 4 + j));
    }
    for (int k = 0; k < 4; k++) {
      EEPROM.write(start + j + k, 0);
    }
    successDelete();
  }
}


boolean checkTwo(byte a[], byte b[]) {
  if (a[0] != NULL)
    match = true;
  for (int k = 0; k < 4; k++) {
    if (a[k] != b[k])
      match = false;
  }
  if (match) {
    return true;
  } else {
    return false;
  }
}


int findIDSLOT(byte find[]) {
  int count = EEPROM.read(0);
  for (int i = 1; i <= count; i++) {
    readID(i);
    if (checkTwo(find, storedCard)) {

      return i;
      break;
    }
  }
}


boolean findID(byte find[]) {
  int count = EEPROM.read(0);
  for (int i = 1; i <= count; i++) {
    readID(i);
    if (checkTwo(find, storedCard)) {
      return true;
      break;
    } else {
    }
  }
  return false;
}



void successWrite() {
  digitalWrite(blueLed, LED_OFF); // Tắt đèn LED xanh
  digitalWrite(redLed, LED_OFF);  // Tắt đèn LED đỏ
  digitalWrite(greenLed, LED_OFF);// Tắt đèn LED xanh lá
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(greenLed, LED_ON); // Bật đèn LED xanh lá
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(greenLed, LED_OFF); // Tắt đèn LED xanh lá
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(greenLed, LED_ON); // Bật đèn LED xanh lá
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(greenLed, LED_OFF); // Tắt đèn LED xanh lá
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(greenLed, LED_ON); // Bật đèn LED xanh lá
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  Serial.println("Succesfully added ID record to EEPROM"); // In ra thông báo thành công trên Serial Monitor
}

void failedWrite() {
  digitalWrite(blueLed, LED_OFF); // Tắt đèn LED xanh
  digitalWrite(redLed, LED_OFF);  // Tắt đèn LED đỏ
  digitalWrite(greenLed, LED_OFF);// Tắt đèn LED xanh lá
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(redLed, LED_ON);  // Bật đèn LED đỏ
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(redLed, LED_OFF); // Tắt đèn LED đỏ
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(redLed, LED_ON);  // Bật đèn LED đỏ
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(redLed, LED_OFF); // Tắt đèn LED đỏ
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(redLed, LED_ON);  // Bật đèn LED đỏ
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  Serial.println("Failed! There is something wrong with ID or bad EEPROM"); // In ra thông báo thất bại trên Serial Monitor
}

void successDelete() {
  digitalWrite(blueLed, LED_OFF); // Tắt đèn LED xanh
  digitalWrite(redLed, LED_OFF);  // Tắt đèn LED đỏ
  digitalWrite(greenLed, LED_OFF);// Tắt đèn LED xanh lá
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(blueLed, LED_ON);  // Bật đèn LED xanh
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(blueLed, LED_OFF); // Tắt đèn LED xanh
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(blueLed, LED_ON);  // Bật đèn LED xanh
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(blueLed, LED_OFF); // Tắt đèn LED xanh
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  digitalWrite(blueLed, LED_ON);  // Bật đèn LED xanh
  vTaskDelay(200 / portTICK_PERIOD_MS); // Dừng lại 200 milisecond
  Serial.println("Succesfully removed ID record from EEPROM"); // In ra thông báo trên Serial Monitor
}

boolean isMaster(byte test[]) {
  if (checkTwo(test, masterCard)) // Kiểm tra xem mảng 'test' có khớp với 'masterCard' hay không
    return true; // Nếu khớp thì trả về true
  else
    return false; // Nếu không khớp thì trả về false
}

void failed() {
  digitalWrite(greenLed, LED_ON); // Bật đèn LED xanh lá
  digitalWrite(blueLed, LED_ON); // Bật đèn LED xanh
  digitalWrite(redLed, LED_OFF); // Tắt đèn LED đỏ
  vTaskDelay(1200 / portTICK_PERIOD_MS); // Dừng lại 1200 milisecond (1.2 giây)
}

