#include "controler.h"
#include "net.h"
#include "main.h"

static const char* TAG = "ctrl";

static uint16_t buffer;
static spi_device_handle_t spi;
static esp_adc_cal_characteristics_t adc_cal;

static spi_transaction_t spi_trans = {
    .tx_buffer = NULL,
    .rx_buffer = &buffer,
    .length = 16,
    .rxlength = 16,
};

static void spi_init() {
    spi_bus_config_t buscfg = {
        .miso_io_num =  THERMOCOUPLE_MISO_PIN,
        .sclk_io_num = THERMOCOUPLE_SCK_PIN,
        .mosi_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = (4 * 8)
    };

    spi_device_interface_config_t devCfg={
        .mode = 0,
        .clock_speed_hz = 1000000,
        .spics_io_num=THERMOCOUPLE_CS_PIN,
        .queue_size=3
    };

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devCfg, &spi);
}

static void read_thermocouple(float *temp, bool *valid){

    spi_device_transmit(spi, &spi_trans);
    int16_t raw = (int16_t) SPI_SWAP_DATA_RX(buffer, 16);
    *valid = 0x7FFC != raw;
    raw >>= 3;
    *temp = raw * 0.25;
}
 
float ble_temp_read(void){

    spi_init();
    float temp;
    bool valid;
    
    read_thermocouple(&temp, &valid);

    return valid? temp : 0.0;
}


void controler_task(void *pvParameters) {

    bool valid;
    spi_init();

    gpio_set_direction(RELAY_CH0_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY_CH1_PIN, GPIO_MODE_OUTPUT);

    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc_cal);

    while(1) {
        read_thermocouple(&temp, &valid);

        gpio_set_level(8, !WIFI_CONNECTED);   

        if (!valid){
            ESP_LOGE(TAG, "temp invalid");
            relay_channel1 = false;
        }
        else{
            relay_channel1 = temp < temp_setpoint;
        }

        gpio_set_level(RELAY_CH0_PIN, relay_channel0);   
        gpio_set_level(RELAY_CH1_PIN, relay_channel1);

        ext_adc = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_3), &adc_cal) / 1000.0;
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    vTaskDelete(NULL);

}


