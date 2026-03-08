# JARVIS - Just Another Rather Very COOL System

JARVIS is a modular AI assistant and robotic control system designed with a distributed architecture, featuring a central "Brain" and peripheral components like a "Face" for interaction.

## 🏗 Project Structure

- **`src/brain/`**: The core intelligence of JARVIS.
  - `jarvis_main.c`: Entry point for the brain.
  - `jarvis_brain.c`: Logic for command execution, API integration, and communication.
  - `telemetry_daemon.c`: Monitoring and data logging.
  - `agent_simulation.c`: Behavioral simulation environment.
- **`src/hardware/`**: Hardware control scripts.
  - `fan_control.py`: Python script for PWM fan control and RPM monitoring using `lgpio`.
  - **`driver/`**: Linux Kernel Driver for JARVIS Fan (Hardware Offloading).
    - `jarvis_fan.c`: Phase 1: PWM Character Driver.
    - `Makefile`: Build script for the kernel module.
- **`src/face/`**: Firmware for the visual/interaction interface (ESP32-based).
- **`config/`**: System configuration files.
- **`docs/`**: Technical documentation and design notes.
- **`tests/`**: Unit tests and hardware verification scripts (LCD, ESP check).
- **`legacy/`**: Archived firmware versions.

## 🚀 Key Features

- **Distributed Control**: Brain (C) communicates with peripheral devices via UDP.
- **Hardware Management**: Dynamic fan control with both Python and Kernel Driver implementations.
- **Hardware Offloading**: Kernel driver reduces CPU load by utilizing hardware PWM controllers and handling interrupts in kernel space.
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
   - Python 3.x and `lgpio`.
   - Linux Kernel Headers (for driver development).
   - Arduino IDE or PlatformIO for ESP32.
   - `libcurl` and `cJSON` development headers.

2. **Build the Brain**:
   ```bash
   # Build the brain
   gcc -o jarvis_brain src/brain/jarvis_main.c src/brain/jarvis_brain.c -lcurl -lcjson
   ```

3. **Build the Kernel Driver (Phase 1)**:
   ```bash
   cd src/hardware/driver
   make
   sudo insmod jarvis_fan.ko
   # Usage: echo 50 | sudo tee /dev/jarvis_fan
   ```

4. **Deploy Face**:
   Flash `src/face/face_firmware_v2.cpp` to your ESP32 device.

---
*Created and maintained by [perlin-cyber-god](https://github.com/perlin-cyber-god)*
