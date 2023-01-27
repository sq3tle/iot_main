#include "conn.h"
#include "net.h"
#include "main.h"

static const char* TAG = "mqtt";
static EventGroupHandle_t mqtt_event_group;
static esp_mqtt_client_handle_t mqtt_client;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    if (event->event_id == MQTT_EVENT_CONNECTED) {
        xEventGroupSetBits(mqtt_event_group, BIT0);
        return;
    } 
    
    if (event->event_id != MQTT_EVENT_DATA)
        return;
    
    if (strncmp(event->topic, MQTT_TOPIC_USER_RELAY, event->topic_len) == 0){
        
        if (strncmp(event->data, "on", event->data_len) == 0)
        {
            relay_channel0 = true;
            ESP_LOGI(TAG, "Updated relay_channel0 = %i", relay_channel0);
        }
        else if(strncmp(event->data, "off", event->data_len) == 0)
        {
            relay_channel0 = false;
            ESP_LOGI(TAG, "Updated relay_channel0 = %i", relay_channel0);
        }
        return;
    }

    if (strncmp(event->topic, MQTT_TOPIC_TEMP_SETPOINT, event->topic_len) == 0){
        temp_setpoint = strtof(event->data, &(event->data)+3);
        ESP_LOGI(TAG, "Updated temp_setpoint = %f", temp_setpoint);
        return;
    }

    if (strncmp(event->topic, MQTT_TOPIC_SLEEP, event->topic_len) == 0){
        ESP_LOGI(TAG, "Entering low power mode until next power cycle!");
        esp_sleep_enable_timer_wakeup(5 * 1000000);
        esp_deep_sleep_start();
        return;
    }

    if (strncmp(event->topic, MQTT_TOPIC_OTA, event->topic_len) == 0){
        ESP_LOGI(TAG, "OTA triggered!");
        xTaskCreate(firmware_update_task, "firmware_update_task", 8192, NULL, 1, NULL);
        return;
    }

    if (strncmp(event->topic, MQTT_TOPIC_RESET, event->topic_len) == 0){
        ESP_LOGI(TAG, "WiFi credentials reset triggered!");
        reset_creds();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        esp_restart();
        return;
    }

    ESP_LOGI(TAG, "UNHANDLED MQTT_EVENT_DATA");
    printf("%.*s", event->topic_len, event->topic);
    printf(" %.*s\r\n", event->data_len, event->data);

}

static void mqtt_send_data(const char *topic, float value){

    char buffer[12];
    sprintf(buffer, "%.2f", value);
    esp_mqtt_client_publish(mqtt_client,topic,buffer,0,0,0);
}

void mqtt_subscriber(void *pvParameters){

    while(1)
    {
            vTaskDelay(1000 / portTICK_RATE_MS);
            esp_mqtt_client_subscribe(mqtt_client, MQTT_TOPIC_USER_RELAY, 0);
            esp_mqtt_client_subscribe(mqtt_client, MQTT_TOPIC_TEMP_SETPOINT, 0);
            esp_mqtt_client_subscribe(mqtt_client, MQTT_TOPIC_SLEEP, 0);
            esp_mqtt_client_subscribe(mqtt_client, MQTT_TOPIC_OTA, 0);
            esp_mqtt_client_subscribe(mqtt_client, MQTT_TOPIC_RESET, 0);

            while (!WIFI_CONNECTED){
                vTaskDelay(1000 / portTICK_RATE_MS);
                
            };
    }
    
}


void mqtt_publisher(void *pvParameters) {

    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_URL,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    mqtt_event_group = xEventGroupCreate();
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, mqtt_client);
    
    esp_mqtt_client_start(mqtt_client);
    xEventGroupWaitBits(mqtt_event_group, BIT0, false, true, portMAX_DELAY);


    while (1) {
            if (WIFI_CONNECTED) {
                mqtt_send_data(MQTT_TOPIC_TEMP, temp);
                mqtt_send_data(MQTT_TOPIC_CTRL_RELAY, relay_channel1);
                mqtt_send_data(MQTT_TOPIC_EXT_ADC, ext_adc);
                mqtt_send_data(MQTT_TOPIC_RTOS_FREE_MEM, xPortGetFreeHeapSize() / 1024.f);
                mqtt_send_data(MQTT_TOPIC_RTOS_MIN_MEM, xPortGetMinimumEverFreeHeapSize() / 1024.f);

                ESP_LOGI(TAG,"data submited @ temp = %.02f", temp);
            }
            vTaskDelay(5000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}