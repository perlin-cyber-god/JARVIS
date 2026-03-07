# The Distributed Jarvis Master Blueprint

## 🏗️ Architecture Overview

| Component | Role |
|-----------|------|
| **The Brain** (Raspberry Pi 5) | Runs the C Orchestrator, local LLM (Ollama), TTS (Piper), executes terminal commands, and outputs audio |
| **The Face** (ESP32) | Physically wired to the 16x2 LCD and 3 LEDs; connects to Wi-Fi and listens for instructions from the Pi |

---

## 📋 System Phases

### Phase 0: Hardware & Network Diagnostics

Before writing the main system, we prove the components and the network can talk to each other.

- **Test 1: The ESP32 Hardware** — Wire the LEDs and LCD to the ESP32. Write a basic C++ sketch (via Arduino IDE or ESP-IDF) to blink the lights and print "ESP32 Online" to the screen.

- **Test 2: The Audio Output** — Test the Bluetooth pairing (bluetoothctl) or build the Transistor + Laptop speaker amplifier on the Pi. Play a simple .wav file to prove the Pi can speak.

- **Test 3: The Network Ping** — Write a tiny UDP or TCP script to send a message from the Pi to the ESP32 over Wi-Fi to ensure they can communicate.

### Phase 1: The Wireless Face (ESP32 Firmware)

We turn the ESP32 into a dedicated, wireless peripheral.

- **The Code** — You will write C++ firmware for the ESP32 that connects to your local Wi-Fi and opens a UDP socket (a listening port).

- **The Protocol** — It waits for simple string commands from the Pi:
  - `LED:YELLOW:ON` — Turns on the thinking light
  - `LCD:Scanning files...` — Clears the screen and prints that text

### Phase 2: The Agentic Brain (C Orchestrator & Terminal Access)

This is the core of the project. You will write a powerful, multi-threaded C program on the Raspberry Pi.

- **The LLM Bridge** — Your C program will take your typed input and send it to the local Ollama API using libcurl.

- **The Action Engine (popen)** — You will give the AI a strict system prompt (e.g., "To search for a file, output exactly [CMD: find / -name data.txt]"). Your C program will parse the LLM's text. If it detects a command, it will intercept it, run it in the Linux terminal using popen(), capture the output, and feed the result back to the AI to summarize for you.

- **The Broadcaster** — As the C program changes states (Idle → Thinking → Speaking), it will fire off UDP network packets to the ESP32's IP address to update your physical desk display instantly.

### Phase 3: The Voice (Audio Pipeline Integration)

We give Jarvis the ability to speak the final results.

- **The TTS** — When the final text response is ready, your C orchestrator passes it to Piper TTS.

- **The Output** — Piper generates the audio waveform, and the Pi pushes it out through your verified audio hardware (Bluetooth or Transistor circuit).

---

## 🎬 The Workflow in Action

1. You run `./jarvis` on your Pi terminal. The Pi sends a network packet to the ESP32: `LED:BLUE:ON`. Your desk lights up.

2. You type: *"Jarvis, where did I save my python script from yesterday?"*

3. The Pi sends `LED:YELLOW:ON` to the ESP32. The ESP32 LCD updates to *"Thinking..."*

4. The C program asks Ollama. Ollama replies: `[CMD: find /home/perlin -name "*.py" -mtime -1]`

5. Your C program executes that command via `popen()`, finds `/home/perlin/scripts/test.py`, and feeds it back to Ollama.

6. Ollama replies: *"Sir, I found test.py in your scripts folder."*

7. The Pi sends the text to Piper TTS, which speaks the sentence out loud. Simultaneously, it sends `LCD: I found test.py...` to the ESP32 so you can read along.

---

## 🚀 Getting Started

### The 30-Hour Sprint Begins Now

To kick this off properly, we should start with **Phase 0: Test 1**.

Do you have the Arduino IDE (or ESP-IDF) set up for your ESP32? If so, grab your breadboard, the ESP32, the LCD, and the LEDs, and let's write the C++ code to bring the "Face" online!

---

## 📖 Phase 0: Hardware Diagnostics & Sanity Checks

> 💡 **Note:** In systems engineering, we never connect the brain until we know the nervous system actually works. This guide provides the exact steps, wiring logic, and test code to verify every physical component on your workbench before we start writing the complex C orchestrator.

### Test 1: The Visual Status Indicators (ESP32 + LEDs)

We need to verify the ESP32 can successfully drive external outputs without browning out, and that your resistors are the correct value to protect the GPIO pins.

**Components Required:**
- ESP32
- 3× LEDs (Yellow, Blue, Green)
- 3× Resistors (220Ω–330Ω)
- Breadboard
- Jumper Wires

**The Wiring:**

1. Connect the ESP32 GND pin to the negative rail of your breadboard
2. Place the 3 LEDs on the breadboard; connect the short leg (cathode) of each to the GND rail
3. Connect a resistor to the long leg (anode) of each LED
4. Connect the other end of the Yellow LED resistor to ESP32 Pin 12
5. Connect the Blue LED resistor to ESP32 Pin 14
6. Connect the Green LED resistor to ESP32 Pin 27

**The Test Execution:**

Upload this basic C++ sketch to the ESP32 using the Arduino IDE to verify the pins cycle correctly:

```cpp
const int pinYellow = 12;
const int pinBlue = 14;
const int pinGreen = 27;

void setup() {
  pinMode(pinYellow, OUTPUT);
  pinMode(pinBlue, OUTPUT);
  pinMode(pinGreen, OUTPUT);
}

void loop() {
  digitalWrite(pinYellow, HIGH); delay(500); digitalWrite(pinYellow, LOW);
  digitalWrite(pinBlue, HIGH);   delay(500); digitalWrite(pinBlue, LOW);
  digitalWrite(pinGreen, HIGH);  delay(500); digitalWrite(pinGreen, LOW);
}
```

**Pass Condition:** The LEDs sequentially blink in a continuous loop.

---

### Test 2: The UI Interface (ESP32 + 16x2 LCD)

We need to ensure the LCD data lines are secure and the contrast is calibrated so you can actually read the AI's responses.

**Components Required:**
- ESP32
- 16x2 LCD Display

> **Note:** If your LCD has a 4-pin I2C backpack attached, follow the I2C wiring. If it has 16 bare pins, you will need a potentiometer for contrast and 6 data wires.

**The Wiring (Assuming standard I2C Backpack for simplicity):**

| LCD Pin | ESP32 Pin |
|---------|-----------|
| GND | GND |
| VCC | VIN (or 5V) |
| SDA | Pin 21 |
| SCL | Pin 22 |

**The Test Execution:**

Install the `LiquidCrystal_I2C` library in your Arduino IDE and upload this sketch:

```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the standard I2C address

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hardware Init...");
  lcd.setCursor(0, 1);
  lcd.print("Status: ONLINE");
}

void loop() {}
```

**Pass Condition:** The screen lights up and the text is clearly legible. (If the screen is on but blank, use a screwdriver to twist the blue potentiometer on the back of the LCD to adjust the contrast).

---

### Test 3: The Vocal Cords (Raspberry Pi 5 Audio)

We must prove the Pi can output an audio waveform before we install the Text-to-Speech engine. We will test the analog hardware route first.

**Components Required:**
- Raspberry Pi 5
- NPN Transistor (like a 2N2222 or BC547)
- 1kΩ Resistor
- 10µF Capacitor
- Laptop Speaker

**The Wiring (The Audio Amplifier):**

1. Connect the Pi's GND (Pin 6) to the breadboard ground rail
2. Connect the transistor's Emitter leg to GND
3. Connect a hardware PWM pin on the Pi (like GPIO 18 / Pin 12) to the 1kΩ resistor, and plug the other end of the resistor into the transistor's Base (middle leg)
4. Connect the laptop speaker: One wire goes to the Pi's 5V (Pin 2); the other wire goes to the transistor's Collector leg
5. Place the capacitor parallel to the speaker wires to smooth out the digital noise

**The Test Execution:**

Open your Raspberry Pi terminal and generate a test tone using standard ALSA audio utilities:

```bash
speaker-test -t sine -f 440 -c 1 -s 1
```

**Pass Condition:** You hear a clear, continuous 440Hz beep (an 'A' note) from the salvaged speaker. Press Ctrl+C to stop it.

> **Alternative:** If you opt for the Bluetooth speaker route instead: Run `bluetoothctl`, type `scan on`, find your speaker's MAC address, type `pair <MAC>`, `trust <MAC>`, and `connect <MAC>`. Then run the exact same `speaker-test` command.

---

### Test 4: The Network Bridge (Wi-Fi Ping)

The final check. The Brain (Pi) and the Face (ESP32) must be able to see each other on your local network.

**Setup Steps:**

1. Flash a basic Wi-Fi connection script to your ESP32
2. Have it print its assigned IP address to the Serial Monitor (e.g., `192.168.1.50`)
3. Open your Raspberry Pi terminal

**The Test Execution:**

```bash
ping -c 4 192.168.1.50
```

**Pass Condition:** You see 4 packets transmitted, 4 received, 0% packet loss. The bridge is established.

---

## ❓ Next Steps

Which of these diagnostic tests are you setting up on your breadboard right now? We can dig into the specifics if you run into any wiring issues or compilation errors!

