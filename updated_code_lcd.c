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
char incomingPacket[1024]; 

// 4. Scrolling Text Variables
String scrollText = "";
bool isScrolling = false;
int scrollPos = 0;
unsigned long lastScroll = 0;
const int scrollSpeed = 300; 

// 5. Animation Engine Variables (NEW)
bool isAnimating = false;
int animFrame = 0;
unsigned long lastAnimTime = 0;

// Declare animation functions so the compiler knows they exist
void image00(); void image01(); void image02(); void image03();
void image04(); void image05(); void image06(); void image07();

void setup() {
  Serial.begin(115200);

  pinMode(pinRed, OUTPUT);
  pinMode(pinBlue, OUTPUT);
  pinMode(pinGreen, OUTPUT);

  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Booting Jarvis..");

  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nSUCCESS! Connected.");
  udp.begin(localUdpPort);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready! IP:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP()); 

  digitalWrite(pinRed, LOW); digitalWrite(pinBlue, LOW); digitalWrite(pinGreen, HIGH);
}

void loop() {
  // 1. Listen for incoming packets (Non-blocking)
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 1024);
    if (len > 0) incomingPacket[len] = 0; 
    
    String msg = String(incomingPacket);

    // --- THE HARDWARE ROUTER ---
    if (msg.startsWith("LED:RED")) {
      digitalWrite(pinRed, HIGH); digitalWrite(pinBlue, LOW); digitalWrite(pinGreen, LOW);
    } 
    else if (msg.startsWith("LED:BLUE")) {
      digitalWrite(pinRed, LOW); digitalWrite(pinBlue, HIGH); digitalWrite(pinGreen, LOW);
    } 
    else if (msg.startsWith("LED:GREEN")) {
      digitalWrite(pinRed, LOW); digitalWrite(pinBlue, LOW); digitalWrite(pinGreen, HIGH);
    }
    // --- TRIGGER ANIMATION ---
    else if (msg.startsWith("ANIM:BUTTERFLY")) {
      isScrolling = false;
      isAnimating = true;
      animFrame = 0;
      lastAnimTime = millis();
      image00(); // Show the first frame immediately
    }
    // --- TRIGGER TEXT ---
    else if (msg.startsWith("LCD:")) {
      isAnimating = false; // Kill the animation instantly if text arrives!
      String lcdText = msg.substring(4);
      lcd.clear();
      
      if (lcdText.length() <= 16) {
        isScrolling = false;
        lcd.setCursor(0, 0);
        lcd.print(lcdText);
      } 
      else if (lcdText.length() <= 32) {
        isScrolling = false;
        lcd.setCursor(0, 0);
        lcd.print(lcdText.substring(0, 16));
        lcd.setCursor(0, 1);
        lcd.print(lcdText.substring(16));
      } 
      else {
        isScrolling = true;
        scrollPos = 0;
        scrollText = lcdText + "                "; 
      }
    }
  }

  // 2. The Scrolling Engine (Runs continuously in the background)
  if (isScrolling) {
    if (millis() - lastScroll > scrollSpeed) {
      lastScroll = millis();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("JARVIS SAYS:"); 
      lcd.setCursor(0, 1);
      lcd.print(scrollText.substring(scrollPos, scrollPos + 16));
      scrollPos++;
      if (scrollPos > scrollText.length() - 16) {
        scrollPos = 0;
      }
    }
  }

  // 3. The Animation Engine (Non-blocking, runs via millis)
  if (isAnimating) {
    unsigned long currentMillis = millis();
    
    if (animFrame == 0 && currentMillis - lastAnimTime > 250) {
      image01(); animFrame = 1; lastAnimTime = currentMillis;
    } 
    else if (animFrame == 1 && currentMillis - lastAnimTime > 250) {
      image02(); animFrame = 2; lastAnimTime = currentMillis;
    } 
    else if (animFrame == 2 && currentMillis - lastAnimTime > 250) {
      image03(); animFrame = 3; lastAnimTime = currentMillis;
    } 
    else if (animFrame == 3 && currentMillis - lastAnimTime > 700) {
      image04(); animFrame = 4; lastAnimTime = currentMillis;
    } 
    else if (animFrame == 4 && currentMillis - lastAnimTime > 250) {
      image05(); animFrame = 5; lastAnimTime = currentMillis;
    } 
    else if (animFrame == 5 && currentMillis - lastAnimTime > 250) {
      image06(); animFrame = 6; lastAnimTime = currentMillis;
    } 
    else if (animFrame == 6 && currentMillis - lastAnimTime > 700) {
      image07(); animFrame = 7; lastAnimTime = currentMillis;
    } 
    else if (animFrame == 7 && currentMillis - lastAnimTime > 1250) {
      image00(); animFrame = 0; lastAnimTime = currentMillis; // Loop back to start
    }
  }
}

// ==========================================
// BUTTERFLY ANIMATION FRAMES
// ==========================================

void image00() {
    lcd.clear();
    byte image22[8] = {B00110, B01101, B11011, B10011, B00111,   B01111, B01111, B11111};
    byte image23[8] = {B01111, B11110, B11100, B11000,   B11000, B10000, B10000, B00000};
    byte image07[8] = {B00000, B00000, B00000,   B00000, B00000, B00000, B00001, B00111};
    byte image08[8] = {B00000, B01000,   B10000, B10000, B10000, B11111, B11111, B11000};
    byte image09[8] = {B00000,   B00000, B00000, B00000, B00000, B11000, B11000, B00100};
    lcd.createChar(0, image22);
    lcd.createChar(1, image23);
    lcd.createChar(2, image07);
    lcd.createChar(3, image08);
    lcd.createChar(4, image09);
    lcd.setCursor(5, 1); lcd.write(byte(0));
    lcd.setCursor(6, 1); lcd.write(byte(1));
    lcd.setCursor(6, 0); lcd.write(byte(2));
    lcd.setCursor(7, 0); lcd.write(byte(3));
    lcd.setCursor(8, 0); lcd.write(byte(4));
}

void image01() {
    lcd.clear();
    byte image22[8] = {B00110, B00101,   B00011, B00011, B00111, B01111, B01111, B11111};
    byte image23[8] = {B01111,   B11110, B11100, B11000, B11000, B10000, B10000, B00000};
    byte image07[8]   = {B00000, B00000, B00000, B00000, B00000, B00000, B11001, B10111};
    byte   image08[8] = {B00000, B01000, B10000, B10000, B10000, B11111, B11111, B11000};
    byte image09[8] = {B00000, B00000, B00000, B00000, B00000, B11000, B11000, B00100};
    byte image06[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00000, B00011};
    lcd.createChar(0, image22);
    lcd.createChar(1, image23);
    lcd.createChar(2, image07);
    lcd.createChar(3, image08);
    lcd.createChar(4, image09);
    lcd.createChar(5, image06);
    lcd.setCursor(5, 1); lcd.write(byte(0));
    lcd.setCursor(6, 1); lcd.write(byte(1));
    lcd.setCursor(6, 0); lcd.write(byte(2));
    lcd.setCursor(7, 0); lcd.write(byte(3));
    lcd.setCursor(8, 0); lcd.write(byte(4));
    lcd.setCursor(5, 0); lcd.write(byte(5));
}

void image02() {
    lcd.clear();
    byte image22[8] = {B00000, B00001, B00011, B00011, B00111, B01111, B01111, B11111};
    byte image23[8] = {B01111, B11110, B11100, B11000, B11000, B10000, B10000, B00000};
    byte image07[8] = {B00000, B00000, B00000, B00001, B00111, B00100, B11001, B10111};
    byte image08[8] = {B00000, B01000, B10000, B10000, B10000, B11111, B11111, B11000};
    byte image09[8] = {B00000, B00000, B00000, B00000, B00000, B11000, B11000, B00100};
    lcd.createChar(0, image22);
    lcd.createChar(1, image23);
    lcd.createChar(2, image07);
    lcd.createChar(3, image08);
    lcd.createChar(4, image09);
    lcd.setCursor(5, 1); lcd.write(byte(0));
    lcd.setCursor(6, 1); lcd.write(byte(1));
    lcd.setCursor(6, 0); lcd.write(byte(2));
    lcd.setCursor(7, 0); lcd.write(byte(3));
    lcd.setCursor(8, 0); lcd.write(byte(4));
}

void image03() {
    lcd.clear();
    byte image22[8] = {B00000, B00001, B00011, B00011, B00111, B01111, B01111, B11111};
    byte image23[8] = {B01111, B11110, B11100, B11000, B11000, B10000, B10000, B00000};
    byte image07[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00001, B00111};
    byte image08[8] = {B00000, B01000, B10000, B10000, B10000, B11111, B11111, B11010};
    byte image09[8] = {B00000, B00000, B00000, B00000, B00000, B11000, B11000, B00100};
    byte image24[8] = {B00010, B00111, B00111, B00111, B00111, B00111, B00010, B00000};
    lcd.createChar(0, image22);
    lcd.createChar(1, image23);
    lcd.createChar(2, image07);
    lcd.createChar(3, image08);
    lcd.createChar(4, image09);
    lcd.createChar(5, image24);
    lcd.setCursor(5, 1); lcd.write(byte(0));
    lcd.setCursor(6, 1); lcd.write(byte(1));
    lcd.setCursor(6, 0); lcd.write(byte(2));
    lcd.setCursor(7, 0); lcd.write(byte(3));
    lcd.setCursor(8, 0); lcd.write(byte(4));
    lcd.setCursor(7, 1); lcd.write(byte(5));
}

void image04() {
    lcd.clear();
    byte image22[8] = {B00000, B00001, B00011, B00011, B00111, B01111, B01111, B11111};
    byte image23[8] = {B01111, B11110, B11100, B11000, B11000, B10001, B10000, B00000};
    byte image07[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00001, B00111};
    byte image08[8] = {B00000, B01000, B10000, B10000, B10000, B11111, B11111, B11010};
    byte image09[8] = {B00000, B00000, B00000, B00000, B00000, B11000, B11000, B00100};
    byte image24[8] = {B00010, B00100, B01011, B10101, B11010, B10101, B11010, B01110};
    byte image25[8] = {B00000, B00000, B00000, B10000, B10000, B00000, B00000, B00000};
    lcd.createChar(0, image22);
    lcd.createChar(1, image23);
    lcd.createChar(2, image07);
    lcd.createChar(3, image08);
    lcd.createChar(4, image09);
    lcd.createChar(5, image24);
    lcd.createChar(6, image25);
    lcd.setCursor(5, 1); lcd.write(byte(0));
    lcd.setCursor(6, 1); lcd.write(byte(1));
    lcd.setCursor(6, 0); lcd.write(byte(2));
    lcd.setCursor(7, 0); lcd.write(byte(3));
    lcd.setCursor(8, 0); lcd.write(byte(4));
    lcd.setCursor(7, 1); lcd.write(byte(5));
    lcd.setCursor(8, 1); lcd.write(byte(6));
}

void image05() {
    lcd.clear();
    byte image24[8] = {B01010, B10100, B01011, B10101, B11010, B10101, B11010, B01110};
    byte image25[8] = {B00000, B00000, B00000, B10000, B10000, B00000, B00000, B00000};
    byte image23[8] = {B01101, B01010, B01101, B00111, B00000, B00000, B00000, B00000};
    byte image07[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00001, B00011};
    byte image08[8] = {B00000, B00000, B00000, B00000, B00000, B00000, B00000, B10000};
    lcd.createChar(0, image24);
    lcd.createChar(1, image25);
    lcd.createChar(2, image23);
    lcd.createChar(3, image07);
    lcd.createChar(4, image08);
    lcd.setCursor(7, 1); lcd.write(byte(0));
    lcd.setCursor(8, 1); lcd.write(byte(1));
    lcd.setCursor(6, 1); lcd.write(byte(2));
    lcd.setCursor(6, 0); lcd.write(byte(3));
    lcd.setCursor(7, 0); lcd.write(byte(4));
}

void image06() {
    lcd.clear();
    byte image08[8] = {B00000, B00100, B01010, B01010, B10001, B00011, B00110, B01100};
    byte image07[8] = {B00000, B00000, B00000, B00000, B00001, B00010, B00010, B00001};
    byte image09[8] = {B00000, B00000, B10000, B11000, B00000, B00000, B11000, B00100};
    byte image24[8] = {B00100, B00100, B00011, B00000, B00000, B00000, B00000, B00000};
    byte image25[8] = {B10000, B00000, B00000, B00000, B00000, B00000, B00000, B00000};
    lcd.createChar(0, image08);
    lcd.createChar(1, image07);
    lcd.createChar(2, image09);
    lcd.createChar(3, image24);
    lcd.createChar(4, image25);
    lcd.setCursor(7, 0); lcd.write(byte(0));
    lcd.setCursor(6, 0); lcd.write(byte(1));
    lcd.setCursor(8, 0); lcd.write(byte(2));
    lcd.setCursor(7, 1); lcd.write(byte(3));
    lcd.setCursor(8, 1); lcd.write(byte(4));
}

void image07() {
    lcd.clear();
    byte image24[8] = {B10101, B01110, B01110, B00100, B10101, B01110, B00100, B11111};
    byte image08[8] = {B00000, B00100, B01010, B01010, B10001, B00011, B00110, B01100};
    byte image07[8] = {B00000, B00000, B00000, B00000, B00001, B00010, B00010, B00001};
    byte image09[8] = {B00000, B00000, B10000, B11000, B00000, B00000, B00000, B00000};
    lcd.createChar(0, image24);
    lcd.createChar(1, image08);
    lcd.createChar(2, image07);
    lcd.createChar(3, image09);
    lcd.setCursor(7, 1); lcd.write(byte(0));
    lcd.setCursor(7, 0); lcd.write(byte(1));
    lcd.setCursor(6, 0); lcd.write(byte(2));
    lcd.setCursor(8, 0); lcd.write(byte(3));
}