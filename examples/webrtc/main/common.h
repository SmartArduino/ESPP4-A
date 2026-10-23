#pragma once

#include "esp_webrtc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VIDEO_WIDTH  1920
#define VIDEO_HEIGHT 1080
#define VIDEO_FPS    25

/** Initialize the media system. */
int media_sys_buildup(void);

/** Get the media sources. */
int media_sys_get_provider(esp_webrtc_media_provider_t *provide);

/** Start WebRTC. */
int start_webrtc(void);

/** Print WebRTC status. */
void query_webrtc(void);

/** Stop WebRTC. */
int stop_webrtc(void);

#ifdef __cplusplus
}
#endif
