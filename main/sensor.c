#include "freertos/FreeRTOS.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "ssd1306.h"
#include "esp_log.h"

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define TAG "SOIL_SENSOR"
#define SOIL_SENSOR_ADC_CHANNEL ADC_CHANNEL_2 
#define ADC_UNIT_USED ADC_UNIT_2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


adc_oneshot_unit_handle_t adc_handle;

void display_msg(char *msg, SSD1306_t *dev, bool f) {
    // ssd1306_clear_screen(dev, false);
    ssd1306_display_text(dev, 0, "                ", 16, false); // clear current line
    ssd1306_display_text(dev, 0, msg, strlen(msg), false);
    if (f)
        free(msg);
}

void adc_init() {
    adc_oneshot_unit_init_cfg_t adc_cfg = {
        .unit_id = ADC_UNIT_USED,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, SOIL_SENSOR_ADC_CHANNEL, &chan_cfg));
}

int read_soil_adc() {
    int raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, SOIL_SENSOR_ADC_CHANNEL, &raw));
    ESP_LOGI(TAG, "READ : %d", raw);
    return raw;
}

float read_soil_voltage() {
    int raw = read_soil_adc();
    return raw / 4095.0f * 3.3f;
}

void display_soil_value(SSD1306_t *dev) {
    float voltage = read_soil_voltage();
    float min_voltage = 0.36;  // dry
    float max_voltage = 0.59;  // humid

    float humidity = (max_voltage - voltage) / (max_voltage - min_voltage) * 100.0f;

    if (humidity < 0) humidity = 0;
    if (humidity > 100) humidity = 100;

    char msg[32];
    snprintf(msg, sizeof(msg), "water: %.2f %%", humidity);
    display_msg(msg, dev, 0);
}

void app_main(void) {
    SSD1306_t dev;

    i2c_master_init(&dev, OLED_SDA, OLED_SCL, OLED_ADDR);

    // init écran
    ssd1306_init(&dev, SCREEN_WIDTH, SCREEN_HEIGHT);

    adc_init();

    while (1) {
        display_soil_value(&dev);
        vTaskDelay(200);
    }
    display_msg("18%", &dev, 0);
}