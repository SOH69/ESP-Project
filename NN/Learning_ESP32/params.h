// Activation functions
#define SIGMOID 0
#define RELU 1

// FFT parameters
#define SAMPLES 256
#define MAX_FREQ 20 // kHz
// Attenuation and threshold for display
#define COEF 40
#define ATT 500.0f

//
#define RATIO 0.8f         // ratio of training data vs. testing
#define EPOCHS 6000        // number of training epochs
#define NHID 40            // number of hidden layers
#define ACTIVATION RELU // chosen activation function of hidden layer
#define BATCH 50           // number of data used for training in each epoch
#define LR 1.0f            // initial learning rate
#define ANNEAL 0.9999f     // rate of change of learning rate
#define MAXERR 0.0005f     // stop training if error is less than this
#define DETECT 0.9f        // detection threshold
