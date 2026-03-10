idf_component_register(
    SRCS "pwm.c" "lv_port.c" "hwDataAccess.c"
    INCLUDE_DIRS    "."
    REQUIRES    "driver"  "lvgl"  "bsp" "esp_adc"
)