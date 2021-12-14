#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
//#include <SD.h>
//#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputUSB            usb1;           //xy=151,285
AudioAnalyzeRMS          rms2;           //xy=432,387
AudioAnalyzePeak         peak1;          //xy=435,185
AudioAnalyzePeak         peak2;          //xy=435,340
AudioAnalyzeRMS          rms1;           //xy=436,228
AudioOutputPT8211        pt8211_1;       //xy=464,286
AudioConnection          patchCord1(usb1, 0, pt8211_1, 0);
AudioConnection          patchCord2(usb1, 0, peak1, 0);
AudioConnection          patchCord3(usb1, 0, rms1, 0);
AudioConnection          patchCord4(usb1, 1, pt8211_1, 1);
AudioConnection          patchCord5(usb1, 1, peak2, 0);
AudioConnection          patchCord6(usb1, 1, rms2, 0);
// GUItool: end automatically generated code

#define HOLD_MILLIS  4000


void audio_setup() {
  AudioMemory(12);
  level_l = level_r = 0.0;
  peak_l = peak_r = 0.0;
}


void audio_refresh_data() {
  static unsigned long last_hold = 0;
  
  if (peak1.available() && peak2.available()) {
    //level_l = rms1.read();
    level_l = peak1.read();
    //level_r = rms2.read();
    level_r = peak2.read();
  }

  peak_l = max(peak_l, level_l);
  peak_r = max(peak_r, level_r);
  if (peak_l == level_l || peak_r == level_r) {
    last_hold = millis();
  } else if ((millis() - last_hold) > HOLD_MILLIS) {
    peak_l = level_l;
    peak_r = level_r;
  }

}

