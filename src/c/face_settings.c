#include "face_settings.h"
#ifdef LAB_HOST_RENDER
#include <stdlib.h>
static int preview(const char *key,int max) {
  const char *v=getenv(key);return v ? lab_clamp(atoi(v),0,max) : 0;
}
FaceSettings face_settings_get(void) {
  return (FaceSettings){preview("WORDCLOCK_PREAMBLE_LIT",1),
    preview("WORDCLOCK_CAPTION_MODE",2),preview("WORDCLOCK_LIGHT_THEME",1)};
}
#else
#include "kit/kit.h"
#define FACE_SETTINGS_KEY 2000
static FaceSettings s_face;
FaceSettings face_settings_get(void) { return s_face; }
static bool valid(FaceSettings s) {
  return s.preamble_lit<=1 && s.caption_mode<=2 && s.light_theme<=1;
}
void kit_consumer_settings_init(void) {
  FaceSettings value;
  if (persist_get_size(FACE_SETTINGS_KEY)==sizeof(value) &&
      persist_read_data(FACE_SETTINGS_KEY,&value,sizeof(value))==sizeof(value) && valid(value)) s_face=value;
}
bool kit_consumer_settings_receive(DictionaryIterator *iter) {
  int32_t p,c,l;
  if(!kit_tuple_int(iter,MESSAGE_KEY_PREAMBLE_LIT,&p) ||
     !kit_tuple_int(iter,MESSAGE_KEY_CAPTION_MODE,&c) ||
     !kit_tuple_int(iter,MESSAGE_KEY_LIGHT_THEME,&l)) return false;
  if(p<0||p>1||c<0||c>2||l<0||l>1)return false;
  FaceSettings next={p,c,l};
  if(memcmp(&s_face,&next,sizeof(next))==0)return false;
  s_face=next;
  if(persist_write_data(FACE_SETTINGS_KEY,&next,sizeof(next))!=sizeof(next))
    APP_LOG(APP_LOG_LEVEL_WARNING,"Face settings could not be saved");
  APP_LOG(APP_LOG_LEVEL_INFO,"Face settings applied: preamble=%ld caption=%ld light=%ld",(long)p,(long)c,(long)l);
  return true;
}
#endif
