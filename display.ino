// TODO: move fonts and display global variable to this file

void display_init() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();

  display.setTextColor(WHITE);
  display.setTextSize(1);

}

void display_draw() {
  display.clearDisplay();
  display.setFont();
  display.setCursor(0,0);
  switch (main_state) {
    case DB_BIGNUM:
      display_draw_db_bignum();
      display.print("dB");
      break;
    default:
      display.print("UNKNOWN");
  }
  display.display();
}

void display_draw_db_bignum() {
  // TODO: what does the bigger size of the normal text size look like?
  // TODO: refactor this to draw arbitrary bignums
  display.setFont(&FreeMono18pt7b);
  display.setCursor(20, 50);
  float temp_db = level_to_db((peak_l + peak_r) / 2.0);
  if (temp_db == -INFINITY) {
    display.print("  -oo");
  } else {
    if (temp_db > -10.0) display.print(" ");
    display.print(temp_db, 1);
  }
}

