#include <WiFi.h>
#include "base64.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <HTTPClient.h>
#include "esp_camera.h"
const char* ssid = "";
const char* password = "";
const char* serverUrl = "http://ip:5000/upload";

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

#define LED_GPIO_NUM      33

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector
  
  Serial.begin(115200);
  delay(100);
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
  config.frame_size = FRAMESIZE_VGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    while (1); // halt the program if camera initialization fails
  }
  camera_fb_t * fb = NULL;
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  
}

void sendImageToServer(uint8_t* data, size_t len) {
  // Encode the image data to Base64
  String base64Data = base64::encode(data, len);

  // Create the payload JSON data (if needed)
  String payload = "{\"image\":\"" + base64Data + "\"}";

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  // Make the HTTP POST request with the Base64-encoded image data in the payload
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.printf("Image sent successfully, HTTP Response code: %d\n", httpResponseCode);
  } else {
    Serial.printf("Image upload failed, HTTP Response code: %d\n", httpResponseCode);
  }

  // Close the HTTP connection
  http.end();
}




void loop() {
  // Take a photo
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Create a copy of the camera frame buffer
  uint8_t* dataCopy = (uint8_t*)malloc(fb->len);
  if (dataCopy == NULL) {
    Serial.println("Memory allocation failed");
    esp_camera_fb_return(fb);
    return;
  }
  memcpy(dataCopy, fb->buf, fb->len);

  // Free the camera frame buffer
  esp_camera_fb_return(fb);

  // Send the photo to the Flask server
  sendImageToServer(dataCopy, fb->len);

  // Free the memory for the copy
  free(dataCopy);

  // Wait for some time before taking another photo
  delay(60000);
}
