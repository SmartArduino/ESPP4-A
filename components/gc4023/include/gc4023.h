#pragma once

#include "esp_cam_sensor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GC4023_SCCB_ADDR   0x29
#define GC4023_PID         0x4023
#define GC4023_SENSOR_NAME "GC4023"

esp_cam_sensor_device_t *gc4023_detect(esp_cam_sensor_config_t *config);

#ifdef __cplusplus
}
#endif
