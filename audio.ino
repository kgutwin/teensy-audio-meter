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
AudioMixer4              mixer1;
AudioAnalyzeFFT1024      fft1024;
AudioAnalyzeStereo       stereo1;

AudioConnection          patchCord1(usb1, 0, pt8211_1, 0);
AudioConnection          patchCord2(usb1, 0, peak1, 0);
AudioConnection          patchCord3(usb1, 0, rms1, 0);
AudioConnection          patchCord4(usb1, 1, pt8211_1, 1);
AudioConnection          patchCord5(usb1, 1, peak2, 0);
AudioConnection          patchCord6(usb1, 1, rms2, 0);
AudioConnection          patchCord7(usb1, 0, mixer1, 0);
AudioConnection          patchCord8(usb1, 1, mixer1, 1);
AudioConnection          patchCord9(mixer1, fft1024);
AudioConnection          patchCord10(usb1, 0, stereo1, 0);
AudioConnection          patchCord11(usb1, 1, stereo1, 1);
// GUItool: end automatically generated code

#define HOLD_MILLIS  4000


void audio_setup() {
  AudioMemory(24);
  mixer1.gain(0, 0.5);
  mixer1.gain(1, 0.5);
  
  level_l = level_r = 0.0;
  peak_l = peak_r = 0.0;
  stereo_correlation = 0.0;
}


void audio_refresh_data() {
  /*
   * Peak levels (dB meter)
   */
  static unsigned long last_hold = 0;
  
  if (peak1.available() && peak2.available()) {
    //level_l = rms1.read();
    level_l = peak1.read();
    //level_r = rms2.read();
    level_r = peak2.read();
    audio_connected = 1;
  } else {
    audio_connected = 0;
  }

  peak_l = max(peak_l, level_l);
  peak_r = max(peak_r, level_r);
  if (peak_l == level_l || peak_r == level_r) {
    last_hold = millis();
  } else if ((millis() - last_hold) > HOLD_MILLIS) {
    peak_l = level_l;
    peak_r = level_r;
  }

  /*
   * FFT (spectrum)
   */
  if (fft1024.available()) {
    // read the 512 FFT frequencies into 16 levels
    // music is heard in octaves, but the FFT data
    // is linear, so for the higher octaves, read
    // many FFT bins together.
    fft_level[0] =  fft1024.read(0);
    fft_level[1] =  fft1024.read(1);
    fft_level[2] =  fft1024.read(2, 3);
    fft_level[3] =  fft1024.read(4, 6);
    fft_level[4] =  fft1024.read(7, 10);
    fft_level[5] =  fft1024.read(11, 15);
    fft_level[6] =  fft1024.read(16, 22);
    fft_level[7] =  fft1024.read(23, 32);
    fft_level[8] =  fft1024.read(33, 46);
    fft_level[9] =  fft1024.read(47, 66);
    fft_level[10] = fft1024.read(67, 93);
    fft_level[11] = fft1024.read(94, 131);
    fft_level[12] = fft1024.read(132, 184);
    fft_level[13] = fft1024.read(185, 257);
    fft_level[14] = fft1024.read(258, 359);
    fft_level[15] = fft1024.read(360, 511);    
  }

  /*
   * Stereo (correlation, scatter)
   */
  if (stereo1.available()) {
    stereo_correlation = stereo1.readCorrelation();
    stereo1.readScatter(stereo_px_l, stereo_px_r, AUDIO_BLOCK_SAMPLES);
  }
}

