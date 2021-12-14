#include "math.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMono18pt7b.h> // for large number display
#include <Fonts/Picopixel.h>      // good for small (5px high)
// The default font should be good for drawing basic stuff

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

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


Adafruit_SSD1306 display(2); // in fact, the reset pin is not there


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

  //leds_update();

}

void loop() {
  audio_refresh_data();
  
  display_draw();
  /*
  float v = abs(1600 - ((long) millis() % 3200)) / 2000.0;
  temp_level_l = v;
  temp_level_r = max(0, v - 0.1);
  temp_peak_l = min(temp_level_l + 0.15, 1.0);
  temp_peak_r = min(temp_level_r + 0.15, 1.0);
  */
  leds_set_value(
    level_to_db(level_l), level_to_db(level_r),
    level_to_db(peak_l), level_to_db(peak_r)
  );
  delay(32 - (millis() % 32));
}
