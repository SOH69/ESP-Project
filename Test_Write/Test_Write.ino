#include "SPIFFS.h"
 
void setup() {
 
  Serial.begin(115200);
 
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
 
  File file = SPIFFS.open("/Snore.txt", FILE_WRITE);
 
  if (!file) {
    Serial.println("There was an error opening the file for writing");
    return;
  }
  int snore = file.read();
  Serial.println(snore);
  Serial.println(snore++);
  if (file.print(snore)) {
    Serial.println("File was written");
  } else {
    Serial.println("File write failed");
  }
 
  file.close();

  File fileToRead = SPIFFS.open("/Snore.txt");
  if (!fileToRead)
  {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("File Content:");
  while (fileToRead.available())
  {
    Serial.write(fileToRead.read());
  }
  fileToRead.close();
}
 
void loop() {}
