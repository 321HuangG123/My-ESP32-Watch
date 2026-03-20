idf_component_register(
    SRCS            "user_ChargCheckTask.c"     "user_DataSaveTask.c"       "user_hardwareInitTask.c"
                    "user_KeyTask.c"            "user_runModeTask.c"        "user_ScrRenewTask.c"
                    "user_SensUpdateTask.c"     "user_taskInit.c"
                    
    INCLUDE_DIRS    "."
    REQUIRES    "driver"  "lvgl"  "bsp"  "ui" "utils"
)