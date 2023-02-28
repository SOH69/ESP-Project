#define SWAP(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)

// Data object.
typedef struct
{
  // 2D floating point array of input.
  float** in;
  // 2D floating point array of target.
  float** tg;
  // Number of inputs to neural network.
  int nips;
  // Number of outputs to neural network.
  int nops;
  // Number of rows in file (number of sets for neural network).
  int rows;
}
Data;

// New 2D array of floats.
static float** new2d(const int rows, const int cols)
{
  float** row = (float**) malloc((rows) * sizeof(float*));
  for (int r = 0; r < rows; r++)
    row[r] = (float*) malloc((cols) * sizeof(float));
  return row;
}

// New data object.
static Data ndata(const int nips, const int nops, const int rows)
{
  const Data data = {
    new2d(rows, nips), new2d(rows, nops), nips, nops, rows
  };
  return data;
}

// Randomly shuffles a data object.
static void shuffle(const Data d, int nBatch, int nTrain)
{
  //  for (int a = 0; a < d.rows; a++)
  for (int a = 0; a < nBatch; a++)
  {
    //    const int b = esp_random() % d.rows;
    const int b = esp_random() % nTrain;
    float* ot = d.tg[a];
    float* it = d.in[a];
    // Swap output.
    d.tg[a] = d.tg[b];
    d.tg[b] = ot;
    // Swap input.
    d.in[a] = d.in[b];
    d.in[b] = it;
  }
}

// Display "file read" and shuffle dataset
void finish (int iRow, int bands, int disp) {
  if (disp == 1) {
    Serial.println ("Done");
    Serial.printf("Read %d samples of %d frequency bands\n", iRow, bands);
    display.clear();
    display.setFont(ArialMT_Plain_10);
    char text[25];
    sprintf(text, "Read %d samples", iRow);
    display.drawString(64, 15, text);
    sprintf(text, "of %d frequency bands", bands);
    display.drawString(64, 35, text);
    display.display();
    delay(1500);
  }
  // Shuffling dataset
  for (int i = 0; i < 3 * iRow; i++) {
    int ind1 = random(iRow);
    int ind2 = random(iRow);
    for (int j = 0; j < bands + 1; j++) SWAP(input[ind1][j], input[ind2][j]);
    SWAP(output[ind1], output[ind2]);
  }
}

int init (const char * path, byte disp) {
  if (disp == 1) {
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 10, "LEARNING");
    display.drawString(64, 35, "PHASE");
    display.display();
    delay(1500);
  }
  if (!SPIFFS.exists(path)) {
    Serial.println("Dataset not found. Please create one before learning.");
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 5, "Dataset not found");
    display.drawString(64, 25, "Please create one");
    display.drawString(64, 45, "before learning!");
    display.display();
    while (1);
  }
  // Read dataset file
  File file = SPIFFS.open(path, FILE_READ);
  // Read number of frequency bands
  bands = 0;
  char c = file.read();
  while (c != 13) { // CR
    bands = bands * 10 + c - '0';
    c = file.read();
  }
  if (bands > maxInput) {
    Serial.println("Abort: too many input neurons");
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 15, "TOO MANY");
    display.drawString(64, 35, "INPUT NEURONS");
    display.display();
    while (1);
  }
  if (disp == 1) Serial.print("Reading dataset: ");
  c = file.read(); // read LF
  // Now read the values
  int iRow = 0;
  int iCol = 0;
  int val = 0;
  int valMax = 0;
  while (file.available()) {
    char c = file.read();
    //  Serial.print(c);
    switch (c) {
      case 10: // CR
        {
          //  Serial.print(".");
          output[iRow] = val;
          for (int i = 0; i <= bands + 1; i++)
            input[iRow][i] = input[iRow][i] / ATT;
          //            input[iRow][i] = input[iRow][i] / valMax;
          ++iRow;
          if (iRow == maxRows) {
            finish (iRow, bands, disp);
            return maxRows;
          }
          iCol = 0;
          val = 0;
          valMax = 0;
          break;
        }
      case 13: // LF
        break;
      case 32: // space
      case 44: // ,  (to read CSV files)
      case 46: // .
      case 59: // ;
        input[iRow][iCol] = val;
        if (val > valMax) valMax = val;
        ++iCol;
        val = 0;
        break;
      default: // numbers
        val = val * 10 + c - '0';
        break;
    }
  }
  finish (iRow, bands, disp);
  return iRow;
}
