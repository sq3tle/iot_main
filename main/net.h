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

//ota
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

//provisioning
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>

bool WIFI_CONNECTED;

#define PROVISION_PREFIX "PROV_LODOWNIK"

void init_network(void); 
void provision(void);