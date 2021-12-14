# Main Board

* The line controlling the Output Enable on the shift registers was
  omitted; this was corrected via a bodge wire. Later revisions may
  wish to tie this to the Teensy.

# Display Board

* The peak LEDs are swapped (numbering pattern is misleading); this is
  corrected in software. 

* LED resistor values are updated based on tests; the peak LEDs are
  150 ohms and the bargraphs are 83 ohm resistor packs. Brightness is
  acceptable but I am slightly nervous about total current consumption
  (can see the bargraphs flicker in brightness at times).