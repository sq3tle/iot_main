#include "net.h"

static const char *TAG = "network";
static EventGroupHandle_t wifi_event_group;
bool WIFI_CONNECTED = false;

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data) {
    static int retries;
    if (event_base == WIFI_PROV_EVENT) {
        switch (event_id) {
            case WIFI_PROV_START:
                ESP_LOGI(TAG, "Provisioning started");
                break;
            case WIFI_PROV_CRED_RECV: {
                wifi_sta_config_t *wifi_sta_cfg = (wifi_sta_config_t *)event_data;
                ESP_LOGI(TAG, "Received Wi-Fi credentials"
                         "\n\tSSID     : %s\n\tPassword : %s",
                         (const char *) wifi_sta_cfg->ssid,
                         (const char *) wifi_sta_cfg->password);
                break;
            }
            case WIFI_PROV_CRED_FAIL: {
                retries++;
                if (retries >= 3) {
                    ESP_LOGI(TAG, "Failed to connect with provisioned AP, reseting provisioned credentials");
                    retries = 0;
                    wifi_prov_mgr_reset_sm_state_on_failure();
                }
                break;
            }
            case WIFI_PROV_CRED_SUCCESS:
                ESP_LOGI(TAG, "Provisioning successful");
                retries = 0;
                break;
            case WIFI_PROV_END:
                wifi_prov_mgr_deinit();
                break;
            default:
                break;
        }
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, BIT0);
        WIFI_CONNECTED = true;
        retries = 0;

    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        WIFI_CONNECTED = false;
        retries++;
        if (retries >= 3) {
                    ESP_LOGI(TAG, "Failed to connect with saved AP");
                    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));
                    provision();
                    retries = 0;
                }
        esp_wifi_connect();
    }
}

static void wifi_init_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = PROVISION_PREFIX;
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X", ssid_prefix, eth_mac[5]);
}

void provision(void){
        char service_name[12];

        wifi_prov_mgr_config_t config = {
            .scheme = wifi_prov_scheme_ble,
            .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
        };

        wifi_prov_mgr_init(config);
                
        get_device_service_name(service_name, sizeof(service_name));
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(WIFI_PROV_SECURITY_1, NULL, service_name, NULL));
        ESP_LOGI(TAG, "Starting provisioning");
}

void init_network(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    bool provisioned = false;

    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM
    };

    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));
    wifi_prov_mgr_deinit(); //not elegant - fix if posible

    if (!provisioned) {
        provision();
    } 
    else {
        ESP_LOGI(TAG, "Already provisioned, starting Wi-Fi");
        wifi_prov_mgr_deinit();
        wifi_init_sta();
    }

    xEventGroupWaitBits(wifi_event_group, BIT0, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Finished network setup");
    vTaskDelay(500 / portTICK_RATE_MS);
}
