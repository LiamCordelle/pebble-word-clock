#pragma once
#include <pebble.h>
#define MESSAGE_KEY_PREAMBLE_LIT 12
#define MESSAGE_KEY_CAPTION_MODE 13
#define MESSAGE_KEY_LIGHT_THEME 14
int persist_get_size(uint32_t key);
int persist_read_data(uint32_t key,void *data,size_t size);
int persist_write_data(uint32_t key,const void *data,size_t size);
