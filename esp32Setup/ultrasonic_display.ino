#include <SPI.h>
#include <TFT_eSPI.h>

// HC-SR04 pins
const int trigPin = 27;  // TX/SDA
const int echoPin = 26;  // RX/SCL

TFT_eSPI tft = TFT_eSPI();

// Variables for distance measurement
long duration;
int distance;
int lastDistance = -1;

void setup() {
  Serial.begin(115200);
  
  // Initialize display
  tft.init();
  tft.setRotation(2);
  
  // Initialize HC-SR04 pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Clear screen and display initial message
  tft.fillScreen(TFT_BLACK);
  displayHeader();
  
  Serial.println("Ultrasonic sensor initialized!");
}

void loop() {
  // Measure distance
  distance = measureDistance();
  
  // Only update display if distance changed significantly
  if (abs(distance - lastDistance) > 1) {
    displayDistance(distance);
    lastDistance = distance;
  }
  
  // Print to serial
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  delay(100); // Small delay between measurements
}

int measureDistance() {
  // Clear the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Set the trigPin HIGH for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distance = duration * 0.034 / 2;
  
  // Limit distance to reasonable range
  if (distance > 400) distance = 400;
  if (distance < 0) distance = 0;
  
  return distance;
}

void displayHeader() {
  // Title
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.println("HC-SR04 Sensor");
  
  // Subtitle
  tft.setTextColor(TFT_YELLOW);
  tft.setTextSize(1);
  tft.setCursor(20, 50);
  tft.println("Distance Monitor");
  
  // Draw separator line
  tft.drawLine(20, 70, 200, 70, TFT_WHITE);
}

void displayDistance(int dist) {
  // Clear previous readings
  tft.fillRect(20, 90, 180, 80, TFT_BLACK);
  
  // Display distance value
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 90);
  tft.print("Distance: ");
  tft.print(dist);
  tft.println(" cm");
  
  // Simple detection: within 30cm or not
  tft.setTextSize(3);
  tft.setCursor(20, 120);
  
  if (dist <= 30) {
    // Something detected within 30cm
    tft.setTextColor(TFT_RED);
    tft.println("SOMETHING");
    tft.setTextColor(TFT_ORANGE);
    tft.setCursor(20, 150);
    tft.println("DETECTED!");
    
    // Draw warning circle
    tft.drawCircle(120, 180, 25, TFT_RED);
    tft.fillCircle(120, 180, 20, TFT_RED);
  } else {
    // Nothing detected within 30cm
    tft.setTextColor(TFT_GREEN);
    tft.println("CLEAR");
    tft.setTextColor(TFT_BLUE);
    tft.setCursor(20, 150);
    tft.println("AREA");
    
    // Draw clear circle
    tft.drawCircle(120, 180, 25, TFT_GREEN);
    tft.fillCircle(120, 180, 20, TFT_GREEN);
  }
}
