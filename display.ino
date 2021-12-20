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




Adafruit_SSD1306 display(2); // in fact, the reset pin is not connected

float display_fft_db[16];

void display_init() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(1);

}

void display_draw() {
  // TODO: add display timeout that checks peak_l/peak_r
  display.clearDisplay();
  const char* screen_title = "";
  switch (main_state) {
    case DB_BIGNUM:
      screen_title = "dB";
      //display_draw_db_bignum(level_to_db((peak_l + peak_r) / 2.0));
      display_draw_db_bignum(level_to_db(max(peak_l, peak_r)));
      break;
    case FFT_BARS:
      screen_title = "Spectrum";
      display_update_fft_bars();
      display_draw_bars_db(display_fft_db, 16, -35.0);
      break;
    case LUFS_M_BIGNUM:
      screen_title = "LUFS";
      display_draw_db_bignum(lufs_momentary);
      break;
    case STEREO_STEREO:
      screen_title = "Stereo";
      display_draw_stereo();
      break;
    default:
      screen_title = "UNKNOWN";
  }
  display.setFont();
  display.setCursor(0, 0);
  display.setTextColor(WHITE, BLACK);
  display.print(screen_title);
  if (! audio_connected) {
    display.setCursor(56, 0);
    display.print("DISCONNECTED");
  }
  display.display();
}


void display_draw_infinity(int16_t x0, int16_t y0, int16_t r, int16_t w, uint16_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  display.startWrite();
  display.writePixel(x0, y0 + r, color);
  display.writePixel(x0, y0 - r, color);
  display.writePixel(x0 - r, y0, color);
  display.writePixel(x0 + r + w, y0, color);
  display.writePixel(x0 + w, y0 + r, color);
  display.writePixel(x0 + w, y0 - r, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    display.writePixel(x0 + x, y0 + y, color);
    display.writePixel(x0 - x, y0 + y, color);
    display.writePixel(x0 + x, y0 - y, color);
    display.writePixel(x0 - x, y0 - y, color);
    display.writePixel(x0 - y, y0 + x, color);
    display.writePixel(x0 - y, y0 - x, color);

    display.writePixel(x0 + x + w, y0 + y, color);
    display.writePixel(x0 - x + w, y0 + y, color);
    display.writePixel(x0 + x + w, y0 - y, color);
    display.writePixel(x0 - x + w, y0 - y, color);
    display.writePixel(x0 + y + w, y0 - x, color);
    display.writePixel(x0 + y + w, y0 + x, color);
  }
  display.writeLine(x0 + x, y0 + y, x0 + w - x, y0 - y, color);
  display.writeLine(x0 + x, y0 - y, x0 + w - x, y0 + y, color);
  display.endWrite();
}


void display_draw_db_bignum(float db) {
  display.setFont(&FreeMono18pt7b);
  display.setCursor(20, 50);
  if (db == -INFINITY) {
    //display.print("  -oo");
    display.print("  -");
    display_draw_infinity(94, 41, 8, 20, WHITE);
  } else {
    if (db > -10.0) display.print(" ");
    display.print(db, 1);
  }
}


void display_draw_hexnum(uint32_t v) {
  display.setCursor(20, 50);
  display.print(v, HEX);
}


void display_draw_numbers(float* values, int n_values) {
  display.setCursor(0, 10);
  for (int i=0; i<(n_values / 4); i++) {
    for (int j=0; j<4; j++) {
      display.print(values[i*3 + j], 2);
      display.print(" ");
    }
    display.print("\n");
  }
}

void display_draw_bars_db(float* dbs, int num_values, const float lowest_db) {
  const int bar_width = display.width() / num_values;
  const int bar_scale_h = display.height();

  const int num_lines = 6;
  const int line_height = display.height() / num_lines;

  for (int i=0; i<num_values; i++) {
    int bar_height = (max(0, dbs[i] - lowest_db) / -lowest_db) * bar_scale_h;
    
    display.fillRect(
      i * bar_width, display.height() - bar_height,
      bar_width - 1, bar_height,
      WHITE
    );
    
    // draw pixels for horizontal lines
    for (int j=1; j<num_lines; j++) {
      display.drawPixel(
        (i * bar_width) + (bar_width - 1), 
        (j * line_height), WHITE
      );
    }
  }

  // draw db scale
  display.setFont();
  display.setTextColor(WHITE);
  for (int i=1; i<num_lines; i++) {
    display.setCursor(0, i*line_height);
    display.print((lowest_db / num_lines) * i, 0);
  }
}

#define DECAY_RATE_DB_PER_MSEC  (12.0 / 1000.0)
elapsedMillis display_fft_bars_timer;

void display_update_fft_bars() {
  const float db_decay = display_fft_bars_timer * DECAY_RATE_DB_PER_MSEC;
  display_fft_bars_timer = 0;
  
  for (int i=0; i<16; i++) {
    float instant_db = level_to_db(fft_level[i]);
    if (instant_db >= display_fft_db[i]) {
      display_fft_db[i] = instant_db;
    } else {
      display_fft_db[i] -= db_decay;
    }
  }
}


void display_draw_stereo() {
  // correlation bar is 60 px wide with 2px margin on either side
  int bar_width = stereo_correlation * 30;
  if (bar_width == 0) {
    display.drawFastVLine(32, 28, 8, WHITE);
  } else if (bar_width < 0) {
    display.fillRect(32 + bar_width, 28, -bar_width, 8, WHITE);
  } else {
    display.fillRect(32, 28, bar_width, 8, WHITE);
  }

  display.drawFastVLine(32, 23, 4, WHITE);
  display.drawFastVLine(32, 38, 4, WHITE);
  display.drawFastVLine(62, 23, 4, WHITE);
  display.drawFastVLine(62, 38, 4, WHITE);
  display.drawFastVLine(2, 23, 4, WHITE);
  display.drawFastVLine(2, 38, 4, WHITE);
  
  display.setFont();
  display.setTextColor(WHITE);
  display.setCursor(18, 50);
  if (stereo_correlation >= 0.0) display.print(" ");
  display.print(stereo_correlation, 2); 

  // scatter
  for (int i=8; i<64; i+=4) {
    display.drawPixel(64 + i, i, WHITE);
    display.drawPixel(128 - i, i, WHITE);
  }
  for (int i=0; i<AUDIO_BLOCK_SAMPLES; i++) {
    display.drawPixel(
      stereo_px_r[i] - stereo_px_l[i] + 96, 
      stereo_px_r[i] + stereo_px_l[i] + 32, 
      WHITE
    );
  }
  display.setCursor(64, 0);
  display.print("L");
  display.setCursor(122, 0);
  display.print("R");
}

