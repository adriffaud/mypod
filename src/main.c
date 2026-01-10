#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>
#include <string.h>

LOG_MODULE_REGISTER(app);

#define TEXT_STR "Hello world"
#define GLYPH_W 5
#define GLYPH_H 7
#define GLYPH_SPACING 1

static const uint8_t *get_glyph(char c)
{
	static const uint8_t space[GLYPH_H] = { 0, 0, 0, 0, 0, 0, 0 };
	static const uint8_t H[GLYPH_H] = { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 };
	static const uint8_t e[GLYPH_H] = { 0x00, 0x0E, 0x11, 0x1F, 0x10, 0x0E, 0x00 };
	static const uint8_t l[GLYPH_H] = { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 };
	static const uint8_t o[GLYPH_H] = { 0x00, 0x0E, 0x11, 0x11, 0x11, 0x0E, 0x00 };
	static const uint8_t w[GLYPH_H] = { 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A, 0x00 };
	static const uint8_t r[GLYPH_H] = { 0x00, 0x16, 0x19, 0x10, 0x10, 0x10, 0x00 };
	static const uint8_t d[GLYPH_H] = { 0x01, 0x01, 0x0D, 0x13, 0x11, 0x0F, 0x00 };

	switch (c) {
	case 'H':
		return H;
	case 'e':
		return e;
	case 'l':
		return l;
	case 'o':
		return o;
	case 'w':
		return w;
	case 'r':
		return r;
	case 'd':
		return d;
	case ' ':
	default:
		return space;
	}
}

static uint16_t pack_rgb565(enum display_pixel_format format, uint8_t r5, uint8_t g6, uint8_t b5)
{
	if (format == PIXEL_FORMAT_BGR_565) {
		uint8_t tmp = r5;

		r5 = b5;
		b5 = tmp;
	}

	return sys_cpu_to_be16((r5 << 11) | (g6 << 5) | b5);
}

static void draw_text_rgb565(uint16_t *buf, uint16_t buf_w, uint16_t buf_h,
			     const char *text, enum display_pixel_format format)
{
	const uint16_t fg = pack_rgb565(format, 31, 63, 31);
	const uint16_t bg = pack_rgb565(format, 0, 0, 0);
	size_t text_len = strlen(text);

	for (uint16_t y = 0; y < buf_h; y++) {
		for (uint16_t x = 0; x < buf_w; x++) {
			buf[y * buf_w + x] = bg;
		}
	}

	for (size_t i = 0; i < text_len; i++) {
		const uint8_t *glyph = get_glyph(text[i]);
		uint16_t x0 = i * (GLYPH_W + GLYPH_SPACING);

		for (uint16_t y = 0; y < GLYPH_H; y++) {
			uint8_t row = glyph[y];

			for (uint16_t x = 0; x < GLYPH_W; x++) {
				if (row & BIT(GLYPH_W - 1 - x)) {
					uint16_t px = x0 + x;

					if (px < buf_w) {
						buf[y * buf_w + px] = fg;
					}
				}
			}
		}
	}
}

static int fill_screen_color(const struct device *display, const struct display_capabilities *caps,
			     enum display_pixel_format format, uint8_t r5, uint8_t g6, uint8_t b5)
{
	static uint16_t line_buf[320];
	struct display_buffer_descriptor desc = {
		.width = 0,
		.height = 1,
		.pitch = 0,
	};
	uint16_t color;

	if (caps->x_resolution > ARRAY_SIZE(line_buf)) {
		LOG_ERR("Line buffer too small for width %u", caps->x_resolution);
		return -EINVAL;
	}

	color = pack_rgb565(format, r5, g6, b5);
	for (uint16_t x = 0; x < caps->x_resolution; x++) {
		line_buf[x] = color;
	}

	desc.width = caps->x_resolution;
	desc.pitch = caps->x_resolution;
	desc.buf_size = caps->x_resolution * sizeof(line_buf[0]);

	for (uint16_t y = 0; y < caps->y_resolution; y++) {
		int rc = display_write(display, 0, y, &desc, line_buf);

		if (rc != 0) {
			LOG_ERR("display_write failed at line %u: %d", y, rc);
			return rc;
		}
	}

	return 0;
}

int main(void)
{
	const struct device *display = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	struct display_capabilities caps;
	enum display_pixel_format format = PIXEL_FORMAT_RGB_565;
	static uint16_t text_buf[(sizeof(TEXT_STR) - 1) * (GLYPH_W + GLYPH_SPACING) * GLYPH_H];
	struct display_buffer_descriptor desc;
	uint16_t text_w;
	int rc;

	if (!device_is_ready(display)) {
		LOG_ERR("Display not ready");
		return 0;
	}
	LOG_INF("Display device ready");

	display_get_capabilities(display, &caps);
	LOG_INF("Display %ux%u fmt=0x%08x", caps.x_resolution, caps.y_resolution,
		caps.supported_pixel_formats);
	if (!(caps.supported_pixel_formats & PIXEL_FORMAT_RGB_565) &&
	    !(caps.supported_pixel_formats & PIXEL_FORMAT_BGR_565)) {
		LOG_ERR("No RGB/BGR565 support (formats=0x%08x)", caps.supported_pixel_formats);
		return 0;
	}
	if (caps.supported_pixel_formats & PIXEL_FORMAT_BGR_565) {
		format = PIXEL_FORMAT_BGR_565;
	}

	rc = display_set_pixel_format(display, format);
	if (rc != 0) {
		LOG_ERR("display_set_pixel_format failed: %d", rc);
		return 0;
	}
	LOG_INF("display_set_pixel_format ok");

	rc = display_blanking_off(display);
	if (rc != 0) {
		LOG_ERR("display_blanking_off failed: %d", rc);
		return 0;
	}
	LOG_INF("display_blanking_off ok");
	display_clear(display);
	LOG_INF("display_clear ok");
	k_sleep(K_MSEC(10));

	rc = fill_screen_color(display, &caps, format, 31, 0, 0);
	if (rc == 0) {
		k_sleep(K_MSEC(300));
		rc = fill_screen_color(display, &caps, format, 0, 63, 0);
	}
	if (rc == 0) {
		k_sleep(K_MSEC(300));
		rc = fill_screen_color(display, &caps, format, 0, 0, 31);
	}
	if (rc != 0) {
		LOG_ERR("fill_screen_color failed: %d", rc);
		return 0;
	}
	LOG_INF("fill_screen_color ok");

	text_w = (sizeof(TEXT_STR) - 1) * (GLYPH_W + GLYPH_SPACING);
	draw_text_rgb565(text_buf, text_w, GLYPH_H, TEXT_STR, format);

	desc.buf_size = text_w * GLYPH_H * 2;
	desc.width = text_w;
	desc.height = GLYPH_H;
	desc.pitch = text_w;

	uint16_t x_pos = 0;
	uint16_t y_pos = 0;

	if (caps.x_resolution > text_w) {
		x_pos = (caps.x_resolution - text_w) / 2;
	}
	if (caps.y_resolution > GLYPH_H) {
		y_pos = (caps.y_resolution - GLYPH_H) / 2;
	}

	rc = display_write(display, x_pos, y_pos, &desc, text_buf);
	if (rc != 0) {
		LOG_ERR("display_write failed: %d", rc);
		return 0;
	}
	LOG_INF("display_write ok");
	LOG_INF("Hello world drawn on display (%ux%u)", desc.width, desc.height);

	for (;;) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
