#include "AudioStream.h"
#include <ebur128.h>

class AudioAnalyzeLUFS : public AudioStream
{
public:
  AudioAnalyzeLUFS() : AudioStream(2, inputQueueArray) {
    sts = ebur128_init(2, AUDIO_SAMPLE_RATE, EBUR128_MODE_S);
    has_samples = false;
  }
  void update(void) {
    audio_block_t *block_l, *block_r;
    const int16_t *p_l, *p_r, *end;
    int16_t interleaved[AUDIO_BLOCK_SAMPLES * 2];
    int16_t *intp = interleaved;

    block_l = receiveReadOnly(0);
    block_r = receiveReadOnly(1);
    if (!block_l || !block_r) return;
    
    p_l = block_l->data;
    p_r = block_r->data;
    end = p_l + AUDIO_BLOCK_SAMPLES;
    do {
      *(intp++) = *(p_l++);
      *(intp++) = *(p_r++);
    } while (p_l < end);
    ebur128_add_frames_short(sts, interleaved, AUDIO_BLOCK_SAMPLES);
    has_samples = true;
    release(block_l);
    release(block_r);
  }
  float readLoudnessMomentary(void) {
    double out;
    ebur128_loudness_momentary(sts, &out);
    has_samples = false;
    return out;
  }
  bool available(void) {
    return has_samples;
  }

private:
  audio_block_t *inputQueueArray[2];
  volatile bool has_samples;
  ebur128_state *sts;
};
