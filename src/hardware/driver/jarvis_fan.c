#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/pwm.h>
#include <linux/device.h>
#include <linux/err.h>

#define DEVICE_NAME "jarvis_fan"
#define CLASS_NAME "jarvis"

static int major_number;
static struct class* fan_class = NULL;
static struct device* fan_device = NULL;
static struct pwm_device *pwm_fan = NULL;

// PWM settings: 25Hz -> Period = 40,000,000 ns (1/25 * 10^9)
#define PWM_PERIOD_NS 40000000

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    char kbuf[16];
    int duty_percent;
    long duty_ns;
    int ret;

    if (len > sizeof(kbuf) - 1) len = sizeof(kbuf) - 1;
    if (copy_from_user(kbuf, buffer, len)) return -EFAULT;
    kbuf[len] = '\0';

    ret = kstrtoint(kbuf, 10, &duty_percent);
    if (ret < 0) return ret;

    if (duty_percent < 0) duty_percent = 0;
    if (duty_percent > 100) duty_percent = 100;

    duty_ns = (PWM_PERIOD_NS * duty_percent) / 100;
    
    if (pwm_fan) {
        struct pwm_state state;
        pwm_get_state(pwm_fan, &state);
        state.period = PWM_PERIOD_NS;
        state.duty_cycle = (long)duty_ns;
        state.enabled = (duty_ns > 0);
        pwm_apply_state(pwm_fan, &state);
        pr_info("Jarvis Fan: Duty cycle set to %d%% (%ld ns)\n", duty_percent, duty_ns);
    } else {
        pr_warn("Jarvis Fan: PWM device not acquired, can't set duty cycle.\n");
    }

    return len;
}

static struct file_operations fops = {
    .write = dev_write,
};

static int __init fan_driver_init(void) {
    pr_info("Jarvis Fan: Initializing Phase 1 driver\n");

    // 1. Register Major Number
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        pr_err("Jarvis Fan: Failed to register major number\n");
        return major_number;
    }

    // 2. Register Device Class
    fan_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(fan_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(fan_class);
    }

    // 3. Create Device
    fan_device = device_create(fan_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(fan_device)) {
        class_destroy(fan_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(fan_device);
    }

    // 4. Request PWM
    // Note: This requires proper device tree overlay to expose the PWM.
    pwm_fan = pwm_request(0, DEVICE_NAME); // Try index 0
    if (IS_ERR(pwm_fan)) {
        pr_warn("Jarvis Fan: PWM request failed, hardware PWM might need DT overlay. Error: %ld\n", PTR_ERR(pwm_fan));
        pwm_fan = NULL; 
    } else {
        struct pwm_state state;
        pwm_init_state(pwm_fan, &state);
        state.period = PWM_PERIOD_NS;
        state.duty_cycle = 0;
        state.enabled = false;
        pwm_apply_state(pwm_fan, &state);
        pr_info("Jarvis Fan: PWM0 requested successfully.\n");
    }

    return 0;
}

static void __exit fan_driver_exit(void) {
    if (pwm_fan) {
        struct pwm_state state;
        pwm_get_state(pwm_fan, &state);
        state.enabled = false;
        pwm_apply_state(pwm_fan, &state);
        pwm_free(pwm_fan);
    }
    device_destroy(fan_class, MKDEV(major_number, 0));
    class_unregister(fan_class);
    class_destroy(fan_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("Jarvis Fan: Driver unloaded\n");
}

module_init(fan_driver_init);
module_exit(fan_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JARVIS Team");
MODULE_DESCRIPTION("Phase 1: Character Driver for PWM Fan Control");
