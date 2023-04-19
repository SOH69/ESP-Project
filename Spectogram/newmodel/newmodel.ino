#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/experimental/micro/kernels/micro_ops.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/experimental/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "my_model_data.h" // Include the TensorFlow Lite model converted to a C array

#define OLED_RESET -1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int microphonePin = 35; // Analog input pin for the microphone
const int threshold = 80; // Threshold for snore detection

// Define the input and output tensors

namespace {
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  tflite::ErrorReporter* error_reporter = nullptr;
  TfLiteTensor* inputTensor = nullptr;
  TfLiteTensor* outputTensor = nullptr;
  const uint32_t kTensorArenaSize = 16 * 1024;
  static uint8_t tensor_arena[kTensorArenaSize];
}

// Define the interpreter


void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(microphonePin, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Snore Detection");
  display.display();
  delay(2000);

  // Initialize the model
  model = tflite::GetModel(my_model_tflite);
  if (model == nullptr) {
    Serial.println("Failed to load model");
    while (1);
  }

  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;


  static tflite::MicroMutableOpResolver micro_mutable_op_resolver;
  //micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_DEPTHWISE_CONV_2D, tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED, tflite::ops::micro::Register_FULLY_CONNECTED());
  micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX, tflite::ops::micro::Register_SOFTMAX());


  static tflite::MicroInterpreter static_interpreter(model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;
  TfLiteStatus status = interpreter->AllocateTensors();
  if (status != kTfLiteOk) {
    Serial.println("Failed to allocate tensors");
    while (1);
  }

  // Get the input and output tensors
  inputTensor = interpreter->input(0);
  outputTensor = interpreter->output(0);
}

void loop() {
  int micValue = analogRead(microphonePin);

  // Normalize the microphone value to be between 0 and 1
  float micValueNormalized = micValue / 1024.0;

  // Copy the microphone value to the input tensor
  inputTensor->data.f[0] = micValueNormalized;

  // Run the inference
  TfLiteStatus status = interpreter->Invoke();
  if (status != kTfLiteOk) {
    Serial.println("Failed to invoke interpreter");
    while (1);
  }

  // Get the output value from the output tensor
  float outputValue = outputTensor->data.f[0];

  // Determine if snoring is detected based on the output value and the threshold
  bool snoreDetected = outputValue > threshold;

  // Display the result on the OLED display
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.print("Snore Detected: ");
  display.println(snoreDetected ? "Yes" : "No");
  display.display();
}
