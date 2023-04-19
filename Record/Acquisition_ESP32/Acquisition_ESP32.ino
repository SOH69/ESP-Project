/*
  Program for sound acquisition and saving in SPIFFS
  to create a dataset for deep learning code

  Alternately make snoring sound and silence
  Follow instructions on OLED display

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
#define FORMAT_SPIFFS_IF_FAILED true

unsigned long chrono, chrono2;
unsigned long sampling_period_us;
float complex data[SAMPLES];
int LOG2SAMPLE = log(SAMPLES) / log(2);
const char filename[] = "/Data.txt";
#define BUTTON 19
#define LEDPIN 2
#include "functions.h"

void setup() {
  Serial.begin(115200);
  adc1_config_width(ADC_WIDTH_BIT_12);
  // ADC1 Channel 7 is GPIO 35 (microphone)
  adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
  sampling_period_us = round(1000ul * (1.0 / MAX_FREQ));
  Serial.println("\nSOUND ACQUISITION\n");
  pinMode (BUTTON, INPUT_PULLUP);  // button on 19
  pinMode (LEDPIN, OUTPUT);
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  display.init();
  //  display.flipScreenVertically(); // Adjust to suit or remove
  init(filename);
}

void loop() {
  static unsigned int N = 0;
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 5, "PUSH BUTTON TO");
  display.drawString(64, 25, "START ACQUISITION");
  if (N != 0) display.drawString(64, 45, "... AGAIN ...");
  char text[7];
  sprintf(text,"(%d)",N);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.drawString(128, 52, text);
  display.display();

  // Push the button to acquire samples
  while (digitalRead(BUTTON)) yield();
    delay(30); // debounce
    // Acquire 10 snore samples
    acquisition(1);
    delay(3000);
    // Acquire 10 silence samples
    acquisition(0);
  ++N;
}
