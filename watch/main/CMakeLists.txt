idf_component_register(
    SRCS "main.c" 
         "tasks/user_taskInit.c" 
         "tasks/user_runModeTask.c"
         "tasks/user_hardwareInitTask.c"
         
    INCLUDE_DIRS "." "tasks"
)
