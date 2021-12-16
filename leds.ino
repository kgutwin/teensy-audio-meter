IntervalTimer ledsTimer;
/*
 * The LED column display consists of a pair of 8-bit shift registers configured such
 * that the first 11 bits control the X coordinate and the next 4 bits control
 * the Y coordinate of a 2D grid.
 * 
 * Data is clocked into the shift registers serially, least significant bit first.
 * The first bit is always zero since it is not connected to anything; followed next
 * by the four bits of the Y coordinate, and finally the 11 bits of the X coordinate.
 * Once all 16 bits are in, the RCK signal is asserted, which releases the data to
 * the output latches and the LEDs can light.
 * 
 * Each of the four Y coordinate values is displayed one at a time; the display
 * cycle time is fast enough to minimize flicker (currently set at 4ms per part).
 * Refresh is handled by the IntervalTimer library which is interrupt-driven, so
 * display refresh is not affected by timing jitter in the main loop.
 */

// Global variable storing the currently displayed LED columns.
uint16_t leds_columns[4];

// TODO: check on those delayMicroseconds(), do we need them? What speed are we seeing?

#define LEDS_WRITE_BIT(v)  \
  digitalWrite(SR_SER, (v));

#define LEDS_CLOCK()       \
  delayMicroseconds(1);    \
  digitalWrite(SR_CLK, 1); \
  delayMicroseconds(2);    \
  digitalWrite(SR_CLK, 0); \
  delayMicroseconds(1);

#define LEDS_SHOW()        \
  delayMicroseconds(1);    \
  digitalWrite(SR_RCK, 1); \
  delayMicroseconds(1);    \
  digitalWrite(SR_RCK, 0); \
  delayMicroseconds(1);

void leds_init() {
  // clock in 17 zero bits to clear the screen
  LEDS_WRITE_BIT(0);
  for (int i=0; i<17; i++) {
    LEDS_CLOCK();
  }
  LEDS_SHOW();
  
  ledsTimer.begin(leds_refresh, 4000);
}

/*
 * Refresh the LEDs by displaying a new column. This function is called by the
 * IntervalTimer set up in leds_init() and displays one new column each time it is called.
 */
void leds_refresh() {
  static uint8_t channel = 0;
  uint16_t column = leds_columns[channel];

  // first bit ignored
  LEDS_WRITE_BIT(0);
  LEDS_CLOCK();
  // channel select
  for (int i=3; i>=0; i--) {
    LEDS_WRITE_BIT(i == channel);
    LEDS_CLOCK();
  }
  // column
  for (int i=0; i<11; i++) {
    LEDS_WRITE_BIT(column & 1);
    LEDS_CLOCK();
    column >>= 1;
  }
  LEDS_SHOW();
  
  channel = (channel + 1) % 4;
}

// Mapping of dB thresholds to LEDs. This is taken from the LED bargraphs on the X32 console.
const float leds_vals[] = {
   0.0,  
  -1.0,  -2.0,  -3.0,  -4.0,  -6.0,  -8.0, -10.0, -12.0, -15.0, -18.0,
 -21.0, -24.0, -27.0, -30.0, -33.0, -36.0, -39.0, -42.0, -45.0, -48.0 
};

void leds_set_value(float left_rms_db, float right_rms_db,
                    float left_peak_db, float right_peak_db) {                      

  // left
  uint32_t left_column = 0;
  bool left_peak_found = 0;
  for (int i=0; i<21; i++) {
    if (!left_peak_found && -left_peak_db <= -leds_vals[i]) {
      left_column |= 1 << (20 - i);
      left_peak_found = 1;
    }
    if (-left_rms_db <= -leds_vals[i]) {
      left_column |= 1 << (20 - i);
    }
  }

  // right
  uint32_t right_column = 0;
  bool right_peak_found = 0;
  for (int i=0; i<21; i++) {
    if (!right_peak_found && -right_peak_db <= -leds_vals[i]) {
      right_column |= 1 << (20 - i);
      right_peak_found = 1;
    }
    if (-right_rms_db <= -leds_vals[i]) {
      right_column |= 1 << (20 - i);
    }
  }

  // assign columns
  leds_columns[0] = (left_column >> 10) & 0x3ff;
  leds_columns[1] = left_column & 0x3ff;
  leds_columns[2] = (right_column >> 10) & 0x3ff;
  leds_columns[3] = right_column & 0x3ff;

  // swap silly peak LEDs
  leds_columns[0] |= (right_column & (1 << 20)) >> 10;
  leds_columns[2] |= (left_column & (1 << 20)) >> 10;
}

