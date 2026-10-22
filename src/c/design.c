#include "lab.h"
#include "kit/kit_calc.h"
#include "face_settings.h"

// Word Clock: a illuminated lit-letter grid (BRIEF.md section 8.7).
//
// 11x10 letters on a 16x19 px pitch, every glyph centred in its own cell.
// Three tones: the time's words are lit (white, bold), the constant preamble
// IT IS and the AM/PM marker are soft (light grey, bold), and every other
// letter is unlit (dim grey, regular). The four corner dots count the minutes
// past each five-minute step, clockwise from top left. Tiny captions above and
// below carry the date and today's steps (or a low battery).

#define COLS 11
#define ROWS 10
#define PITCH_X 16
#define PITCH_Y 19
#define GRID_X 12            // (200 - 11 * 16) / 2: the grid is exactly centred
#define INK_TOP 21           // first row's cap top; ink block is 9 * 19 + 14 = 185 px
#define G24B_INK_OFFSET 10   // Gothic 24 Bold draws its caps 10 px below the box top
#define G14B_INK_OFFSET 5    // Gothic 14 Bold caps: 5 px below the box top

#define TOP_BAND_MID 10      // vertical centre of the band above the grid
#define BOTTOM_BAND_MID 217  // mirrors TOP_BAND_MID from the bottom edge
#define DOT_INSET 10         // corner dot centre, from the screen edges
#define DOT_RADIUS 2

#define C_BG (face_settings_get().light_theme ? GColorWhite : GColorBlack)
#define C_DIM (face_settings_get().light_theme ? GColorFromHEX(0xaaaaaa) : GColorFromHEX(0x555555))
#define C_SOFT (face_settings_get().light_theme ? GColorFromHEX(0x555555) : GColorFromHEX(0xaaaaaa))
#define C_LIT (face_settings_get().light_theme ? GColorBlack : GColorWhite)

static const char GRID[ROWS][COLS + 1] = {
  "ITLISASAMPM",
  "ACQUARTERDC",
  "TWENTYFIVEX",
  "HALFSTENFTO",
  "PASTERUNINE",
  "ONESIXTHREE",
  "FOURFIVETWO",
  "EIGHTELEVEN",
  "SEVENTWELVE",
  "TENSEOCLOCK",
};

typedef struct {
  uint8_t row;
  uint8_t col;
  uint8_t len;
} Word;

static const Word W_IT = {0, 0, 2};
static const Word W_IS = {0, 3, 2};
static const Word W_AM = {0, 7, 2};
static const Word W_PM = {0, 9, 2};
static const Word W_A = {1, 0, 1};
static const Word W_QUARTER = {1, 2, 7};
static const Word W_TWENTY = {2, 0, 6};
static const Word W_FIVE = {2, 6, 4};
static const Word W_HALF = {3, 0, 4};
static const Word W_TEN = {3, 5, 3};
static const Word W_TO = {3, 9, 2};
static const Word W_PAST = {4, 0, 4};
static const Word W_OCLOCK = {9, 5, 6};

// Indexed by hour % 12.
static const Word HOURS[12] = {
  {8, 5, 6},   // TWELVE
  {5, 0, 3},   // ONE
  {6, 8, 3},   // TWO
  {5, 6, 5},   // THREE
  {6, 0, 4},   // FOUR
  {6, 4, 4},   // FIVE
  {5, 3, 3},   // SIX
  {8, 0, 5},   // SEVEN
  {7, 0, 5},   // EIGHT
  {4, 7, 4},   // NINE
  {9, 0, 3},   // TEN
  {7, 5, 6},   // ELEVEN
};

enum { LEVEL_DIM = 0, LEVEL_SOFT = 1, LEVEL_LIT = 2 };

static void light(uint8_t level[ROWS][COLS], Word w, uint8_t value) {
  for (int i = 0; i < w.len; i++) {
    level[w.row][w.col + i] = value;
  }
}

// Word-clock wording: the time is floored to its five-minute step and the
// remainder goes to the corner dots.
static void compose(uint8_t level[ROWS][COLS], int hour, int minute) {
  memset(level, LEVEL_DIM, ROWS * COLS);
  light(level, W_IT, face_settings_get().preamble_lit ? LEVEL_LIT : LEVEL_SOFT);
  light(level, W_IS, face_settings_get().preamble_lit ? LEVEL_LIT : LEVEL_SOFT);
  light(level, hour < 12 ? W_AM : W_PM, LEVEL_SOFT);

  int step = minute / 5;
  int shown_hour = step >= 7 ? hour + 1 : hour;
  switch (step) {
    case 0: light(level, W_OCLOCK, LEVEL_LIT); break;
    case 1: case 11: light(level, W_FIVE, LEVEL_LIT); break;
    case 2: case 10: light(level, W_TEN, LEVEL_LIT); break;
    case 3: case 9:
      light(level, W_A, LEVEL_LIT);
      light(level, W_QUARTER, LEVEL_LIT);
      break;
    case 4: case 8: light(level, W_TWENTY, LEVEL_LIT); break;
    case 5: case 7:
      light(level, W_TWENTY, LEVEL_LIT);
      light(level, W_FIVE, LEVEL_LIT);
      break;
    case 6: light(level, W_HALF, LEVEL_LIT); break;
  }
  if (step >= 1 && step <= 6) {
    light(level, W_PAST, LEVEL_LIT);
  } else if (step >= 7) {
    light(level, W_TO, LEVEL_LIT);
  }
  light(level, HOURS[shown_hour % 12], LEVEL_LIT);
}

static GColor level_color(uint8_t level) {
  if (level == LEVEL_LIT) {
    return C_LIT;
  }
  return level == LEVEL_SOFT ? C_SOFT : C_DIM;
}

// Lit letters are bold and unlit ones regular, so the lit words gain weight
// the way LED letters bloom on a real word clock.
static void draw_grid(GContext *ctx, uint8_t level[ROWS][COLS]) {
  GFont bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  GFont regular = fonts_get_system_font(FONT_KEY_GOTHIC_24);
  char glyph[2] = {0, 0};
  for (int r = 0; r < ROWS; r++) {
    int y = INK_TOP + r * PITCH_Y - G24B_INK_OFFSET;
    for (int c = 0; c < COLS; c++) {
      glyph[0] = GRID[r][c];
      lab_text(ctx, glyph, level[r][c] == LEVEL_DIM ? regular : bold,
               GRect(GRID_X + c * PITCH_X, y, PITCH_X, 28), GTextAlignmentCenter,
               level_color(level[r][c]));
    }
  }
}

// Corner dots, clockwise from top left: one per minute past the 5-minute step.
static void draw_minute_dots(GContext *ctx, GRect bounds, int extra) {
  const int right = bounds.size.w - 1 - DOT_INSET;
  const GPoint corners[4] = {
    GPoint(DOT_INSET, TOP_BAND_MID),
    GPoint(right, TOP_BAND_MID),
    GPoint(right, BOTTOM_BAND_MID),
    GPoint(DOT_INSET, BOTTOM_BAND_MID),
  };
  for (int i = 0; i < 4; i++) {
    graphics_context_set_fill_color(ctx, i < extra ? C_LIT : C_DIM);
    graphics_fill_circle(ctx, corners[i], DOT_RADIUS);
  }
}

static bool is_letter(char c) {
  return c >= 'A' && c <= 'Z';
}

// Gothic 14 Bold ink metrics, measured on Emery: caps have a 1 px bearing on
// each side of their advance, digits are 5 px of ink with no right bearing,
// and "4" alone starts at x = 0 (why "6,842" sets unevenly by default).
typedef struct {
  int8_t left;   // ink offset from the draw origin
  int8_t width;  // ink width
} Ink;

static Ink caption_ink(char c, GFont font) {
  switch (c) {
    case '1': return (Ink){1, 4};
    case '4': return (Ink){0, 5};
    case ',': return (Ink){0, 2};
    case '%': return (Ink){1, 10};
    case '-': return (Ink){1, 3};
    case '.': case ':': return (Ink){1, 2};
  }
  if (c >= '0' && c <= '9') {
    return (Ink){1, 5};
  }
  char glyph[2] = {c, 0};
  return (Ink){1, lab_text_size(glyph, font).w - 2};
}

// Ink-to-ink spacing: letters are tracked open to echo the grid, digits sit a
// little tighter, and a comma hugs the digit before it.
#define CAPTION_LETTER_GAP 3
#define CAPTION_DIGIT_GAP 2
#define CAPTION_WORD_GAP 7

static int caption_gap(char prev, char next, bool space) {
  if (space) {
    return CAPTION_WORD_GAP;
  }
  if (next == ',') {
    return 1;
  }
  return is_letter(prev) && is_letter(next) ? CAPTION_LETTER_GAP : CAPTION_DIGIT_GAP;
}

// Walks the caption once to measure (draw = false) or to draw it.
static int caption_pass(GContext *ctx, const char *text, GFont font, int x, int y,
                        GColor color, bool draw) {
  char glyph[2] = {0, 0};
  char prev = 0;
  bool space = false;
  int start = x;
  for (const char *p = text; *p; p++) {
    if (*p == ' ') {
      space = prev != 0;
      continue;
    }
    if (prev) {
      x += caption_gap(prev, *p, space);
    }
    Ink ink = caption_ink(*p, font);
    if (draw) {
      glyph[0] = *p;
      lab_text(ctx, glyph, font, GRect(x - ink.left, y, ink.width + 8, 18),
               GTextAlignmentLeft, color);
    }
    x += ink.width;
    prev = *p;
    space = false;
  }
  return x - start;
}

// Small caps caption whose ink is centred on the screen and on `ink_mid`.
static void draw_caption(GContext *ctx, GRect bounds, const char *text, int ink_mid,
                         GColor color) {
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  int width = caption_pass(ctx, text, font, 0, 0, color, false);
  int y = ink_mid - 4 - G14B_INK_OFFSET;
  caption_pass(ctx, text, font, (bounds.size.w - width) / 2, y, color, true);
}

static void draw_date(GContext *ctx, GRect bounds, const LabData *d) {
  if(face_settings_get().caption_mode==1) {
    draw_caption(ctx,bounds,"WORD CLOCK",TOP_BAND_MID,C_SOFT);return;
  }
  char date[16];
  snprintf(date, sizeof(date), "%s %d %s", lab_day_abbr(d->now.tm_wday), d->now.tm_mday,
           lab_month_abbr(d->now.tm_mon));
  lab_upper(date);
  draw_caption(ctx, bounds, date, TOP_BAND_MID, C_SOFT);
}

// Bottom caption: today's steps, which light up like the grid's words once the
// goal is reached. A low battery (below 20%, not charging) takes the slot in
// Signal Deck's amber/red, so it is only on screen when it needs attention.
#define LOW_BATTERY_PCT 20
static bool draw_warning(GContext *ctx, GRect bounds, const LabData *d) {
  char text[24];
  if (d->battery_pct < LOW_BATTERY_PCT && !d->charging) {
    snprintf(text, sizeof(text), d->bt_connected ? "BATTERY %d%%" : "%d%% PHONE LOST", d->battery_pct);
    draw_caption(ctx, bounds, text, BOTTOM_BAND_MID, lab_battery_color(d->battery_pct));
    return true;
  }
  if (d->charging) {
    snprintf(text,sizeof(text),d->bt_connected ? "CHARGING %d%%" : "CHARGING OFFLINE",d->battery_pct);
    draw_caption(ctx,bounds,text,BOTTOM_BAND_MID,C_LIT);return true;
  }
  if (!d->bt_connected) {
    draw_caption(ctx,bounds,"PHONE LOST",BOTTOM_BAND_MID,C_SOFT);return true;
  }
  return false;
}

static void draw_status(GContext *ctx,GRect bounds,const LabData *d) {
  if(draw_warning(ctx,bounds,d))return;
  char text[24];
  if(face_settings_get().caption_mode==1) {
    snprintf(text,sizeof(text),"%s %d %s",lab_day_abbr(d->now.tm_wday),d->now.tm_mday,lab_month_abbr(d->now.tm_mon));
    lab_upper(text);draw_caption(ctx,bounds,text,BOTTOM_BAND_MID,C_SOFT);return;
  }
  if(face_settings_get().caption_mode==2) {
    snprintf(text,sizeof(text),"BATTERY %d%%",d->battery_pct);
    draw_caption(ctx,bounds,text,BOTTOM_BAND_MID,C_SOFT);return;
  }
  if (!d->steps_ok) {
    draw_caption(ctx, bounds, "-- STEPS", BOTTOM_BAND_MID, C_SOFT);
    return;
  }
  char steps[12];
  lab_fmt_thousands(steps, sizeof(steps), d->steps);
  snprintf(text, sizeof(text), "%s STEPS", steps);
  bool goal = d->step_goal > 0 && d->steps >= d->step_goal;
  draw_caption(ctx, bounds, text, BOTTOM_BAND_MID, goal ? C_LIT : C_SOFT);
}

static void draw(GContext *ctx, GRect bounds, const LabData *d) {
  graphics_context_set_fill_color(ctx, C_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  uint8_t level[ROWS][COLS];
  compose(level, d->now.tm_hour, d->now.tm_min);
  draw_grid(ctx, level);
  draw_minute_dots(ctx, bounds, d->now.tm_min % 5);
  draw_date(ctx, bounds, d);
  draw_status(ctx, bounds, d);
}


// Each row's Gothic 24 bold cap band is 14px. Row ink starts at 30, 56,
// 82, 108, 138, 164 and 190; the last cap ends at 204, inside the 228px screen.
static void tracked(GContext *ctx,const char *text,GFont font,int x,int y,GColor color) {
  char glyph[2]={0,0};
  for(const char *p=text;*p;p++) {
    if(*p==' '){x+=6;continue;}
    glyph[0]=*p;
    int w=lab_text_size(glyph,font).w;
    lab_text(ctx,glyph,font,GRect(x,y,w+4,30),GTextAlignmentLeft,color);x+=w+1;
  }
}
static void detail_row(GContext *ctx,GRect bounds,const char *label,const char *value,int ink_y) {
  tracked(ctx,label,fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),12,ink_y-4,C_SOFT);
  lab_text(ctx,value,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
    GRect(85,ink_y-10,bounds.size.w-97,30),GTextAlignmentRight,C_LIT);
}
static void detail_time(char *out,size_t n,int minute,bool is_24h) {
  kit_format_event_time(out,n,minute,is_24h,0,KitMeridiemFull);
}
static void draw_detail(GContext *ctx,GRect bounds,const LabData *d) {
  graphics_context_set_fill_color(ctx,C_BG);graphics_fill_rect(ctx,bounds,0,GCornerNone);
  draw_caption(ctx,bounds,"HERE AND NOW",10,C_SOFT);
  char value[24],label[16];
  if(d->hr_ok)snprintf(value,sizeof(value),"%d",d->heart_rate);else snprintf(value,sizeof(value),"--");
  detail_row(ctx,bounds,"HEART",value,30);
  if(d->steps_ok)lab_fmt_thousands(value,sizeof(value),d->steps);else snprintf(value,sizeof(value),"--");
  detail_row(ctx,bounds,"STEPS",value,56);
  if(d->weather_ok)snprintf(value,sizeof(value),"%d.%d",d->uv_x10/10,d->uv_x10%10);else snprintf(value,sizeof(value),"--");
  detail_row(ctx,bounds,"UV",value,82);
  if(d->weather_ok) {
    int temp=d->temp_fahrenheit ? kit_temperature(d->temp,true) : d->temp;
    snprintf(value,sizeof(value),"%d %s",temp,d->temp_fahrenheit ? "F" : "C");
    snprintf(label,sizeof(label),"%s",lab_weather_text(d->weather_code));lab_upper(label);
  } else {snprintf(value,sizeof(value),"--");snprintf(label,sizeof(label),"NO WEATHER");}
  lab_text(ctx,value,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),GRect(12,98,60,30),GTextAlignmentLeft,C_LIT);
  lab_text(ctx,label,fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD),GRect(68,104,bounds.size.w-80,25),GTextAlignmentRight,C_LIT);
  bool tomorrow=d->sunrise_min<0 || d->minute_of_day>=d->sunrise_min;
  int rise=tomorrow ? d->tomorrow_sunrise_min : d->sunrise_min;
  if(d->weather_ok && rise>=0)detail_time(value,sizeof(value),rise,d->is_24h);
  else snprintf(value,sizeof(value),"--");
  detail_row(ctx,bounds,tomorrow && d->weather_ok && rise>=0 ? "RISE +1" : "RISE",value,138);
  bool set_tomorrow=d->sunset_min<0 || d->minute_of_day>=d->sunset_min;
  int set=set_tomorrow ? d->tomorrow_sunset_min : d->sunset_min;
  if(d->weather_ok && set>=0)detail_time(value,sizeof(value),set,d->is_24h);
  else snprintf(value,sizeof(value),"--");
  detail_row(ctx,bounds,set_tomorrow && d->weather_ok && set>=0 ? "SET +1" : "SET",value,164);
  snprintf(label,sizeof(label),"%s%s",d->world_label,d->world_day_delta>0 ? " +1" : d->world_day_delta<0 ? " -1" : "");
  detail_time(value,sizeof(value),d->world_hour*60+d->world_min,d->is_24h);
  detail_row(ctx,bounds,label,value,190);
  draw_warning(ctx,bounds,d);
}
const LabDesign design_wordclock = {"Word Clock", NULL, NULL, draw, draw_detail};
