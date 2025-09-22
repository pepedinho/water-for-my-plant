#include "freertos/FreeRTOS.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "ssd1306.h"
#include "esp_log.h"
#include "driver/ledc.h"

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


#define SERVO_PIN 18          // servo pin (PWM)
#define SERVO_FORWARD 3277 // rotation sens A
#define SERVO_BACKWARD 6553 // rotation sens B
#define SERVO_STOP 4915  

void move_xms(int duration_ms) {
    // ouverture
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, SERVO_FORWARD);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    // stop
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, SERVO_STOP);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void return_xms(int duration_ms) {
    // retour
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, SERVO_BACKWARD);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    // stop
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, SERVO_STOP);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void display_msg(char *msg, int line,  SSD1306_t *dev, bool f) {
    // ssd1306_clear_screen(dev, false);
    ssd1306_display_text(dev, line, "                ", 16, false); // clear current line
    ssd1306_display_text(dev, line, msg, strlen(msg), false);
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
    display_msg(msg, 0, dev, 0);
}

void app_main(void) {
    SSD1306_t dev;

    i2c_master_init(&dev, OLED_SDA, OLED_SCL, OLED_ADDR);

    // init écran
    ssd1306_init(&dev, SCREEN_WIDTH, SCREEN_HEIGHT);

    adc_init();

        ledc_timer_config_t timer_conf = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_16_BIT, // Résolution 16 bits
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 50, // fréquence PWM pour servo
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t channel_conf = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO_PIN,
        .duty           = 0,   // duty initial
        .hpoint         = 0
    };
    ledc_channel_config(&channel_conf);


    ssd1306_clear_screen(&dev, false);
    while (1) {
        display_soil_value(&dev);
        display_msg("run servo",1 ,&dev, false);
        move_xms(450);
        display_msg("arrose...",1 ,&dev, false);
        vTaskDelay(pdMS_TO_TICKS(5000));
        display_msg("reset servo", 1,&dev, false);
        return_xms(450);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
