/*

Run this sketch on the Tx Zigduino, open the serial monitor at 9600 baud, and type in stuff
Watch the Rx Zigduino output what you've input into the serial port

*/

#include <ZigduinoRadio.h>

void setup()
{
  ZigduinoRadio.begin(0);
  Serial.begin(9600);
  
  ZigduinoRadio.attachError(errHandle);
  ZigduinoRadio.attachTxDone(onXmitDone);
}

void loop()
{
  if (Serial.available())
  {
    ZigduinoRadio.beginTransmission();
  
    while(Serial.available())
      ZigduinoRadio.write(Serial.read());
      
    ZigduinoRadio.endTransmission();
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

void onXmitDone(radio_tx_done_t x)
{
  Serial.println();
  Serial.print("TxDone: ");
  Serial.print((uint16_t)x);
  Serial.println();
}
