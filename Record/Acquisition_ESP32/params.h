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

// FFT parameters
#define SAMPLES 256
#define MAX_FREQ 20 // kHz
// Attenuation and threshold for display
#define COEF 20
// Frequency bands
#define BANDS 32
