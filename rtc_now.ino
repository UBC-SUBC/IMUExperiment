#include <Wire.h>
#include "DS3231.h"

RTClib RTC;

void setup () {
    Serial.begin(57600);
    Wire.begin();
}

void loop () {
  
    delay(1000);
  
    DateTime now = RTC.now();

    
    Serial.print(now.hour() , DEC);
    Serial.print(':');
    Serial.print(now.minute() , DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
}
