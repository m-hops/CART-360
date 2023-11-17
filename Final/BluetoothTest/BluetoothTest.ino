#include "BluetoothSerial.h"

void setup() {
  Serial.begin(921600);
  SerialBT.begin(device_name);

}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }

  if (Serial.available()) {
    Serial.write(Serial.read());
  }

  delay(20);
}
