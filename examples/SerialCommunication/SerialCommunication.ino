/*
 * SerialCommunication
 * Communicate between Serial terminal and USB terminal
 */

#include <USBSerial.h>

USBSerial USBSerial;

void setup()
{
  // Use normal serial port on main board
  Serial.begin(115200);

  // Use USB serial port on extension board
  USBSerial.begin(115200);
}

void loop()
{
  // Send from USBSerial to Serial
  while (USBSerial.available()) {
    Serial.write(USBSerial.read());
  }

  // Send from Serial to USBSerial
  while (Serial.available()) {
    USBSerial.write(Serial.read());
  }
}
