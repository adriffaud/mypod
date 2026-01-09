#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/kernel.h>

int main(void)
{
	const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

	if (device_is_ready(display)) {
		display_blanking_off(display);
	}

	for (;;) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
