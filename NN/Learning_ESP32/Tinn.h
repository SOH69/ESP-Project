/*
   functions from the TINN neural network library
*/

typedef struct
{
  // All the weights.
  float* w;
  // Hidden to output layer weights.
  float* x;
  // Biases.
  float* b;
  // Hidden layer.
  float* h;
  // Output layer.
  float* o;
  // Number of biases - always two - Tinn only supports a single hidden layer.
  int nb;
  // Number of weights.
  int nw;
  // Number of inputs.
  int nips;
  // Number of hidden neurons.
  int nhid;
  // Number of outputs.
  int nops;
}
Tinn;

// Computes error.
static float err(const float a, const float b)
{
  return 0.5f * (a - b) * (a - b);
}

// Returns partial derivative of error function.
static float pderr(const float a, const float b)
{
  return a - b;
}

// Computes total error of target to output.
static float toterr(const float* const tg, const float* const o, const int size)
{
  float sum = 0.0f;
  for (int i = 0; i < size; i++)
    sum += err(tg[i], o[i]);
  return sum;
}

// Activation function.
static float act(const float a, int actNumber)
{
  switch (actNumber) {
    case SIGMOID:
      return 1.0f / (1.0f + expf(-a));
      break;
    case RELU:
      return (a > 0) ? a : 0;
      break;
  }
}

// Returns partial derivative of activation function.
static float pdact(const float a, int actNumber)
{
  switch (actNumber) {
    case SIGMOID:
      return a * (1.0f - a);
      break;
    case RELU:
      return (a > 0) ? 1 : 0;
  }
}

// Returns floating point random from 0.0 - 1.0.
static float frand()
{
  return  (esp_random() / (float)UINT32_MAX);
}

// Performs back propagation.
static void bprop(const Tinn t, const float* const in, const float* const tg, float rate)
{
  for (int i = 0; i < t.nhid; i++)
  {
    float sum = 0.0f;
    // Calculate total error change with respect to output.
    for (int j = 0; j < t.nops; j++)
    {
      const float a = pderr(t.o[j], tg[j]);
      const float b = pdact(t.o[j], SIGMOID); // SIGMOID at output layer
      sum += a * b * t.x[j * t.nhid + i];
      // Correct weights in hidden to output layer.
      t.x[j * t.nhid + i] -= rate * a * b * t.h[i];
    }
    // Correct weights in input to hidden layer.
    for (int j = 0; j < t.nips; j++)
      t.w[i * t.nips + j] -= rate * sum * pdact(t.h[i], ACTIVATION) * in[j];
  }
}

// Performs forward propagation.
static void fprop(const Tinn t, const float* const in)
{
  // Calculate hidden layer neuron values.
  for (int i = 0; i < t.nhid; i++)
  {
    float sum = 0.0f;
    for (int j = 0; j < t.nips; j++)
      sum += in[j] * t.w[i * t.nips + j];
    t.h[i] = act(sum + t.b[0], ACTIVATION);
  }
  // Calculate output layer neuron values.
  for (int i = 0; i < t.nops; i++)
  {
    float sum = 0.0f;
    for (int j = 0; j < t.nhid; j++)
      sum += t.h[j] * t.x[i * t.nhid + j];
    t.o[i] = act(sum + t.b[1], SIGMOID); // SIGMOID at output layer
  }
}

// Randomizes tinn weights and biases.
static void wbrand(const Tinn t)
{
  for (int i = 0; i < t.nw; i++) t.w[i] = frand() - 0.5f;
  for (int i = 0; i < t.nb; i++) t.b[i] = frand() - 0.5f;
}

// Returns an output prediction given an input.
float* xtpredict(const Tinn t, const float* const in)
{
  fprop(t, in);
  return t.o;
}

// Trains a tinn with an input and target output with a learning rate. Returns target to output error.
float xttrain(const Tinn t, const float* const in, const float* const tg, float rate)
{
  fprop(t, in);
  bprop(t, in, tg, rate);
  return toterr(tg, t.o, t.nops);
}

// Constructs a tinn with number of inputs, number of hidden neurons, and number of outputs
Tinn xtbuild(const int nips, const int nhid, const int nops)
{
  Tinn t;
  // Tinn only supports one hidden layer so there are two biases.
  t.nb = 2;
  t.nw = nhid * (nips + nops);
  t.w = (float*) calloc(t.nw, sizeof(*t.w));
  t.x = t.w + nhid * nips;
  t.b = (float*) calloc(t.nb, sizeof(*t.b));
  t.h = (float*) calloc(nhid, sizeof(*t.h));
  t.o = (float*) calloc(nops, sizeof(*t.o));
  t.nips = nips;
  t.nhid = nhid;
  t.nops = nops;
  wbrand(t);
  return t;
}

// Saves a tinn to disk.
void xtsave(const Tinn t, const char* const path)
{
  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.printf("%s - failed to open file for writing\n", path);
    return;
  }
  // Save header.
  file.printf("%d %d %d\n", t.nips, t.nhid, t.nops);
  // Save biases and weights.
  for (int i = 0; i < t.nb; i++) file.printf("%f\n", (double) t.b[i]);
  for (int i = 0; i < t.nw; i++) file.printf("%f\n", (double) t.w[i]);
  file.close();
}

// Reads a positive integer in a file
int readIntFile (File file) {
  int val = 0;
  while (file.available()) {
    char c = file.read();
    switch (c) {
      case 10: // CR
        {
          return val;
          break;
        }
      case 13: // LF
        break;
      case 32: // space
      case 44: // ,  (to read CSV files)
      case 46: // .
      case 59: // ;
        return val;
        break;
      default: // numbers
        val = val * 10 + c - '0';
        break;
    }
  }
  return val;
}

// Reads a float from a file
float readFloatFile (File file) {
  float val = 0;
  int integ = 0;
  int frac = 0;
  int sign = 1;
  while (file.available()) {
    char c = file.read();
    //  Serial.print(c);
    switch (c) {
      case 10: // CR
        {
          return sign * integ;
          break;
        }
      case 13: // LF
        break;
      case 45: // - (minus sign)
        sign = -1;
        break;
      case 46: // .
        {
          frac = readIntFile (file);
          int n = log10(frac) + 1;
          val = sign * (integ + frac / pow(10, n));
          return val;
          break;
        }
      case 32: // space
      case 44: // ,  (to read CSV files)
      case 59: // ;
        return sign * integ;
        break;
      default: // numbers
        integ = integ * 10 + c - '0';
        break;
    }
  }
  return sign * val;
}

// Loads a tinn from disk.
Tinn xtload(const char* const path)
{
  File file = SPIFFS.open(path);
  if (!file || file.isDirectory()) {
    Serial.printf("%s - failed to open file for reading\n", path);
    while (1);
  }
  // Load header.
  int nips = readIntFile (file);
  int nhid = readIntFile (file);
  int nops = readIntFile (file);
  // Build a new tinn.
  const Tinn t = xtbuild(nips, nhid, nops);
  // Load biases and weights.
  for (int i = 0; i < t.nb; i++) t.b[i] = readFloatFile (file);
  for (int i = 0; i < t.nw; i++) t.w[i] = readFloatFile (file);
  file.close();
  return t;
}

// Frees object from heap.
void xtfree(const Tinn t)
{
  free(t.w);
  free(t.b);
  free(t.h);
  free(t.o);
}

// Prints an array of floats. Useful for printing predictions.
void xtprint(const float * arr, const int size)
{
  for (int i = 0; i < size; i++)
    Serial.printf("%f ", (double) arr[i]);
  Serial.printf("\n");
}
