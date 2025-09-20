idf_component_register(SRCS "sensor.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_adc driver
                    PRIV_REQUIRES ssd1306
                    )
