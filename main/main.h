#pragma once

//common
#include <stdio.h>
#include <string.h>

//rtos
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

//basic
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include "esp_sleep.h"

//ota
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_wifi.h"

//ota server selection
#define USE_SQ3TLE_OTA_SERVER false

// atomic 
bool relay_channel0;
bool relay_channel1;

// not atomic - check safety in risc-v
float ext_adc;
float temp_setpoint;
float temp;

void firmware_update_task(void *pvParameter);
void reset_creds(void);
