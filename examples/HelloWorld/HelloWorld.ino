/*
 * HelloWorld
 * Output to USB terminal on the extension board
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
  // Print normal serial port on main board
  Serial.println("Hello World");

  // Print USB serial port on extension board
  USBSerial.println("Hello World");

  delay(1000);
}
