
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Setup.h"
#include "sendimage.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_camera.h>


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