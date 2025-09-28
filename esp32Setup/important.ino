#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <qrcode_espi.h>

// IR Sensor pins
const int irAnalogPin = 34;  // ADC1_CH6 (GPIO 34)
const int irDigitalPin = 35; // ADC1_CH7 (GPIO 35)

// Display
TFT_eSPI tft = TFT_eSPI();
QRcode_eSPI qrcode(&tft);

// WiFi credentials
const char* ssid = "honey";
const char* password = "honey@123";

// API endpoint
const char* apiUrl = "https://testingserverethindia.onrender.com/hash";

// System states
enum SystemState {
  LANDING_PAGE,
  COUNTING_BOTTLES,
  COUNTDOWN,
  GENERATING_QR,
  SHOWING_QR,
  RETURNING_HOME
};

SystemState currentState = LANDING_PAGE;

// System variables
int bottleCount = 0;
unsigned long lastBottleTime = 0;
unsigned long countdownStartTime = 0;
unsigned long qrDisplayStartTime = 0;
bool isCounting = false;
String currentHash = "";
String currentBottleNumber = "";

// Timing constants
const unsigned long COUNTDOWN_TIME = 10000; // 10 seconds
const unsigned long QR_DISPLAY_TIME = 5000; // 5 seconds
const unsigned long DEBOUNCE_TIME = 500; // 500ms debounce
const unsigned long ANIMATION_DELAY = 50; // Animation frame delay

// Object detection variables
int lastAnalogValue = 0;
bool bottlePresent = false;
bool lastBottlePresent = false;
unsigned long lastDetectionTime = 0;

// Animation variables
unsigned long lastAnimationTime = 0;
int animationFrame = 0;
bool animationDirection = true; // true = fade in, false = fade out

// Function declarations
void showLandingPage();
void showLandingPageAnimated();
void showCountingScreen();
void showCountdownScreen();
void showQRScreen();
void returnToHome();
void displayQRCode(String hash);
void connectToWiFi();
bool callHashAPI(String bottleNumber);
void generateQRCode(String hash);

bool isBottleDetected(int analogValue) {
  return (analogValue > 400 || analogValue < 100);
}

void onBottleDetected() {
  bottleCount++;
  lastBottleTime = millis();
  isCounting = true;
  
  Serial.print("Bottle detected! Count: ");
  Serial.println(bottleCount);
  
  // Switch to counting state
  currentState = COUNTING_BOTTLES;
}

void startCountdown() {
  countdownStartTime = millis();
  currentState = COUNTDOWN;
  Serial.println("Starting 10-second countdown...");
}

void handleCountdown() {
  unsigned long elapsed = millis() - countdownStartTime;
  
  if (elapsed >= COUNTDOWN_TIME) {
    // Countdown finished, start QR generation
    currentState = GENERATING_QR;
    Serial.println("Countdown finished. Generating QR code...");
  }
}

void handleQRGeneration() {
  Serial.println("=== Starting QR Generation ===");
  
  if (bottleCount == 0) {
    Serial.println("No bottles detected. Returning to home.");
    currentState = RETURNING_HOME;
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    currentBottleNumber = String(bottleCount) + "_" + String(millis());
    Serial.print("Bottle ID: ");
    Serial.println(currentBottleNumber);
    
    if (callHashAPI(currentBottleNumber)) {
      generateQRCode(currentHash);
      currentState = SHOWING_QR;
      qrDisplayStartTime = millis();
    } else {
      Serial.println("API call failed. Returning to home.");
      currentState = RETURNING_HOME;
    }
  } else {
    Serial.println("WiFi not connected. Returning to home.");
    currentState = RETURNING_HOME;
  }
}

void handleQRDisplay() {
  unsigned long elapsed = millis() - qrDisplayStartTime;
  
  if (elapsed >= QR_DISPLAY_TIME) {
    // QR display time finished, return to home
    currentState = RETURNING_HOME;
    Serial.println("QR display time finished. Returning to home.");
  }
}

void handleReturnHome() {
  // Reset all variables
  bottleCount = 0;
  currentHash = "";
  currentBottleNumber = "";
  isCounting = false;
  
  // Return to landing page
  currentState = LANDING_PAGE;
  Serial.println("Returned to home. Ready for new cycle.");
}

bool callHashAPI(String bottleNumber) {
  HTTPClient http;
  http.begin(apiUrl);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("ngrok-skip-browser-warning", "true");
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(15000);

  String jsonPayload = "{\"bottleNumber\":\"" + bottleNumber + "\"}";
  Serial.print("Sending JSON: ");
  Serial.println(jsonPayload);

  int httpResponseCode = http.POST(jsonPayload);
  Serial.print("HTTP Code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Response: ");
    Serial.println(response);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print("JSON Parse Error: ");
      Serial.println(error.c_str());
      http.end();
      return false;
    }

    if (doc.containsKey("hash")) {
      currentHash = doc["hash"].as<String>();
      Serial.print("Hash received: ");
      Serial.println(currentHash);
      http.end();
      return true;
    } else {
      Serial.println("Hash key not found.");
    }
  } else {
    Serial.print("HTTP Error: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return false;
}

void generateQRCode(String hash) {
  displayQRCode(hash);
}

void displayQRCode(String hash) {
  tft.fillScreen(TFT_BLACK);

  // Title
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.println("QR Code Generated");

  // Bottle Info
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(20, 50);
  tft.print("Bottles: ");
  tft.println(bottleCount);

  // Hash (shortened)
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(20, 70);
  tft.print("Hash: ");
  tft.println(hash.substring(0, 16) + "...");

  // QR Code
  qrcode.create(hash);

  // Footer
  tft.setTextColor(TFT_GREEN);
  tft.setCursor(20, 200);
  tft.println("QR code generated!");
}

void showLandingPageAnimated() {
  tft.fillScreen(TFT_BLACK);
  
  // Animated title with fade effect
  if (millis() - lastAnimationTime > ANIMATION_DELAY) {
    animationFrame += animationDirection ? 1 : -1;
    
    if (animationFrame >= 255) {
      animationDirection = false;
    } else if (animationFrame <= 0) {
      animationDirection = true;
    }
    
    lastAnimationTime = millis();
  }
  
  // Product name with animated color
  uint16_t animatedColor = tft.color565(animationFrame, 255 - animationFrame, 255);
  tft.setTextColor(animatedColor);
  tft.setTextSize(3);
  tft.setCursor(40, 60);
  tft.println("EcoMint");
  
  // Thank you message with pulsing effect
  uint16_t pulseColor = tft.color565(255, animationFrame, 0);
  tft.setTextColor(pulseColor);
  tft.setTextSize(2);
  tft.setCursor(30, 100);
  tft.println("Thank You!");
  
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(20, 130);
  tft.println("Help Nature Nourish");
  
  // Animated border
  for (int i = 0; i < 4; i++) {
    uint16_t borderColor = tft.color565(0, animationFrame, 255 - animationFrame);
    tft.drawRect(10 + i, 10 + i, 300 - 2*i, 220 - 2*i, borderColor);
  }
  
  // Instructions with fade
  uint16_t fadeColor = tft.color565(animationFrame, animationFrame, animationFrame);
  tft.setTextColor(fadeColor);
  tft.setCursor(20, 180);
  tft.println("Place bottles to start counting");
}

void showLandingPage() {
  tft.fillScreen(TFT_BLACK);
  
  // Product name
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(3);
  tft.setCursor(40, 60);
  tft.println("EcoMint");
  
  // Thank you message
  tft.setTextColor(TFT_GREEN);
  tft.setTextSize(2);
  tft.setCursor(30, 100);
  tft.println("Thank You!");
  
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(20, 130);
  tft.println("Help Nature Nourish");
  
  // Instructions
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(20, 180);
  tft.println("Place bottles to start counting");
}

void showCountingScreen() {
  // Clear screen
  tft.fillScreen(TFT_BLACK);
  
  // Show bottle count in center
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(4);
  tft.setCursor(120, 80);
  tft.println(bottleCount);
  
  // Label
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(100, 120);
  tft.println("Bottles");
  
  // Show countdown in bottom right corner
  if (currentState == COUNTDOWN) {
    unsigned long elapsed = millis() - countdownStartTime;
    unsigned long remaining = (COUNTDOWN_TIME - elapsed) / 1000;
    
    if (remaining > 0) {
      tft.setTextColor(TFT_RED);
      tft.setTextSize(2);
      tft.setCursor(250, 200);
      tft.println(remaining);
    }
  }
}

void showCountdownScreen() {
  showCountingScreen(); // Reuse counting screen with countdown
}

void showQRScreen() {
  // QR code is already displayed by displayQRCode function
  // Just add any additional UI elements if needed
}

void returnToHome() {
  // This will be handled by handleReturnHome()
  showLandingPageAnimated(); // Use animated version
}

void connectToWiFi() {
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(20, 100);
  tft.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    tft.setTextColor(TFT_GREEN);
    tft.setCursor(20, 120);
    tft.println("WiFi Connected!");
  } else {
    Serial.println("WiFi failed.");
    tft.setTextColor(TFT_RED);
    tft.setCursor(20, 120);
    tft.println("WiFi Failed!");
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(2);
  qrcode.init();
  tft.fillScreen(TFT_BLACK);
  
  // Initialize IR sensor pins
  pinMode(irAnalogPin, INPUT);
  pinMode(irDigitalPin, INPUT);
  
  // Connect to WiFi
  connectToWiFi();
  
  // Show landing page
  showLandingPageAnimated();
  
  Serial.println("EcoMint Bottle Counter initialized!");
}

void loop() {
  // Read sensor
  int analogValue = analogRead(irAnalogPin);
  int digitalValue = digitalRead(irDigitalPin);
  
  // Detect bottle
  bool currentBottlePresent = isBottleDetected(analogValue);

  // Update for display
  bottlePresent = currentBottlePresent;

  // Debounced detection
  if (currentBottlePresent && !lastBottlePresent) {
    if (millis() - lastDetectionTime > DEBOUNCE_TIME) {
      onBottleDetected();
      lastDetectionTime = millis();
    }
  }

  lastBottlePresent = currentBottlePresent;
  lastAnalogValue = analogValue;

  // Handle different states
  switch (currentState) {
    case LANDING_PAGE:
      showLandingPageAnimated();
      break;
      
    case COUNTING_BOTTLES:
      showCountingScreen();
      // Start countdown if no bottles detected for a while
      if (millis() - lastBottleTime > 2000) { // 2 seconds after last bottle
        startCountdown();
      }
      break;
      
    case COUNTDOWN:
      handleCountdown();
      showCountdownScreen();
      break;
      
    case GENERATING_QR:
      handleQRGeneration();
      break;
      
    case SHOWING_QR:
      handleQRDisplay();
      break;
      
    case RETURNING_HOME:
      handleReturnHome();
      break;
  }

  delay(100);
}
