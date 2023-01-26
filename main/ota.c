#include "main.h"

static const char* TAG = "ota";

#if USE_SQ3TLE_OTA_SERVER
extern const uint8_t server_cert_pem_start[] asm("_binary_lets_encrypt_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_lets_encrypt_cert_pem_end");
#else
#include "esp_crt_bundle.h"
#endif

static esp_err_t compare_version(esp_app_desc_t *new_app_info)
{
    if (new_app_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

    if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0) {
        ESP_LOGW(TAG, "Local running version is the same. Aborting...");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void firmware_update_task(void *pvParameter){

    esp_wifi_set_ps(WIFI_PS_NONE);
    
    esp_err_t ota_finish_err = ESP_OK;
    esp_http_client_config_t config = {
#if USE_SQ3TLE_OTA_SERVER
        .url = "https://ota.sq3tle.dev/iot_main.bin",
        .cert_pem = (char *)server_cert_pem_start,       
#else
        .url = "https://github.com/sq3tle/iot_main/raw/master/build/iot_main.bin",
        .crt_bundle_attach = esp_crt_bundle_attach,
#endif
        .keep_alive_enable = true,
        .skip_cert_common_name_check = false,
        .timeout_ms = 1500
    };
    
    esp_https_ota_config_t ota_config = {
        .http_config = &config
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
        vTaskDelete(NULL);
    }

    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");

        esp_https_ota_abort(https_ota_handle);
        vTaskDelete(NULL);
    }
    err = compare_version(&app_desc);
    if (err != ESP_OK) {

        esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
        esp_https_ota_abort(https_ota_handle);
        vTaskDelete(NULL);
    }

    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        ESP_LOGD(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
    }

    ota_finish_err = esp_https_ota_finish(https_ota_handle);

    if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
        ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
    } 
}