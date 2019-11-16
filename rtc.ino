#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val) {
  return( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val) {
  return( (val/16*10) + (val%16) );
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
}

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

void displayTime() {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  // send it to the serial monitor
  Serial.print(2000+year, DEC);
  Serial.print("-");
  Serial.print(month, DEC);
  Serial.print("-");
  Serial.print(dayOfMonth, DEC);
   Serial.print(" ");
  Serial.print(hour, DEC);
  Serial.print(":");
  if (minute<10)
    Serial.print("0");
  Serial.print(minute, DEC);
  Serial.print(":");
  if (second<10)
    Serial.print("0");
  Serial.println(second, DEC);
}

void loop() {
  displayTime(); // display the real-time clock data on the Serial Monitor,
  delay(1000); // every second
}
