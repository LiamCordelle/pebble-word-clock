#pragma once
#include "lab.h"
typedef struct { uint8_t preamble_lit, caption_mode, light_theme; } FaceSettings;
FaceSettings face_settings_get(void);
