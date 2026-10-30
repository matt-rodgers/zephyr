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

static const struct led_rgb colours[] = {
	{ .r = 0xFF, .g = 0x20, .b = 0x00, }, /* orange */
	{ .r = 0xFF, .g = 0x00, .b = 0x40, }, /* purple */
	{ .r = 0x60, .g = 0xA0, .b = 0x00, }, /* green */
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

	if (!device_is_ready(led_strip_device)) {
		printk("LED strip not ready\n");
		return;
	}

	while (1) {
		for (size_t i = 0; i < 3; i++) {
			const struct led_rgb *rgb = &colours[i];

			uint8_t brightness = 1;
			bool rising = true;

			while (brightness > 0) {
				if (rising && brightness <= 30) {
					brightness++;
				} else {
					rising = false;
					brightness--;
				}

			        memset(&pixels, 0, sizeof(pixels));

			        for (uint32_t i = 0; i < LED_STRIP_NUM_PIXELS; i++) {
			            pixels[i].r = scale_brightness(rgb->r, brightness);
			            pixels[i].g = scale_brightness(rgb->g, brightness);
			            pixels[i].b = scale_brightness(rgb->b, brightness);
			        }

			        /* Update the LED strip */
			        ret = led_strip_update_rgb(led_strip_device, pixels, LED_STRIP_NUM_PIXELS);
			        if (ret < 0) {
			            printk("Error %d updating LED strip\n", ret);
			        }

			        k_msleep(40);
			}
		}
	}
}
K_THREAD_DEFINE(led_strip_thread_id, LED_STRIP_STACK_SIZE, led_strip_thread_entry,
                NULL, NULL, NULL,
        	LED_STRIP_PRIORITY, 0, 0);
