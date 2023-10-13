
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ESP32_CAM_Send_Photo_to_Server
//======================================== Including the libraries.
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
//======================================== 

//======================================== CAMERA_MODEL_AI_THINKER GPIO.
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
//======================================== Insert your network credentials.
const char* ssid = "Iphone";
const char* password = "12121212";
//======================================== 

const int trigPin = 15;
const int echoPin = 13;
long duration;
int distance;

const int redPin = 14;
const int yellowPin = 2;
const int greenPin = 12;

// Server Address or Server IP.
String serverName = "175.41.176.164";  //--> Change with your server computer's IP address or your Domain name.
// The file path "upload_img.php" on the server folder.
String serverPath = "/api/storages/upload";
// Server Port.
const int serverPort = 8080;


WiFiClient client;

//________________________________________________________________________________ sendPhotoToServer()
void sendPhotoToServer() {
  String AllData;
  String DataBody;

  Serial.println();
  Serial.println("-----------");
 
  //---------------------------------------- Pre capture for accurate timing.
  Serial.println("Taking a photo...");

  // if (LED_Flash_ON == true) {
  //   digitalWrite(FLASH_LED_PIN, HIGH);
  //   delay(1000);
  // }
  
  for (int i = 0; i <= 3; i++) {
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
     if(!fb) {
        Serial.println("Camera capture failed");
        Serial.println("Restarting the ESP32 CAM.");
        delay(1000);
        ESP.restart();
        return;
      } 
    esp_camera_fb_return(fb);
    delay(200);
  }
  
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    Serial.println("Restarting the ESP32 CAM.");
    delay(1000);
    ESP.restart();
    return;
  } 

  // if (LED_Flash_ON == true) digitalWrite(FLASH_LED_PIN, LOW);
  
  Serial.println("Taking a photo was successful.");
  //---------------------------------------- 

  Serial.println("Connecting to server: " + serverName);

  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");   
    unsigned long currentMillis = millis(); // Hoặc sử dụng micros() để lấy thời gian chính xác hơn
    String fileName = "ESP32CAM_" + String(currentMillis) + ".jpg";
    String post_data = "--dataMarker\r\nContent-Disposition: form-data; name=\"file\"; filename=\"" + fileName + "\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String head = post_data;
    String boundary = "\r\n--dataMarker--\r\n";
    
    uint32_t imageLen = fb->len;
    uint32_t dataLen = head.length() + boundary.length();
    uint32_t totalLen = imageLen + dataLen;
    
    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=dataMarker");
    client.println();
    client.print(head);
  
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0; n<fbLen; n=n+1024) {
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
    }   
    client.print(boundary);
    
    esp_camera_fb_return(fb);
   
    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;
    Serial.println("Response : ");
    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(200);
         
      // Skip HTTP headers   
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (AllData.length()==0) { state=true; }
          AllData = "";
        }
        else if (c != '\r') { AllData += String(c); }
        if (state==true) { DataBody += String(c); }
        startTimer = millis();
      }
      if (DataBody.length()>0) { break; }
    }
    client.stop();
    Serial.println(DataBody);
    Serial.println("-----------");
    Serial.println();
    
  }
  else {
    client.stop();
    DataBody = "Connection to " + serverName +  " failed.";
    Serial.println(DataBody);
    Serial.println("-----------");
  }
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:

  // Disable brownout detector.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin,INPUT);

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  
  Serial.begin(115200);
  Serial.println();

  // pinMode(FLASH_LED_PIN, OUTPUT);

  // Setting the ESP32 WiFi to station mode.
  WiFi.mode(WIFI_STA);
  Serial.println();

  //---------------------------------------- The process of connecting ESP32 CAM with WiFi Hotspot / WiFi Router.
  Serial.println();
  Serial.print("Connecting to : ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  // The process timeout of connecting ESP32 CAM with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 CAM has not been successfully connected to WiFi, the ESP32 CAM will restart.
  // I made this condition because on my ESP32-CAM, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      Serial.println();
      Serial.print("Failed to connect to ");
      Serial.println(ssid);
      Serial.println("Restarting the ESP32 CAM.");
      delay(1000);
      ESP.restart();
    }
  }

  Serial.println();
  Serial.print("Successfully connected to ");
  Serial.println(ssid);
  //Serial.print("ESP32-CAM IP Address: ");
  //Serial.println(WiFi.localIP());
  //---------------------------------------- 

  //---------------------------------------- Set the camera ESP32 CAM.
  Serial.println();
  Serial.print("Set the camera ESP32 CAM...");
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //--> 0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 8;  //--> 0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println();
    Serial.println("Restarting the ESP32 CAM.");
    delay(1000);
    ESP.restart();
  }

  sensor_t * s = esp_camera_sensor_get();

  // Selectable camera resolution details :
  // -UXGA   = 1600 x 1200 pixels
  // -SXGA   = 1280 x 1024 pixels
  // -XGA    = 1024 x 768  pixels
  // -SVGA   = 800 x 600   pixels
  // -VGA    = 640 x 480   pixels
  // -CIF    = 352 x 288   pixels
  // -QVGA   = 320 x 240   pixels
  // -HQVGA  = 240 x 160   pixels
  // -QQVGA  = 160 x 120   pixels
  s->set_framesize(s, FRAMESIZE_SXGA); //--> UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

  Serial.println();
  Serial.println("Set camera ESP32 CAM successfully.");
  //---------------------------------------- 

  Serial.println();
}
//________________________________________________________________________________ 

//________________________________________________________________________________ VOID LOOP()
// void loop() {
//   // put your main code here, to run repeatedly:

//   //---------------------------------------- Timer/Millis to capture and send photos to server every 20 seconds (see Interval variable).
//   digitalWrite(trigPin, LOW);
//   // delayMicroseconds(2);

//   digitalWrite(trigPin,HIGH);
//   // delayMicroseconds(10);
//   digitalWrite(trigPin,LOW);
//   duration = pulseIn(echoPin, HIGH);
//   distance = duration*34/2;
//   Serial.print("Distance Measured = ");
//   Serial.print(distance);
//   Serial.println("mm");
//   // Serial.println("den do:",digitalRead(redPin) == HIGH);


//    // Đèn đỏ sáng, đèn vàng và đèn xanh tắt (đèn đỏ đang sáng)
//   // digitalWrite(redPin, HIGH);
//   // digitalWrite(yellowPin, LOW);
//   // digitalWrite(greenPin, LOW);
//   // delay(5000); // Chờ 3 giây

//   // Đèn xanh sáng, đèn đỏ và đèn vàng tắt (đèn xanh đang sáng)
//   digitalWrite(redPin, LOW);
//   digitalWrite(yellowPin, LOW);
//   digitalWrite(greenPin, HIGH);
//   delay(3000); // Chờ 3 giây

//   // Đèn vàng sáng, đèn đỏ và đèn xanh tắt (đèn vàng đang sáng)
//   digitalWrite(redPin, LOW);
//   digitalWrite(yellowPin, HIGH);
//   digitalWrite(greenPin, LOW);
//   delay(1000); // Chờ 1 giây

//   digitalWrite(redPin, HIGH);
//   digitalWrite(yellowPin, LOW);
//   digitalWrite(greenPin, LOW);
//   // delay(5000); // Chờ 3 giây

//   Serial.print("digitalRead(redPin)");
//   Serial.println(digitalRead(redPin));
//   if (distance < 5000 && digitalRead(redPin) == 1) {
                                                                                                                                                                                                                                                                              
//     sendPhotoToServer();
//   }
//   delay(5000); // Chờ 3 giây
//     // delay(1000);
//   // if (currentMillis - previousMillis >= Interval) {
//   //   previousMillis = currentMillis;
    
//   //   sendPhotoToServer();
//   // }
//   //---------------------------------------- 
// }

void loop() {
  // Đo khoảng cách từ cảm biến siêu âm
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 34 / 2;  // Đổi thời gian thành khoảng cách (đơn vị mm)
  Serial.print("Distance Measured = ");
  Serial.print(distance);
  Serial.println("mm");

  // Kiểm tra trạng thái của đèn đỏ
  bool isRedLightOn = digitalRead(redPin) == HIGH;

  // Nếu đèn đỏ sáng và khoảng cách nhỏ hơn 5000mm (5m), gửi ảnh lên server
  if (isRedLightOn && distance < 5000) {
    sendPhotoToServer();
  }

  // Kiểm tra khoảng cách và thay đổi màu đèn giao thông tương ứng
  if (isRedLightOn) {
    // Đèn đỏ sáng, đèn vàng và đèn xanh tắt (đèn đỏ đang sáng)
    digitalWrite(redPin, HIGH);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, LOW);
  } else {
    // Đèn xanh sáng, đèn đỏ và đèn vàng tắt (đèn xanh đang sáng)
    digitalWrite(redPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, HIGH);
    delay(3000); // Chờ 3 giây

    // Đèn vàng sáng, đèn đỏ và đèn xanh tắt (đèn vàng đang sáng)
    digitalWrite(redPin, LOW);
    digitalWrite(yellowPin, HIGH);
    digitalWrite(greenPin, LOW);
    delay(1000); // Chờ 1 giây

    // Đèn đỏ sáng, đèn vàng và đèn xanh tắt (đèn đỏ đang sáng)
    digitalWrite(redPin, HIGH);
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, LOW);
  }

  delay(5000); // Chờ 5 giây trước khi lặp lại quy trình
}

