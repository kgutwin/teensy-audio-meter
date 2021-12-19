#include "AudioStream.h"
#include "math.h"

class AudioAnalyzeStereo : public AudioStream
{
public:
  AudioAnalyzeStereo() : AudioStream(2, inputQueueArray) {
    lr_product = l_square = r_square = 0.0;
    n_samples = 0;
    for (int i=0; i<AUDIO_BLOCK_SAMPLES; i++) {
      l_pixels[i] = r_pixels[i] = 0;
    }
  }
  void update(void) {
    audio_block_t *block_l, *block_r;
    const int16_t *p_l, *p_r, *end;
    int8_t *l_px, *r_px;

    block_l = receiveReadOnly(0);
    block_r = receiveReadOnly(1);
    if (!block_l || !block_r) return;
    
    p_l = block_l->data;
    p_r = block_r->data;
    end = p_l + AUDIO_BLOCK_SAMPLES;
    float lr_product_local = 0.0, l_square_local = 0.0, r_square_local = 0.0;
    l_px = l_pixels;
    r_px = r_pixels;
    do {
      int16_t left = *p_l++;
      int16_t right = *p_r++;
      
      lr_product_local += left * right;
      l_square_local += left * left;
      r_square_local += right * right;

      *(l_px++) = left >> 10;
      *(r_px++) = right >> 10;
    } while (p_l < end);
    n_samples += AUDIO_BLOCK_SAMPLES;
    lr_product += lr_product_local;
    l_square += l_square_local;
    r_square += r_square_local;
    release(block_l);
    release(block_r);
  }

  bool available(void) {
    return n_samples != 0;
  }
  
  float readCorrelation(void) {
    // http://rs-met.com/documents/tutorials/StereoProcessing.pdf
    // correlation equals the division between:
    // * the average of (the product of the left and right channels)
    // * the square root of the product of:
    //   * the average of (the square of the left channel)
    //   * the average of (the square of the right channel)    
    if (n_samples == 0) return 0.0;
    __disable_irq();
    float lrp = lr_product;
    float lsq = l_square;
    float rsq = r_square;
    int nsp = n_samples;
    lr_product = l_square = r_square = 0.0;
    n_samples = 0;
    __enable_irq();

    if (lsq == 0 || rsq == 0) return 0.0;

    return ((lrp / nsp) / sqrt((lsq / nsp) * (rsq / nsp)));
  }

  void readScatter(int8_t *lpx, int8_t *rpx, int max_px) {
    max_px = min(max_px, AUDIO_BLOCK_SAMPLES);
    for (int i=0; i<max_px; i++) {
      lpx[i] = l_pixels[i];
      rpx[i] = r_pixels[i];
    }
  }
  
private:
  audio_block_t *inputQueueArray[2];
  float lr_product;
  float l_square;
  float r_square;
  int n_samples;
  int8_t l_pixels[AUDIO_BLOCK_SAMPLES];
  int8_t r_pixels[AUDIO_BLOCK_SAMPLES];
};
