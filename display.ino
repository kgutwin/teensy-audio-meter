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


void display_init() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(1);

}

void display_draw() {
  display.clearDisplay();
  switch (main_state) {
    case DB_BIGNUM:
      display_draw_db_bignum();
      display.setFont();
      display.setCursor(0,0);
      display.print("dB");
      break;
    default:
      display.print("UNKNOWN");
  }
  if (! audio_connected) {
    display.setFont();
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


void display_draw_db_bignum() {
  // TODO: what does the bigger size of the normal text size look like?
  // TODO: refactor this to draw arbitrary bignums
  display.setFont(&FreeMono18pt7b);
  display.setCursor(20, 50);
  float temp_db = level_to_db((peak_l + peak_r) / 2.0);
  if (temp_db == -INFINITY) {
    //display.print("  -oo");
    display.print("  -");
    display_draw_infinity(94, 41, 9, 18, WHITE);
  } else {
    if (temp_db > -10.0) display.print(" ");
    display.print(temp_db, 1);
  }
}

