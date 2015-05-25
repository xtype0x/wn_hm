/*

Run this sketch on two Zigduinos, open the serial monitor at 9600 baud, and type in stuff
Watch the Rx Zigduino output what you've input into the serial port of the Tx Zigduino

*/

#include <ZigduinoRadio.h>

#define NODE_ID 0x0001
#define CHANNEL 26
#define TX_DO_CARRIER_SENSE 1
#define TX_SOFT_FCS 1

void setup()
{
  ZigduinoRadio.begin(CHANNEL);
  Serial.begin(9600);
  
  ZigduinoRadio.attachError(errHandle);
  ZigduinoRadio.attachTxDone(onXmitDone);
}

void loop()
{
  if (Serial.available())
  {
    ZigduinoRadio.beginTransmission();
    
    Serial.println();
    Serial.print("Tx: ");
    
    while(Serial.available())
    {
      char c = Serial.read();
      Serial.write(c);
      ZigduinoRadio.write(c);
    }
    
    Serial.println(); 
    
    ZigduinoRadio.endTransmission();
  }
  
  
  delay(100);
}

void errHandle(radio_error_t err)
{
  Serial.println();
  Serial.print("Error: ");
  Serial.print((uint8_t)err, 10);
  Serial.println();
}

void onXmitDone(radio_tx_done_t x)
{
  Serial.println();
  Serial.print("TxDone: ");
  Serial.print((uint8_t)x, 10);
  Serial.println();
}
