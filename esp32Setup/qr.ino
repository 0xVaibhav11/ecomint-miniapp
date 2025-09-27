#include <SPI.h>
#include <TFT_eSPI.h>
#include <qrcode_espi.h>

String qr_content = "hello world";  // Your string content

TFT_eSPI tft = TFT_eSPI();
QRcode_eSPI qrcode(&tft);

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(2);
  qrcode.init();
  
  Serial.println("QR Content:");
  Serial.println(qr_content);
  
  qrcode.create(qr_content);
  
  
  
}

void loop() {}