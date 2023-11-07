String serverName = "54.169.172.45";  //--> Change with your server computer's IP address or your Domain name.
String serverPath = "/api/storages/upload";
// Server Port.
const int serverPort = 3011;
camera_config_t camera_config;
void sendPhotoToServer()
{
  String boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
  String data = "";

  Serial.println("Connecting to server: " + serverName);

  if (client.connect(serverName.c_str(), serverPort))
  {
    Serial.println("Connection successful!");
    String fileName = "ESP32CAM.jpg";
    data = "--" + boundary + "\r\n";
    data += "Content-Disposition: form-data; name=\"file\"; filename=\"" + fileName + "\"\r\n";
    data += "Content-Type: image/jpeg\r\n\r\n";

    String request = "--" + boundary + "\r\n";
    request += "Content-Disposition: form-data; name=\"file\"; filename=\"" + fileName + "\"\r\n";
    request += "Content-Type: image/jpeg\r\n\r\n";

    uint32_t imageLen = 0;
    uint8_t *fbBuf = NULL;

    camera_fb_t *fb = esp_camera_fb_get();
    if (fb)
    {
      imageLen = fb->len;
      fbBuf = fb->buf;
    }

    uint32_t totalLen = data.length() + imageLen + String("\r\n--" + boundary + "--\r\n").length();

    client.println("POST " + serverPath + " HTTPS/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=" + boundary);
    client.println();
    client.print(request);

    if (fbBuf)
    {
      for (size_t n = 0; n < imageLen; n = n + 1024)
      {
        if (n + 1024 < imageLen)
        {
          client.write(fbBuf, 1024);
          fbBuf += 1024;
        }
        else if (imageLen % 1024 > 0)
        {
          size_t remainder = imageLen % 1024;
          client.write(fbBuf, remainder);
        }
      }
      esp_camera_fb_return(fb);
    }
    client.println("\r\n--" + boundary + "--\r\n");
    client.println();

    String response = "";
    while (client.connected() || client.available())
    {
      if (client.available())
      {
        char c = client.read();
        response += c;
      }
    }
    client.stop();
    Serial.println("Response: ");
    Serial.println(response);
    if (response.indexOf("Your success indicator") != -1) {
    isUploadSuccessful = true;
    Serial.println("Gửi ảnh thành công!");
  } else {
    isUploadSuccessful = false;
    Serial.println("Gửi ảnh không thành công.");
  }
  }
  else
  {
    client.stop();
    Serial.println("Connection to " + serverName + " failed.");
  }
}