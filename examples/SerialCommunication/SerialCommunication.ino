/*
 * SerialCommunication
 * Communicate between Serial terminal and USB terminal
 */

#include <USBSerial.h>

USBSerial UsbSerial;

void setup()
{
  // Use normal serial port on main board
  Serial.begin(115200);

  // Use USB serial port on extension board
  UsbSerial.begin(115200);
}

void loop()
{
  // Send from USBSerial to Serial
  while (UsbSerial.available()) {
    Serial.write(UsbSerial.read());
  }

  // Send from Serial to USBSerial
  while (Serial.available()) {
    UsbSerial.write(Serial.read());
  }
}
