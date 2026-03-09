# JARVIS Hardware Drivers

This directory contains kernel-level drivers for the JARVIS hardware platform, specifically targeting the Raspberry Pi 5.

## Drivers

### jarvis_thermal_fan.c
This is the advanced ("God-Tier") thermal cooling device driver for the system fan.

- **Target Hardware:** Raspberry Pi 5 (GPIO 13 / PWM1).
- **Fan Type:** Salvaged 2011 Laptop PWM Fan via transistor circuit.
- **Integration:** Registers as a native Linux `thermal_cooling_device`. This allows the Linux thermal subsystem and governors to automatically control the fan speed based on SoC temperature.
- **Cooling States:**
  - **State 0:** Off (0% duty cycle)
  - **State 1:** Low (40% duty cycle)
  - **State 2:** Medium (70% duty cycle)
  - **State 3:** High (100% duty cycle)
- **Technical Details:**
  - PWM Frequency: 25Hz (40,000,000 ns period).
  - Uses `pwm_apply_state` for atomic hardware updates.
  - Implements `get_max_state`, `get_cur_state`, and `set_cur_state` callbacks for the thermal subsystem.

### jarvis_fan.c
A simpler character driver (`/dev/jarvis_fan`) for manual PWM control. This was likely the Phase 1 development driver before the thermal subsystem integration.
