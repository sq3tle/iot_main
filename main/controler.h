//rtos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//common
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_sleep.h"

//io
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

//definitions
#define RELAY_CH0_PIN 2
#define RELAY_CH1_PIN 6
#define THERMOCOUPLE_CS_PIN 9
#define THERMOCOUPLE_MISO_PIN 4
#define THERMOCOUPLE_SCK_PIN 5

void controler_task(void *pvParameters);
float ble_temp_read(void);



