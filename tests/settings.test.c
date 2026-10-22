#include "settings_api.h"
#include "../src/c/face_settings.h"
#include <assert.h>
#include <stdio.h>
struct DictionaryIterator {int32_t p,c,l;bool complete;};
static FaceSettings saved;
static int saved_size,writes;
int persist_get_size(uint32_t key){assert(key==2000);return saved_size;}
int persist_read_data(uint32_t key,void *data,size_t size){assert(key==2000);memcpy(data,&saved,size);return size;}
int persist_write_data(uint32_t key,const void *data,size_t size){assert(key==2000);memcpy(&saved,data,size);saved_size=size;writes++;return size;}
bool kit_tuple_int(DictionaryIterator *iter,uint32_t key,int32_t *out) {
  if(!iter->complete)return false;
  *out=key==12 ? iter->p : key==13 ? iter->c : iter->l;return true;
}
void kit_consumer_settings_init(void);
bool kit_consumer_settings_receive(DictionaryIterator *iter);
int main(void) {
  kit_consumer_settings_init();assert(face_settings_get().caption_mode==0);
  DictionaryIterator message={1,2,1,true};
  assert(kit_consumer_settings_receive(&message));assert(writes==1);
  assert(face_settings_get().light_theme==1);
  assert(!kit_consumer_settings_receive(&message));assert(writes==1);
  message.c=3;assert(!kit_consumer_settings_receive(&message));assert(writes==1);
  message.c=-1;assert(!kit_consumer_settings_receive(&message));assert(writes==1);
  message.c=1;message.complete=false;assert(!kit_consumer_settings_receive(&message));assert(writes==1);
  // A later valid snapshot updates atomically, then a restart restores it.
  message=(DictionaryIterator){0,1,0,true};assert(kit_consumer_settings_receive(&message));
  saved=(FaceSettings){1,2,1};saved_size=sizeof(saved);kit_consumer_settings_init();
  assert(face_settings_get().caption_mode==2 && face_settings_get().preamble_lit==1);
  saved=(FaceSettings){0,255,0};kit_consumer_settings_init();assert(face_settings_get().caption_mode==2);
  puts("Face settings passed: defaults, atomic validation, idempotence, persistence and corrupt saves.");
}
