# Copilot / AI Agent Instructions for watch (ESP-IDF + LVGL)

This repository is an ESP-IDF project targeting an ESP32-S3-based watch hardware. Keep guidance concise, actionable and rooted in files below.

- **Project type:** ESP-IDF (CMake) project. Top-level build files: `CMakeLists.txt`, `sdkconfig`, `main/CMakeLists.txt`.
- **Key components:** `components/bsp` (display, touch drivers), `components/utils` (LVGL port glue), `components/lvgl` (LVGL library and examples), `main/main.c` (app entry).

- **Big picture:**
  - Boot and init flow: `app_main()` in `main/main.c` calls `lv_port_init()` (in `components/utils/lv_port.c`) then enables display backlight via the ST7789 driver (`components/bsp/st7789_driver.*`) and starts FreeRTOS tasks.
  - UI is provided by LVGL integrated under `components/lvgl`; platform glue lives in `components/utils`.
  - Cross-component interactions use C headers in `components/*` and FreeRTOS tasks/queues; typical pattern: hardware init in BSP, lv_port creates timers/driver hooks, main loop calls `lv_task_handler()`.

- **Build / flash / debug (exact commands):**
  - Build: `idf.py build`
  - Flash: `idf.py -p <PORT> flash` (replace `<PORT>` with system COM port on Windows, e.g. `COM3`)
  - Monitor: `idf.py -p <PORT> monitor`
  - Size analysis: `idf.py size-app` or use the provided "ESP-IDF Size" terminal task
  - Note: project vendor ESP-IDF is referenced in `build/project_description.json` ¡ª use the same ESP-IDF version as configured by the developer environment (see `build/config` and `CMakeCache.txt`).

- **Where to look for examples and patterns:**
  - LVGL integration: `components/utils/lv_port.h` / `components/utils/lv_port.c` (calls `st7789_driver_hw_init()`)
  - Display driver: `components/bsp/st7789_driver.h` / `st7789_driver.c`
  - Application entry: `main/main.c` demonstrates `lv_port_init()`, enabling backlight and creating tasks via `xTaskCreatePinnedToCore(...)` and `lv_task_handler()` loop.

- **Component conventions:**
  - Device drivers and board-specific code go under `components/bsp` with standard component CMake files.
  - Platform glue and utilities live in `components/utils`.
  - Third-party libs (LVGL) are included under `components/lvgl` and use their own idf_component.yml / CMake files.
  - Include paths and component order can be inspected in `build/compile_commands.json` and `build/project_description.json` if needed.

- **Code patterns to follow when modifying code:**
  - Use `ESP_LOGI/ESP_LOGE` macros for logs; common pattern: define `#define TAG "MAIN"` at top of C files.
  - Follow existing task creation style (FreeRTOS): `xTaskCreatePinnedToCore(StartDefaultTask, "defaultTask", 4096, NULL, 5, &handle, 1)`.
  - Hardware init is centralized in BSP drivers; prefer adding new hardware support under `components/bsp` and exposing a simple init function.

- **Integration points / pitfalls discovered:**
  - LVGL requires correct tick/flush integration ¡ª check `lv_port.c` for how `lv_tick_inc()` and the display flush callback are wired.
  - `sdkconfig` is present and authoritative for build-time options. When adding features, expose menuconfig options instead of hardcoding constants.
  - Serial port and flash settings are environment-specific; do not hardcode COM ports in scripts.

- **Testing & CI hints:**
  - There are no explicit unit tests in the repo. For CI builds, run `idf.py build` in a clean environment matching the project's ESP-IDF version.

If any area is unclear or you'd like more detail (examples of adding a component, wiring LVGL flush, or a sample `idf.py` debug sequence), tell me which part to expand.
