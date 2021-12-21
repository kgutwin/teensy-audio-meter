#include "math.h"
#include <Bounce.h>

// The following are all physical pin numbers
#define PT8211_WS  20  // A6
#define PT8211_BCK 21  // A7
#define PT8211_DIN 7   // RX3

#define SW_1       3   // Left
#define SW_2       4   // Right
#define SW_3       5   // Menu
Bounce button_l = Bounce(SW_1, 10);
Bounce button_r = Bounce(SW_2, 10);
Bounce button_m = Bounce(SW_3, 10);

#define D_SDA      18  // SDA0
#define D_SCL      19  // SCL0

#define SR_CLK     13  // SCK
#define SR_SER     11  // DOUT
#define SR_RCK     8   // TX3


enum {
  DB_BIGNUM,
  FFT_BARS,
  LUFS_M_BIGNUM,
  LUFS_M_BARS,
  STEREO_STEREO,
  MAIN_STATE_MAX_ITEMS   // must be last!
} main_state;


bool audio_connected;

float level_l;
float level_r;
float peak_l;
float peak_r;
float fft_level[16];
float stereo_correlation;
int8_t stereo_px_r[AUDIO_BLOCK_SAMPLES];
int8_t stereo_px_l[AUDIO_BLOCK_SAMPLES];
float lufs_momentary;

float level_to_db(float lvl) {
  return 20.0 * log10f(lvl);
}


void setup() {
  pinMode(SR_CLK, OUTPUT);
  pinMode(SR_SER, OUTPUT);
  pinMode(SR_RCK, OUTPUT);
  digitalWrite(SR_CLK, 0);
  digitalWrite(SR_SER, 0);
  digitalWrite(SR_RCK, 0);
  leds_init();

  pinMode(SW_1, INPUT_PULLUP);
  pinMode(SW_2, INPUT_PULLUP);
  pinMode(SW_3, INPUT_PULLUP);

  display_init();

  audio_setup();

  main_state = DB_BIGNUM;

}

elapsedMillis ms_per_frame;

void loop() {
  if (button_m.update()) {
    if (button_m.fallingEdge()) {
      // TODO: implement this in a way that doesn't raise a compiler warning
      main_state = (main_state + 1) % MAIN_STATE_MAX_ITEMS;
    }
  }
  
  if (ms_per_frame > 42) {
    ms_per_frame = 0;
    audio_refresh_data();
  
    display_draw();

    leds_set_value(
      level_to_db(level_l), level_to_db(level_r),
      level_to_db(peak_l), level_to_db(peak_r)
    );
  }
}
