import time
import sys
import lgpio

# Configuration
CHIP_NUM = 0
FAN_PWM_GPIO = 13
TACH_GPIO = 27
PWM_FREQ = 25  # Hz (Some fans prefer higher like 25000, but lgpio works well with lower for simple DC fans)

TEMP_MIN = 40.0
TEMP_MAX = 75.0
DUTY_MIN = 30.0
DUTY_MAX = 100.0

SAMPLE_INTERVAL = 2.0  # Seconds between updates
PULSES_PER_REV = 2     # Standard for most PC fans

# Global pulse counter
pulse_count = 0

def tach_callback(chip, gpio, level, tick):
    global pulse_count
    if level == 1:  # Rising edge
        pulse_count += 1

def get_cpu_temp():
    try:
        with open("/sys/class/thermal/thermal_zone0/temp", "r") as f:
            temp_str = f.read()
            return float(temp_str) / 1000.0
    except Exception as e:
        print(f"Error reading temperature: {e}", file=sys.stderr)
        return 0.0

def calculate_duty(temp):
    if temp <= TEMP_MIN:
        return DUTY_MIN
    if temp >= TEMP_MAX:
        return DUTY_MAX
    
    # Linear interpolation
    duty = DUTY_MIN + (temp - TEMP_MIN) * (DUTY_MAX - DUTY_MIN) / (TEMP_MAX - TEMP_MIN)
    return round(duty, 1)

def main():
    global pulse_count
    h = None
    try:
        h = lgpio.gpiochip_open(CHIP_NUM)
        
        # Setup PWM on GPIO 13
        lgpio.gpio_claim_output(h, FAN_PWM_GPIO)
        
        # Setup Tachometer on GPIO 27 with Pull-up
        lgpio.gpio_claim_input(h, TACH_GPIO, lgpio.SET_PULL_UP)
        cb = lgpio.callback(h, TACH_GPIO, lgpio.RISING_EDGE, tach_callback)
        
        print(f"Fan control started on GPIO {FAN_PWM_GPIO} (PWM) and GPIO {TACH_GPIO} (Tach).")
        print(f"Temp Range: {TEMP_MIN}°C-{TEMP_MAX}°C -> Duty: {DUTY_MIN}%-{DUTY_MAX}%")
        
        last_time = time.time()
        
        while True:
            temp = get_cpu_temp()
            duty_cycle = calculate_duty(temp)
            
            # Update PWM
            lgpio.tx_pwm(h, FAN_PWM_GPIO, PWM_FREQ, duty_cycle)
            
            # Wait for sample interval to count pulses
            time.sleep(SAMPLE_INTERVAL)
            
            # Calculate RPM
            current_time = time.time()
            elapsed = current_time - last_time
            
            # Atomically get and reset pulse_count (as much as possible in Python)
            count = pulse_count
            pulse_count = 0
            last_time = current_time
            
            rpm = (count / elapsed) * 60 / PULSES_PER_REV
            
            print(f"Temp: {temp:.1f}°C | Duty: {duty_cycle}% | RPM: {int(rpm)}")
            
            # Alert logic
            if duty_cycle > 0 and rpm == 0:
                print("!!! ALERT: Fan stalled or disconnected while PWM active! !!!", file=sys.stderr)
                # Optional: trigger system alert like wall or a custom command
                # os.system("wall 'FAN FAILURE DETECTED'")

    except KeyboardInterrupt:
        print("\nStopping fan control...")
    except Exception as e:
        print(f"An error occurred: {e}", file=sys.stderr)
    finally:
        if h is not None:
            # Try to stop PWM and close chip
            try:
                lgpio.tx_pwm(h, FAN_PWM_GPIO, PWM_FREQ, 0)
                lgpio.gpiochip_close(h)
            except:
                pass

if __name__ == "__main__":
    main()
