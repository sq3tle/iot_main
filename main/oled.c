#include "oled.h"

static const char* TAG = "oled";
static SSD1306_t dev;

void oled_clear(void)
{
    ssd1306_clear_screen(&dev, false);
}


void oled_show(const char* text, int len, int line)
{
    ssd1306_display_text(&dev, line, text, len, false);
}

void oled_init(void)
{
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, -1);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
}