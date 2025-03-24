#include "esp_camera.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define CAMERA_MODEL_XIAO_ESP32S3
#include "camera_pins.h"

#define LED_BUILTIN 21

unsigned long lastCaptureTime = 0;
int imageCount = 1;
bool camera_sign = false;
bool sd_sign = false;

void photo_save(const char * fileName) {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Error al capturar imagen");
    return;
  }

  writeFile(SD, fileName, fb->buf, fb->len);
  esp_camera_fb_return(fb);

  Serial.println("✅ Foto guardada");
}

void writeFile(fs::FS &fs, const char * path, uint8_t * data, size_t len){
    Serial.printf("💾 Guardando archivo: %s\r\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("❌ Error al abrir archivo");
        return;
    }
    if(file.write(data, len) == len){
        Serial.println("📁 Archivo escrito correctamente");
    } else {
        Serial.println("⚠️ Error al escribir");
    }
    file.close();
}

void setup() {
  Serial.begin(115200);
//  while(!Serial);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // LED apagado por defecto

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
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  if(psramFound()){
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Error al iniciar cámara: 0x%x", err);
    return;
  }
  
  camera_sign = true;

  if(!SD.begin(21)){
    Serial.println("❌ Error al montar la SD");
    return;
  }
  if(SD.cardType() == CARD_NONE){
    Serial.println("❌ No se detectó tarjeta SD");
    return;
  }

  sd_sign = true;

  Serial.println("✅ Todo listo. Se tomará la primera foto en 10 segundos...");
}

void loop() {
  if(camera_sign && sd_sign){
    unsigned long now = millis();
    if ((now - lastCaptureTime) >= 10000) {
      digitalWrite(LED_BUILTIN, LOW); // 🔆 Encender LED al iniciar foto

      char filename[32];
      sprintf(filename, "/image%d.jpg", imageCount);
      photo_save(filename);
      Serial.printf("📸 Foto guardada: %s\r\n", filename);

      digitalWrite(LED_BUILTIN, HIGH);  // 💡 Apagar LED después de guardar

      imageCount++;
      lastCaptureTime = now;
      Serial.println("⏳ Esperando 10 segundos para la siguiente foto...");
    }
  }
}
