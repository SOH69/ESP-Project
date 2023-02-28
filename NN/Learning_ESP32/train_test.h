/* Create the network and train it:
   - Read dataset
   - Create network
   - Train on training set
   - verify on testing set
   - Print performances
*/

Tinn createAndTrain() {
  // Read the dataset file
  int rows = init(datasetFile, 1); // number of samples

  // Neural network parameters
  const float ratio = RATIO;    // ratio of training data vs. testing
  const int epochs = EPOCHS;    // number of training epochs
  const int nips = bands + 1 ;  // neurons in the input layer
  const int nhid = NHID;        // neurons in the hidden layer
  const int nops = Noutput;     // neurons in the output layer
  float rate = LR;              // initial learning rate (lr)
  const float anneal = ANNEAL;  // rate of change of lr
  int nTrain = ratio * rows;    // size of training datset
  int nTest = rows - nTrain;    // size of testing dataset

  // Create the training set
  Data data = ndata(nips, nops, nTrain);
  for (int row = 0; row < nTrain; row++) {
    for (int col = 0; col < nips; col++)
      data.in[row][col] = input[row][col];
    data.tg[row][0] = output[row];
  }

  // Train, baby, train...
  const Tinn tinn = xtbuild(nips, nhid, nops);
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 5, "TRAINING");
  display.drawString(64, 30, "NETWORK");
  display.display();

  float error, err ;
  int Ndisp = epochs / 20;
  int nBatch = BATCH; //data.rows;
  Serial.printf("\nTraining... on batches of %d data\n", nBatch);
  unsigned long chrono = millis();
  for (int i = 0; i < epochs + 1; i++) {
    // take nBatch random data from the training dataset for training
    shuffle(data, nBatch, nTrain);
    error = 0.0f;
    for (int j = 0; j < nBatch; j++) {
      const float* const in = data.in[j];
      const float* const tg = data.tg[j];
      error += xttrain(tinn, in, tg, rate);
    }
    err = error / nBatch * 100.0;
    if (err < MAXERR && i > 2000) {
      Serial.printf("Epoch %4d\tError %7.4f\tLearning rate %.3f\n",
                    i, err, (double) rate);
      break;
    }
    if (i % Ndisp == 0)
      Serial.printf("Epoch %4d\tError %7.4f\tLearning rate %.3f\n",
                    i, err, (double) rate);
    rate *= anneal;
    int x = map(i, 0, epochs - 1, 0, 128);
    display.fillRect(0, 56, x, 10);
    display.display();
  }
  chrono = millis() - chrono;
  Serial.printf ("Training done in %u ms\n", chrono);
  // Save network
  Serial.println("Saving network on SPIFFS");
  xtsave(tinn, "/Network.txt");

  // Now test the network on test data (not used for training)
  Serial.println("\nTesting on unknown data...");
  display.clear();
  display.setFont(ArialMT_Plain_10);
  char text[25];
  display.drawString(64, 5, "Training done in");
  sprintf(text, "%d ms", chrono);
  display.drawString(64, 20, text);
  sprintf(text, "Error rate %.2f %%", error / data.rows * 100.0);
  display.drawString(64, 40, text);
  display.display();
  delay(1500);

  // Create the testing set
  Data test = ndata(nips, nops, nTest);
  for (int row = 0; row < nTest; row++) {
    for (int col = 0; col < nips; col++)
      test.in[row][col] = input[nTrain + row][col];
    test.tg[row][0] = output[nTrain + row];
  }
  // Test the network
  int Nerrs = 0;
  for (int sample = 0; sample < nTest; sample++) {
    const float* const in = test.in[sample];
    const float* const tg = test.tg[sample];
    const float* const pd = xtpredict(tinn, in);
    char text[4] = "OK ";
    if (abs(tg[0] - pd[0]) > 0.2) {
      strcpy(text, "NOK");
      ++Nerrs;
      Serial.printf ("%3d Expected : %d\tPrediction : %6.3f\t%s\n",
                     sample, (int)tg[0], (double)pd[0], text);
    }
  }
  Serial.printf ("%d errors over %d samples : error rate %.2f%%\n",
                 Nerrs, nTest, Nerrs * 100.0 / nTest);

  // Test over training set
  Nerrs = 0;
  for (int sample = 0; sample < nTrain; sample++) {
    const float* const in = data.in[sample];
    const float* const tg = data.tg[sample];
    const float* const pd = xtpredict(tinn, in);
    if (abs(tg[0] - pd[0]) > 0.2) ++Nerrs;
  }
  Serial.println("\nTesting over the training set :");
  Serial.printf ("%d errors on %d samples : error rate %.2f%%\n\n",
                 Nerrs, nTrain, Nerrs * 100.0 / nTrain);

  return tinn;
}

void splash () {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_24);
  display.drawString(64, 5, "Snore");
  display.drawString(64, 33, "Detector");
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_10);
  display.display();
  delay(1500);
}

void testNetwork (Tinn tinn) {
  // Read the dataset file
  int rows = init(datasetFile, 0); // number of samples
  // Create the dataset
  int nips = tinn.nips;
  int nhid = tinn.nhid;
  int nops = tinn.nops;
  Data data = ndata(nips, nops, rows);
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < nips; col++)
      data.in[row][col] = input[row][col];
    data.tg[row][0] = output[row];
  }
  // Test over entire dataset
  int Nerrs = 0;
  for (int sample = 0; sample < rows; sample++) {
    const float* const in = data.in[sample];
    const float* const tg = data.tg[sample];
    const float* const pd = xtpredict(tinn, in);
    if (abs(tg[0] - pd[0]) > 0.2) ++Nerrs;
  }
  Serial.println("Testing over the entire dataset :");
  Serial.printf ("%d errors on %d samples : error rate %.2f%%\n\n",
                 Nerrs, rows, Nerrs * 100.0 / rows);
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 5, "RESULTS:");
  char text[25];
  sprintf(text, "%d errors found", Nerrs);
  display.drawString(64, 20, text);
  sprintf(text, "on %d samples", rows);
  display.drawString(64, 35, text);
  sprintf(text, "Error rate: %.2f %%", Nerrs * 100.0 / rows);
  display.drawString(64, 50, text);
  display.display();
  delay(3000);
}
