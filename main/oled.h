#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ssd1306.h"
#include "font8x8_basic.h"

#define CONFIG_SDA_GPIO 8
#define CONFIG_SCL_GPIO 7

void oled_init(void);
void oled_clear(void);
void oled_show(const char* text, int len, int line);