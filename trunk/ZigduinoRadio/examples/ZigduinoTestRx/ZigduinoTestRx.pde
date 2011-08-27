/*

Run this sketch on the Rx Zigduino, open the serial monitor at 9600 baud
The output should be what the Tx Zigduino has received over the serial port

*/

#include <ZigduinoRadio.h>

void setup()
{
  ZigduinoRadio.begin(0);
  Serial.begin(9600);
  
  ZigduinoRadio.attachError(errHandle);
}

void loop()
{
  if (ZigduinoRadio.available())
  {
    Serial.println();
    Serial.print("Rx: ");
    
    while(ZigduinoRadio.available())
      Serial.write(ZigduinoRadio.read());
      
    Serial.println();
    Serial.print("LQI: ");
    Serial.print(ZigduinoRadio.getLqi(), 10);
    Serial.print(", RSSI: ");
    Serial.print(ZigduinoRadio.getRssi(), 10);
    Serial.println();
  }
  
  delay(100);
}

void errHandle(radio_error_t err)
{
  Serial.println();
  Serial.print("Error: ");
  Serial.print((uint16_t)err);
  Serial.println();
}
