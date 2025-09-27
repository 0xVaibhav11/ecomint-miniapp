#include <SPI.h>
#include <TFT_eSPI.h>

// Waveshare IR Proximity Sensor (RS3208 LM393) pins
const int irAnalogPin = 34;  // ADC1_CH6 (GPIO 34) - Analog input for distance measurement
const int irDigitalPin = 35; // ADC1_CH7 (GPIO 35) - Digital output (optional, for threshold detection)

TFT_eSPI tft = TFT_eSPI();

// Variables for IR proximity sensor
int irAnalogValue;
int irDigitalValue;
int distance;
int lastDistance = -1;
int stableDistance = -1;
bool objectDetected = false;
bool lastObjectDetected = false;

// Calibration and filtering
const int NUM_READINGS = 5;  // Number of readings to average
const int DETECTION_THRESHOLD = 10;  // 10cm detection threshold
const int MIN_DISTANCE = 2;  // Minimum reliable distance in cm
const int MAX_DISTANCE = 50; // Maximum distance in cm

// CALIBRATED VALUES - Based on actual sensor testing
const int ANALOG_THRESHOLD = 100; // Analog threshold: >100 = no object, <100 = object detected

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(2);
  
  // Initialize IR sensor pins
  pinMode(irAnalogPin, INPUT);
  pinMode(irDigitalPin, INPUT);
  
  // Clear screen and display initial message
  tft.fillScreen(TFT_BLACK);
  displayHeader();
  
  Serial.println("Waveshare IR Proximity Sensor (RS3208 LM393) initialized!");
  Serial.println("Calibrating sensor...");
  
  // Simple calibration - read baseline values
  delay(1000);
  int calValue = analogRead(irAnalogPin);
  int calDigital = digitalRead(irDigitalPin);
  Serial.print("Calibration - Analog: ");
  Serial.print(calValue);
  Serial.print(", Digital: ");
  Serial.println(calDigital);
  
  // Display calibration info
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(20, 80);
  tft.print("Calibration: ");
  tft.print(calValue);
  tft.print(" / ");
  tft.println(calDigital);
}

void loop() {
  // Read sensor values
  irAnalogValue = readAnalogSensor();
  irDigitalValue = digitalRead(irDigitalPin);
  
  // Convert analog reading to distance
  distance = convertToDistance(irAnalogValue);
  
  // Apply stability filter
  if (stableDistance == -1 || abs(distance - stableDistance) > 2) {
    stableDistance = distance;
  }
  
  // Simple object detection using calibrated thresholds
  objectDetected = isObjectDetected(irAnalogValue);
  
  // Only update display if detection status changed
  if (objectDetected != lastObjectDetected || abs(stableDistance - lastDistance) > 2) {
    displayDetectionStatus(objectDetected, stableDistance);
    lastObjectDetected = objectDetected;
    lastDistance = stableDistance;
  }
  
  // Print to serial for debugging
  Serial.print("Analog: ");
  Serial.print(irAnalogValue);
  Serial.print(", Digital: ");
  Serial.print(irDigitalValue);
  Serial.print(", Distance: ");
  Serial.print(stableDistance);
  Serial.print(" cm, Object Detected: ");
  if (objectDetected) {
    if (irAnalogValue > 400) {
      Serial.println("YES (Transparent)");
    } else {
      Serial.println("YES (Non-transparent)");
    }
  } else {
    Serial.println("NO");
  }
  
  delay(200); // Short delay for responsive readings
}

int readAnalogSensor() {
  int readings[NUM_READINGS];
  int sum = 0;
  
  // Take multiple readings for stability
  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = analogRead(irAnalogPin);
    sum += readings[i];
    delay(5);
  }
  
  // Return average reading
  return sum / NUM_READINGS;
}

bool isObjectDetected(int analogValue) {
  // SIMPLE OBJECT DETECTION - Based on your calibration
  // analogValue > 400 = Transparent object detected
  // analogValue < 100 = Non-transparent object detected
  // 100 <= analogValue <= 400 = No object detected
  
  if (analogValue > 400) {
    return true;  // Transparent object detected
  } else if (analogValue < 100) {
    return true;  // Non-transparent object detected
  } else {
    return false; // No object detected (100-400 range)
  }
}

int convertToDistance(int analogValue) {
  // Simple distance estimation based on detection zones
  if (analogValue > 400) {
    return 5;  // Transparent object (estimated distance)
  } else if (analogValue < 100) {
    return 3;  // Non-transparent object (estimated distance)
  } else {
    return MAX_DISTANCE; // No object detected
  }
}

void displayHeader() {
  // Title
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.println("IR Proximity Sensor");
  
  // Subtitle
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(20, 50);
  tft.println("Waveshare RS3208 LM393");
  
  // Draw separator line
  tft.drawLine(20, 70, 200, 70, TFT_WHITE);
}

void displayDetectionStatus(bool detected, int dist) {
  // Clear previous readings
  tft.fillRect(20, 100, 180, 100, TFT_BLACK);
  
  // Display distance value
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.print("Distance: ");
  tft.print(dist);
  tft.println(" cm");
  
  // Display detection status
  tft.setTextSize(3);
  tft.setCursor(20, 130);
  
  if (detected) {
    // Object detected within 10cm
    tft.setTextColor(TFT_RED);
    tft.println("OBJECT");
    tft.setTextColor(TFT_ORANGE);
    tft.setCursor(20, 160);
    tft.println("DETECTED!");
    tft.setTextColor(TFT_YELLOW);
    tft.setTextSize(1);
    tft.setCursor(20, 185);
    tft.println("Within 10cm");
    
    // Draw warning circle
    tft.drawCircle(120, 200, 30, TFT_RED);
    tft.fillCircle(120, 200, 25, TFT_RED);
    
    // Add pulsing effect
    tft.drawCircle(120, 200, 35, TFT_ORANGE);
  } else {
    // No object detected within 10cm
    tft.setTextColor(TFT_GREEN);
    tft.println("CLEAR");
    tft.setTextColor(TFT_BLUE);
    tft.setCursor(20, 160);
    tft.println("AREA");
    tft.setTextColor(TFT_CYAN);
    tft.setTextSize(1);
    tft.setCursor(20, 185);
    tft.println("No object < 10cm");
    
    // Draw clear circle
    tft.drawCircle(120, 200, 30, TFT_GREEN);
    tft.fillCircle(120, 200, 25, TFT_GREEN);
  }
  
  // Display analog and digital values for debugging
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.setCursor(20, 220);
  tft.print("Analog: ");
  tft.print(irAnalogValue);
  tft.print(" | Digital: ");
  tft.println(irDigitalValue);
}

// Function to help with calibration
void calibrateSensor() {
  Serial.println("=== SENSOR CALIBRATION ===");
  Serial.println("Place objects at known distances and note the analog values:");
  Serial.println("1. No object (far): ");
  
  delay(3000);
  int farValue = analogRead(irAnalogPin);
  Serial.print("   Analog value: ");
  Serial.println(farValue);
  
  Serial.println("2. Object at 10cm: ");
  delay(3000);
  int midValue = analogRead(irAnalogPin);
  Serial.print("   Analog value: ");
  Serial.println(midValue);
  
  Serial.println("3. Object very close (2-3cm): ");
  delay(3000);
  int closeValue = analogRead(irAnalogPin);
  Serial.print("   Analog value: ");
  Serial.println(closeValue);
  
  Serial.println("Use these values to adjust the convertToDistance() function");
}
