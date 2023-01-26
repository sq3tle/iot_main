#include "net.h"
#include "conn.h"
#include "controler.h"

static const char* TAG = "main";

bool relay_channel0 = false;
bool relay_channel1 = false;
float ext_adc = 0.0;
float temp_setpoint = 0.0;
float temp = 0.0;

static void init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NOT_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
}

static void memory_task(void *pvParameters) {
    while (1) {
        ESP_LOGI(TAG, "Free %.2f KB / Minimum: %.2f KB", xPortGetFreeHeapSize() / 1024.f, xPortGetMinimumEverFreeHeapSize() / 1024.f);
        vTaskDelay(2500 / portTICK_RATE_MS);
    }
}

void reset_creds(void){
    nvs_flash_erase();
}

void app_main(void) {
    
    init_nvs(); 

    if (esp_reset_reason() == ESP_RST_DEEPSLEEP)
    {
        init_ble();
        return;
    }

    init_network();
    xTaskCreate(memory_task, "memory_task", 2048, NULL, 20, NULL);
    xTaskCreate(mqtt_task, "mqtt_task", 2048, NULL, 15, NULL);
    xTaskCreate(controler_task, "controler_task", 2048, NULL, 5, NULL);

}