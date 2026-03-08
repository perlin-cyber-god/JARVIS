# JARVIS - Just Another Rather Very Intelligent System

JARVIS is a modular AI assistant and robotic control system designed with a distributed architecture, featuring a central "Brain" and peripheral components like a "Face" for interaction.

## 🏗 Project Structure

- **`src/brain/`**: The core intelligence of JARVIS.
  - `jarvis_main.c`: Entry point for the brain.
  - `jarvis_brain.c`: Logic for command execution, API integration, and communication.
  - `telemetry_daemon.c`: Monitoring and data logging.
  - `agent_simulation.c`: Behavioral simulation environment.
- **`src/hardware/`**: Hardware control scripts.
  - `fan_control.py`: Python script for PWM fan control and RPM monitoring using `lgpio`.
- **`src/face/`**: Firmware for the visual/interaction interface (ESP32-based).
- **`config/`**: System configuration files.
- **`docs/`**: Technical documentation and design notes.
- **`tests/`**: Unit tests and hardware verification scripts (LCD, ESP check).
- **`legacy/`**: Archived firmware versions.

## 🚀 Key Features

- **Distributed Control**: Brain (C) communicates with peripheral devices via UDP.
- **Hardware Management**: Dynamic fan control based on CPU temperature with stall detection.
- **System Integration**: Capabilities to execute shell commands and capture output.
- **API Connectivity**: Integrated with `libcurl` and `cJSON` for web services.
- **Telemetry**: Real-time monitoring of system states.
- **Hardware Support**: ESP32 and I2C LCD integration for visual feedback.

## 🛠 Tech Stack

- **Languages**: C, C++, Python
- **Libraries**: `libcurl`, `cJSON`, `socket` (Linux), `lgpio` (Python), `ESP32 Arduino`
- **Configuration**: VS Code with C/C++ tools.

## 📦 Setup & Installation

1. **Prerequisites**:
   - GCC/G++ for Linux.
   - Python 3.x.
   - `lgpio` library (e.g., `sudo apt install python3-lgpio`).
   - Arduino IDE or PlatformIO for ESP32.
   - `libcurl` and `cJSON` development headers.

2. **Build**:
   ```bash
   # Build the brain
   gcc -o jarvis_brain src/brain/jarvis_main.c src/brain/jarvis_brain.c -lcurl -lcjson
   ```

3. **Deploy Face**:
   Flash `src/face/face_firmware_v2.cpp` to your ESP32 device.

---
*Created and maintained by [perlin-cyber-god](https://github.com/perlin-cyber-god)*
