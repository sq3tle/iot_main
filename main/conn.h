#pragma once

//rtos
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

//common
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_sleep.h"

//mqtt
#include "mqtt_client.h"
#include "lwip/sockets.h"
#include "esp_event.h"

//ble
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

//definitions for ble
#define GATTS_SERVICE_UUID 0x181A
#define GATTS_CHAR_UUID 0x2A6E
#define GATTS_DESCR_UUID 0x3333
#define DEVICE_NAME "Lodownik"

//deep sleep times
#define FULL_CYCLE_TIME 20000 
#define AWAKE_CYCLE_TIME 15000

//definitions for mqtt
#define MQTT_URL "mqtt://test.mosquitto.org"

//publish topics
#define MQTT_TOPIC_TEMP "lodownik/temp"
#define MQTT_TOPIC_EXT_ADC "lodownik/ext_adc"
#define MQTT_TOPIC_RTOS_FREE_MEM "lodownik/system/rtos_free"
#define MQTT_TOPIC_RTOS_MIN_MEM "lodownik/system/rtos_min"
#define MQTT_TOPIC_CTRL_RELAY "lodownik/relay1"

//subsciption topics
#define MQTT_TOPIC_USER_RELAY "lodownik/relay0"
#define MQTT_TOPIC_TEMP_SETPOINT "lodownik/setpoint"
#define MQTT_TOPIC_SLEEP "lodownik/system/sleep"
#define MQTT_TOPIC_RESET "lodownik/system/reset"
#define MQTT_TOPIC_OTA "lodownik/system/ota"

void mqtt_task(void *pvParameters);
void init_ble(void);











