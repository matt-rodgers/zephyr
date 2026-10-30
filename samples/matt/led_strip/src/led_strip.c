#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/shell/shell.h>

#include <string.h>


#define LED_STRIP_STACK_SIZE    512
#define LED_STRIP_PRIORITY      4


#define LED_STRIP_NODE       DT_ALIAS(led_strip)
#define LED_STRIP_NUM_PIXELS DT_PROP(DT_ALIAS(led_strip), chain_length)


static const struct device *const led_strip_device = DEVICE_DT_GET(LED_STRIP_NODE);

static const struct led_rgb warm_white = {
    .r = 200, .g = 150, .b = 100
};

/* Array of pixel buffers in the strip */
static struct led_rgb pixels[LED_STRIP_NUM_PIXELS];


static uint8_t scale_brightness(uint8_t original, uint8_t scale_percent)
{
    uint32_t scaled = (uint32_t)original * scale_percent;

    return (uint8_t)(scaled / 100);
}


static void led_strip_thread_entry(void * p1, void * p2, void * p3)
{
    int ret;
    uint8_t brightness_percent = 0;

    if (!device_is_ready(led_strip_device)) {
		printk("LED strip not ready\n");
		return;
	}

    while (1) {
        /* Clear all colours each loop */
        memset(&pixels, 0, sizeof(pixels));

        for (uint32_t i = 0; i < LED_STRIP_NUM_PIXELS; i++) {
            pixels[i].r = scale_brightness(warm_white.r, brightness_percent);
            pixels[i].g = scale_brightness(warm_white.g, brightness_percent);
            pixels[i].b = scale_brightness(warm_white.b, brightness_percent);
        }

        /* Update the LED strip */
        ret = led_strip_update_rgb(led_strip_device, pixels, LED_STRIP_NUM_PIXELS);
        if (ret < 0) {
            printk("Error %d updating LED strip\n", ret);
        }

        printk("some stuff\n");
        k_msleep(1000);

        brightness_percent++;

        if (brightness_percent >= 100) {
            brightness_percent = 0;
        }
    }
}
K_THREAD_DEFINE(led_strip_thread_id, LED_STRIP_STACK_SIZE, led_strip_thread_entry,
                NULL, NULL, NULL,
        	LED_STRIP_PRIORITY, 0, 0);
