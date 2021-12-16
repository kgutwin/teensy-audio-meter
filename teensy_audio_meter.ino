#include "math.h"


// The following are all physical pin numbers
#define PT8211_WS  20  // A6
#define PT8211_BCK 21  // A7
#define PT8211_DIN 7   // RX3

#define SW_1       3   // Left
#define SW_2       4   // Right
#define SW_3       5   // Menu

#define D_SDA      18  // SDA0
#define D_SCL      19  // SCL0

#define SR_CLK     13  // SCK
#define SR_SER     11  // DOUT
#define SR_RCK     8   // TX3


enum {
  DB_BIGNUM,
} main_state;


float level_l;
float level_r;
float peak_l;
float peak_r;

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

elapsedMillis fps;

void loop() {
  if (fps > 32) {
    fps = 0;
    audio_refresh_data();
  
    display_draw();

    leds_set_value(
      level_to_db(level_l), level_to_db(level_r),
      level_to_db(peak_l), level_to_db(peak_r)
    );
  }
}
