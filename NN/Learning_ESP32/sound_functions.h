/* Windowing type */
#define RECTANGLE 0x00 /* rectangle (Box car) */
#define HAMMING 0x01 /* hamming */
#define HANN 0x02 /* hann */
#define TRIANGLE 0x03 /* triangle (Bartlett) */
#define NUTTALL 0x04 /* nuttall */
#define BLACKMAN 0x05 /* blackman */
#define BLACKMAN_NUTTALL 0x06 /* blackman nuttall */
#define BLACKMAN_HARRIS 0x07 /* blackman harris*/
#define FLT_TOP 0x08 /* flat top */
#define WELCH 0x09 /* welch */

typedef enum fft_dir {
  FFT_FORWARD,    /* kernel uses "-1" sign */
  FFT_INVERSE     /* kernel uses "+1" sign */
} fft_dir;

void window (float complex *data, unsigned int log2_N, byte windowType, fft_dir direction) {
  unsigned int N = 1 << log2_N;
  unsigned int Nd2 = N >> 1;
  for (int i = 0; i < Nd2; i++) {
    double indexMinusOne = double(i);
    double ratio = (indexMinusOne / (N - 1));
    double weighingFactor = 1.0;
    // Compute and record weighting factor
    switch (windowType) {
      case RECTANGLE: // rectangle (box car)
        weighingFactor = 1.0;
        break;
      case HAMMING: // hamming
        weighingFactor = 0.54 - (0.46 * cos(2 * PI * ratio));
        break;
      case HANN: // hann
        weighingFactor = 0.54 * (1.0 - cos(2 * PI * ratio));
        break;
      case TRIANGLE: // triangle (Bartlett)
        weighingFactor = 1.0 - ((2.0 * abs(indexMinusOne - ((N - 1) / 2.0))) / (N - 1));
        break;
      case NUTTALL: // nuttall
        weighingFactor = 0.355768 - (0.487396 * (cos(2 * PI * ratio))) + (0.144232 * (cos(4 * PI * ratio))) - (0.012604 * (cos(6 * PI * ratio)));
        break;
      case BLACKMAN: // blackman
        weighingFactor = 0.42323 - (0.49755 * (cos(2 * PI * ratio))) + (0.07922 * (cos(4 * PI * ratio)));
        break;
      case BLACKMAN_NUTTALL: // blackman nuttall
        weighingFactor = 0.3635819 - (0.4891775 * (cos(2 * PI * ratio))) + (0.1365995 * (cos(4 * PI * ratio))) - (0.0106411 * (cos(6 * PI * ratio)));
        break;
      case BLACKMAN_HARRIS: // blackman harris
        weighingFactor = 0.35875 - (0.48829 * (cos(2 * PI * ratio))) + (0.14128 * (cos(4 * PI * ratio))) - (0.01168 * (cos(6 * PI * ratio)));
        break;
      case FLT_TOP: // flat top
        weighingFactor = 0.2810639 - (0.5208972 * cos(2 * PI * ratio)) + (0.1980399 * cos(4 * PI * ratio));
        break;
      case WELCH: // welch
        weighingFactor = 1.0 - sq((indexMinusOne - (N - 1) / 2.0) / ((N - 1) / 2.0));
        break;
    }
    if (direction == FFT_FORWARD) {
      data[i] *= weighingFactor;
      data[N - (i + 1)] *= weighingFactor;
    }
    else {
      data[i] /= weighingFactor;
      data[N - (i + 1)] /= weighingFactor;
    }
  }
}

void ffti_shuffle_f(float complex *data, unsigned int log2_N)
{
  /*
     Basic Bit-Reversal Scheme:

     The incrementing pattern operations used here correspond
     to the logic operations of a synchronous counter.

     Incrementing a binary number simply flips a sequence of
     least-significant bits, for example from 0111 to 1000.
     So in order to compute the next bit-reversed index, we
     have to flip a sequence of most-significant bits.
  */

  unsigned int N = 1 << log2_N;   /* N */
  unsigned int Nd2 = N >> 1;      /* N/2 = number range midpoint */
  unsigned int Nm1 = N - 1;       /* N-1 = digit mask */
  unsigned int i;                 /* index for array elements */
  unsigned int j;                 /* index for next element swap location */

  for (i = 0, j = 0; i < N; i++) {
    if (j > i) {
      float complex tmp = data[i];
      data[i] = data[j];
      data[j] = tmp;
    }

    /*
       Find least significant zero bit
    */

    unsigned int lszb = ~i & (i + 1);

    /*
       Use division to bit-reverse the single bit so that we now have
       the most significant zero bit

       N = 2^r = 2^(m+1)
       Nd2 = N/2 = 2^m
       if lszb = 2^k, where k is within the range of 0...m, then
           mszb = Nd2 / lszb
                = 2^m / 2^k
                = 2^(m-k)
                = bit-reversed value of lszb
    */

    unsigned int mszb = Nd2 / lszb;

    /*
       Toggle bits with bit-reverse mask
    */

    unsigned int bits = Nm1 & ~(mszb - 1);
    j ^= bits;
  }
}



void ffti_evaluate_f(float complex *data, unsigned int log2_N, fft_dir direction)
{
  /*
     In-place FFT butterfly algorithm

     input:
         A[] = array of N shuffled complex values where N is a power of 2
     output:
         A[] = the DFT of input A[]

     for r = 1 to log2(N)
         m = 2^r
         Wm = exp(−j2π/m)
         for n = 0 to N-1 by m
             Wmk = 1
             for k = 0 to m/2 - 1
                 u = A[n + k]
                 t = Wmk * A[n + k + m/2]
                 A[n + k]       = u + t
                 A[n + k + m/2] = u - t
                 Wmk = Wmk * Wm

     For inverse FFT, use Wm = exp(+j2π/m)
  */

  unsigned int N = 1 << log2_N;
  double theta_2pi = (direction == FFT_FORWARD) ? -PI : PI;
  theta_2pi *= 2;

  for (unsigned int r = 1; r <= log2_N; r++)
  {
    unsigned int m = 1 << r;
    unsigned int md2 = m >> 1;
    double theta = theta_2pi / m;
    double re = cos(theta);
    double im = sin(theta);
    double complex Wm = re + I * im;
    for (unsigned int n = 0; n < N; n += m) {
      double complex Wmk = 1.0f;   /* Use double for precision */
      for (unsigned int k = 0; k < md2; k++) {
        unsigned int i_e = n + k;
        unsigned int i_o = i_e + md2;
        double complex t = Wmk * data[i_o];
        data[i_o] = data[i_e] - t;
        data[i_e] = data[i_e] + t;
        t = Wmk * Wm;
        Wmk = t;
      }
    }
  }
}

void ffti_f(float complex data[], unsigned int log2_N, fft_dir direction)
{
  ffti_shuffle_f(data, log2_N);
  ffti_evaluate_f(data, log2_N, direction);
}

int mean(int i1, int i2) {
  unsigned long sum = 0;
  for (int i = i1; i < i2; i++) sum += abs(creal(sound[i]));
  sum /= (i2 - i1);
  return (int)sum;
}

int acquireAmplitude () {
  unsigned int signalMax = 0;
  unsigned int signalMin = 4096;
  unsigned long chrono = micros(); // Sample window 1 ms
  while (micros() - chrono < 1000ul) {
    int sample = analogRead(A0);
    if (sample > signalMax) signalMax = sample;
    else if (sample < signalMin) signalMin = sample;
  }
  unsigned int peakToPeak = signalMax - signalMin;
  int amplitude = peakToPeak - 70;
  constrain(amplitude, 0, 500);
  return amplitude;
}

void acquireSound () {
  for (int i = 0; i < SAMPLES; i++) {
    unsigned long chrono = micros();
    sound[i] = analogRead(A0);
    while (micros() - chrono < sampling_period_us); // do nothing
  }
  window (sound, LOG2SAMPLE, HAMMING, FFT_FORWARD);
  ffti_f(sound, LOG2SAMPLE, FFT_FORWARD);
}

void displaySpectrum () {
  display.setFont(ArialMT_Plain_10);
  int nFreq = display.width();
  int hauteur = display.height();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  float freqs[10] = {.5, 1, 2, 3, 4, 6, 8, 10, 12, 15};
  float FREQmax = MAX_FREQ * 128.0 / SAMPLES;
  for (int i = 0; i < 10; i++) {
    int pos =  FREQ2IND * freqs[i] ;
    if (pos > display.width() + 1) break;
    if (i == 0)display.drawString(pos, 0, ".5");
    else display.drawString(pos, 0, String(freqs[i], 0));
  }
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  // Affichage du spectre
  int ampmax = 0;
  int imax = 0;
  for (int i = 2; i < nFreq; i++) { // SAMPLES / 2
    int amplitude = (int)creal(sound[i]) / COEF;
    if (amplitude > ampmax) {
      ampmax = amplitude;
      imax = i;
    }
    amplitude = min(amplitude, hauteur);
    display.drawVerticalLine (i, hauteur - amplitude, amplitude);
  }

  // Affichage fréquence de plus forte amplitude
  int freqmax = 985.0 * imax * MAX_FREQ / SAMPLES;
  char text[14];
  sprintf(text, "%4d Hz: %d", freqmax, ampmax);
  // Serial.println(text);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 20, text);
  display.display();
}

void acquisition () {
  // Acquire 1 sample
  acquireSound();
  int amp = acquireAmplitude ();
  displaySpectrum();
  int valMax = 0;
  int nFreqs = SAMPLES / bands / 2;
  for (int i = 0; i < bands; i++) {
    int ind1 = i * nFreqs;
    int ind2 = ind1 + nFreqs;
    if (ind1 == 0) ind1 = 3;
    recorded[i] = mean(ind1, ind2);
    if (recorded[i] > valMax) valMax = recorded[i];
  }
  recorded[bands] = amp;
//  if (amp > valMax) valMax = amp;
//  for (int i = 0; i < bands; i++) recorded[i] /= valMax;
  for (int i = 0; i <= bands; i++) recorded[i] /= ATT;
}
