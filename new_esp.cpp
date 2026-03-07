#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// 1. Hardware Pins
const int pinRed = 25;
const int pinBlue = 26;
const int pinGreen = 27;

// 2. LCD Setup
LiquidCrystal_I2C lcd(0x27, 16, 2); 

// 3. Network Setup
const char* ssid = "GreatBhardwaj";
const char* password = "88888888";
WiFiUDP udp;
unsigned int localUdpPort = 1234;
char incomingPacket[255]; // We don't need a massive buffer anymore

// 4. Telemetry State Variables
String valTemp = "--C";
String valCPU = "--%";
String valRAM = "--%";
String ipEnding = "";
bool isAlert = false;

// 5. Heartbeat Variables
unsigned long lastHeartbeatTime = 0;
bool greenLedState = false;

void updateLCD() {
  lcd.clear();
  
  // Row 0: Temp & CPU (Example: "Temp:45C C:12%")
  lcd.setCursor(0, 0);
  lcd.print("Temp:"); lcd.print(valTemp);
  lcd.setCursor(9, 0);
  lcd.print("C:"); lcd.print(valCPU);

  // Row 1: RAM & IP (Example: "RAM:45% IP:.95")
  lcd.setCursor(0, 1);
  lcd.print("RAM:"); lcd.print(valRAM);
  lcd.setCursor(8, 1);
  lcd.print("IP:"); lcd.print(ipEnding);
}

void setup() {
  Serial.begin(115200);

  pinMode(pinRed, OUTPUT);
  pinMode(pinBlue, OUTPUT);
  pinMode(pinGreen, OUTPUT);

  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting Wi-Fi");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  udp.begin(localUdpPort);
  
  // Extract just the ".95" part of "10.42.0.95" to save LCD space
  String fullIP = WiFi.localIP().toString();
  int lastDot = fullIP.lastIndexOf('.');
  ipEnding = fullIP.substring(lastDot); 
  
  updateLCD();
}

void loop() {
  // 1. Check Wi-Fi & Blue LED
  bool wifiConnected = (WiFi.status() == WL_CONNECTED);
  digitalWrite(pinBlue, wifiConnected ? HIGH : LOW);

  // 2. The LED State Machine
  if (isAlert) {
    digitalWrite(pinRed, HIGH); // Danger!
    digitalWrite(pinGreen, LOW); // Kill heartbeat
  } else {
    digitalWrite(pinRed, LOW); // Safe.
    
    // Run Green Heartbeat only if safe and connected
    if (wifiConnected) {
      if (millis() - lastHeartbeatTime > 1000) {
        greenLedState = !greenLedState;
        digitalWrite(pinGreen, greenLedState ? HIGH : LOW);
        lastHeartbeatTime = millis();
      }
    } else {
      digitalWrite(pinGreen, LOW);
    }
  }

  // 3. Listen for Telemetry from the Raspberry Pi
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) incomingPacket[len] = 0; 
    String msg = String(incomingPacket);

    // Parse Alert States
    if (msg.startsWith("STATE:ALERT")) {
      isAlert = true;
    } 
    else if (msg.startsWith("STATE:NOMINAL")) {
      isAlert = false;
    } 
    // Parse Hardware Data. Format expected: "UPDATE:45,12,45"
    else if (msg.startsWith("UPDATE:")) {
      int firstComma = msg.indexOf(',');
      int secondComma = msg.indexOf(',', firstComma + 1);
      
      if (firstComma > 0 && secondComma > 0) {
        valTemp = msg.substring(7, firstComma) + "C";
        valCPU = msg.substring(firstComma + 1, secondComma) + "%";
        valRAM = msg.substring(secondComma + 1) + "%";
        
        updateLCD(); // Only refresh screen when new data arrives
      }
    }
  }
}