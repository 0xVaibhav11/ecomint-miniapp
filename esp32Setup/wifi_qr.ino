#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <qrcode_espi.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// API endpoint
const char* api_url = "https://api.github.com/users/octocat";

TFT_eSPI tft = TFT_eSPI();
QRcode_eSPI qrcode(&tft);

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(2);
  qrcode.init();
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Make HTTPS request
  String response = makeHttpRequest();
  
  if (response.length() > 0) {
    Serial.println("API Response:");
    Serial.println(response);
    
    // Create QR code from response
    qrcode.create(response);

    qrcode.screenupdate();
    
    // Display status
    tft.setTextColor(TFT_GREEN);
    tft.setTextSize(1);
    tft.setCursor(10, 200);

  } else {
    Serial.println("Failed to get response");
    tft.setTextColor(TFT_RED);
    tft.setTextSize(1);
    tft.setCursor(10, 200);
    tft.println("API Request Failed");
  }
}

void loop() {
  // Nothing to do here
}

String makeHttpRequest() {
  HTTPClient http;
  String response = "";
  
  http.begin(api_url);
  http.addHeader("User-Agent", "ESP32");
  
  int httpResponseCode = http.GET();
  
  if (httpResponseCode > 0) {
    response = http.getString();
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
  return response;
}
