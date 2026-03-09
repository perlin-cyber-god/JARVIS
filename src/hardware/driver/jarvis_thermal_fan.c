#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pwm.h>
#include <linux/thermal.h>
#include <linux/err.h>
#include <linux/slab.h>

/**
 * JARVIS Thermal Fan Driver (Method 2: The God-Tier Kernel Way)
 * 
 * Target: Raspberry Pi 5
 * Hardware: Salvaged 2011 Laptop PWM Fan via Transistor Circuit
 * Pin: GPIO 13 (Hardware PWM1)
 */

#define DRIVER_NAME "jarvis_thermal_fan"
#define PWM_PERIOD_NS 40000000 // 25Hz (1/25 * 10^9 ns)
#define MAX_COOLING_STATE 3

// Duty cycle mapping: State 0=0%, 1=40%, 2=70%, 3=100%
static const unsigned int state_duty_percents[] = {0, 40, 70, 100};

struct jarvis_thermal_fan_ctx {
    struct pwm_device *pwm;
    struct thermal_cooling_device *cdev;
    unsigned long cur_state;
};

static struct jarvis_thermal_fan_ctx *fan_ctx;

/**
 * get_max_state - Thermal subsystem asks for the max state index
 */
static int jarvis_get_max_state(struct thermal_cooling_device *cdev,
                               unsigned long *state)
{
    *state = MAX_COOLING_STATE;
    return 0;
}

/**
 * get_cur_state - Thermal subsystem asks for the current state index
 */
static int jarvis_get_cur_state(struct thermal_cooling_device *cdev,
                               unsigned long *state)
{
    struct jarvis_thermal_fan_ctx *ctx = cdev->devdata;
    *state = ctx->cur_state;
    return 0;
}

/**
 * set_cur_state - Thermal governor commands a specific cooling state (0-3)
 */
static int jarvis_set_cur_state(struct thermal_cooling_device *cdev,
                               unsigned long state)
{
    struct jarvis_thermal_fan_ctx *ctx = cdev->devdata;
    struct pwm_state pstate;
    unsigned int duty_percent;
    u64 duty_ns;

    if (state > MAX_COOLING_STATE)
        return -EINVAL;

    // Optimization: Don't re-apply if state hasn't changed
    if (ctx->cur_state == state && pwm_is_enabled(ctx->pwm) == (state > 0))
        return 0;

    duty_percent = state_duty_percents[state];
    
    // Calculate duty cycle: (Period * Percent) / 100
    duty_ns = (u64)PWM_PERIOD_NS * duty_percent;
    do_div(duty_ns, 100);

    pwm_get_state(ctx->pwm, &pstate);
    pstate.period = PWM_PERIOD_NS;
    pstate.duty_cycle = (long)duty_ns;
    pstate.enabled = (duty_ns > 0);

    // Apply the hardware change
    pwm_apply_state(ctx->pwm, &pstate);

    ctx->cur_state = state;
    pr_info("Jarvis Thermal: State changed to %lu (%u%% duty cycle)\n", 
            state, duty_percent);

    return 0;
}

static struct thermal_cooling_device_ops jarvis_fan_ops = {
    .get_max_state = jarvis_get_max_state,
    .get_cur_state = jarvis_get_cur_state,
    .set_cur_state = jarvis_set_cur_state,
};

static int __init jarvis_thermal_fan_init(void)
{
    int ret;

    pr_info("Jarvis Thermal: Initializing RPi 5 PWM Fan Driver (GPIO 13)\n");

    fan_ctx = kzalloc(sizeof(*fan_ctx), GFP_KERNEL);
    if (!fan_ctx)
        return -ENOMEM;

    /**
     * PWM Request:
     * On RPi 5, PWM1 usually handles GPIO 12/13. 
     * pwm_request(1, ...) requests the second PWM channel.
     * Ensure 'dtoverlay=pwm-2chan' or similar is in /boot/config.txt
     */
    fan_ctx->pwm = pwm_request(1, DRIVER_NAME);
    if (IS_ERR(fan_ctx->pwm)) {
        pr_err("Jarvis Thermal: Failed to request PWM1. Check Device Tree overlays.\n");
        ret = PTR_ERR(fan_ctx->pwm);
        goto err_free;
    }

    // Initial hardware state: Off
    fan_ctx->cur_state = 0;
    {
        struct pwm_state initial_state;
        pwm_init_state(fan_ctx->pwm, &initial_state);
        initial_state.period = PWM_PERIOD_NS;
        initial_state.duty_cycle = 0;
        initial_state.enabled = false;
        pwm_apply_state(fan_ctx->pwm, &initial_state);
    }

    // Register as a native Linux cooling device
    fan_ctx->cdev = thermal_cooling_device_register("jarvis-fan", fan_ctx, &jarvis_fan_ops);
    if (IS_ERR(fan_ctx->cdev)) {
        pr_err("Jarvis Thermal: Failed to register thermal cooling device\n");
        ret = PTR_ERR(fan_ctx->cdev);
        goto err_pwm;
    }

    pr_info("Jarvis Thermal: Driver loaded. Cooling device 'jarvis-fan' active.\n");
    return 0;

err_pwm:
    pwm_free(fan_ctx->pwm);
err_free:
    kfree(fan_ctx);
    return ret;
}

static void __exit jarvis_thermal_fan_exit(void)
{
    if (fan_ctx) {
        if (fan_ctx->cdev)
            thermal_cooling_device_unregister(fan_ctx->cdev);
        
        if (fan_ctx->pwm) {
            struct pwm_state exit_state;
            pwm_get_state(fan_ctx->pwm, &exit_state);
            exit_state.enabled = false;
            pwm_apply_state(fan_ctx->pwm, &exit_state);
            pwm_free(fan_ctx->pwm);
        }
        kfree(fan_ctx);
    }
    pr_info("Jarvis Thermal: Driver unloaded and GPIO 13 released\n");
}

module_init(jarvis_thermal_fan_init);
module_exit(jarvis_thermal_fan_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JARVIS Team");
MODULE_DESCRIPTION("Thermal Cooling Device Driver for salvaged laptop fan on RPi 5");
