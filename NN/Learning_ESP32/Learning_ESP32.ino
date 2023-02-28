/*
  Create the neural network
  Read the dataset (SPIFFS file /Data.txt)
  Train the network and print its performances
  Save the network (SPIFFS file /Network.txt)

  (c) Lesept - March 2020
*/
#include <Arduino.h>
#include <driver/adc.h>
#include <complex.h>
#include "params.h"
#define FREQ2IND (SAMPLES * 1.0 / MAX_FREQ)
#include <Wire.h>
#include "SSD1306.h"  // https://github.com/HelTecAutomation/Heltec_ESP32
SSD1306 display(0x3c, 21, 22); // 0.96" OLED display (address, SDA, SCL)
#include "FS.h"
#include "SPIFFS.h"
#include "Tinn.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define FORMAT_SPIFFS_IF_FAILED true
const char datasetFile[] = "/Data.txt";
const char networkFile[] = "/Network.txt";
#define BUTTON 19
#define LEDPIN 2

// Max dimensions 780 x 33 (for 32 freq bands) or 1460 x 17 (for 16 FB)
#define maxRows 1460
#define maxInput 17
#define Noutput 1
float input[maxRows][maxInput];
float output[maxRows];
float recorded[maxInput];
int bands = 0;
unsigned long sampling_period_us, chrono;
float complex sound[SAMPLES];
int LOG2SAMPLE = log(SAMPLES) / log(2);

#include "init.h"
#include "train_test.h"
#include "sound_functions.h"

// Declare the network
Tinn tinn;

void setup() {
  Serial.begin(115200);
  adc1_config_width(ADC_WIDTH_BIT_12);
  // ADC1 Channel 7 is GPIO 35 (microphone)
  adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
  sampling_period_us = round(1000ul * (1.0 / MAX_FREQ));
  pinMode (BUTTON, INPUT_PULLUP);  // button on 19
  pinMode (LEDPIN, OUTPUT);
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  display.init();
  splash();
  //  display.flipScreenVertically(); // Adjust to suit or remove

  bool loadNetwork = false;
  if (SPIFFS.exists(networkFile)) {
    Serial.println ("Found one network file : press button to load");
    display.clear();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(64, 5, "Found one network");
    display.drawString(64, 20, "on SPIFFS");
    display.drawString(64, 35, "Loading Network");
    display.display();
    loadNetwork = true;
    chrono = millis();
    while (millis() - chrono < 3000ul) { // Wait 3 secs for button
      if (!digitalRead(BUTTON)) {
        loadNetwork = true;
        delay(30);
        break;
      }
    }
  }

  if (!loadNetwork) { // Create the network and train it
    Serial.println("OK, let us make a new network");
    tinn = createAndTrain();
  } else {
    // Load the saved network
    Serial.println ("Loading network from file");
    display.drawString(64, 50, "--> LOADING NETWORK <--");
    display.display();
    delay(1500);
    tinn = xtload(networkFile);
  }

  // Test the network
  testNetwork (tinn);
  chrono = millis();
}

void loop() {
  digitalWrite(LEDPIN, LOW);
  acquisition();
  // Test each 1000 ms
  if (millis() - chrono > 1000ul) {
    chrono = millis();
    //    Data data = ndata(tinn.nips, tinn.nops, 1);
    //    for (int col = 0; col < tinn.nips; col++)
    //      data.in[0][col] = recorded[col];
    //    const float* const in = data.in[0];
    //    const float* const pd = xtpredict(tinn, in);
    const float* const pd = xtpredict(tinn, recorded);
    if (pd[0] > DETECT) {  // DETECTION !
      Serial.printf ("Score : %.2f DETECTION\n", pd[0]);
      digitalWrite(LEDPIN, HIGH);
      char text[15];
      sprintf(text, "%.0f%%", pd[0] * 100);
      display.setFont(ArialMT_Plain_16);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.setColor(BLACK);
      int x = 12;
      display.fillRect(x, 8, display.width() - 2 * x, 37);
      display.setColor(WHITE);
      display.drawRect(x, 8, display.width() - 2 * x, 37);
      display.drawString(64, 10, "DETECTION");
      display.drawString(64, 25, text);
      display.display();
      delay(400);
    }
  }
}
