idf_component_register(
    SRCS            "adc_power.c"       "hwDataAccess.c"    "PageManager.c"
                    "pubsub.c"          "pwm.c"             "StrCalculate.c"
    INCLUDE_DIRS    "."
    REQUIRES    "driver"  "lvgl"  "bsp" "esp_adc" "ui"
)