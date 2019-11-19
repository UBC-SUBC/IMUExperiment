// Software for testing accuracy and sensitivity of IMU

#include <I2Cdev.h>
#include <MPU6050.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <ds3231.h>

#define BUTTONPIN 3
#define INDICATORPIN 9
#define DELAYTIME 5
#define DEBOUNCETIME 250

#define GYROTHRESHOLD 200
#define XINDICPIN 7
#define YINDICPIN 6
#define ZINDICPIN 5

#define DS3231_I2C_ADDRESS 0x68

// IMU variables and objects
MPU6050 IMU;
bool IMUWorking = false;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t mx, my, mz;
int16_t temperature;

File outputFile;

int duration;
int oldFall = 0;
boolean running = false;
boolean initialized = false;
uint32_t startTime;

struct ts t; //structure of the time in year, month, day, hour, etc

boolean interrupted = false;

void buttonPress(void);
void displayTime(File);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // Wait for Serial to connect
  }

  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1)
      ;
  }
  Serial.println("initialization done.");

  // Initialize IMU
  Wire.begin();
  Serial.println("Initializing the sensor");
  IMU.initialize();
  IMUWorking = IMU.testConnection();
  if (IMUWorking == true) {
    Serial.println("Successfully Connected");
    delay(500);
    Serial.println("Calibrating gyro and accel");
    IMU.CalibrateAccel();
    IMU.CalibrateGyro();
    delay(500);
    Serial.println("Taking Values from the sensor");
    delay(500);
  } else {
    Serial.println("Connection failed");
  }

  // Button stuff
  pinMode(BUTTONPIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), buttonPress, RISING);

  // Set up running indicator light
  pinMode(INDICATORPIN, OUTPUT);
  digitalWrite(INDICATORPIN, LOW);

  // Set up gyro indicator lights
  pinMode(XINDICPIN, OUTPUT);
  digitalWrite(XINDICPIN, LOW);
  pinMode(YINDICPIN, OUTPUT);
  digitalWrite(YINDICPIN, LOW);
  pinMode(ZINDICPIN, OUTPUT);
  digitalWrite(ZINDICPIN, LOW);

  // DS3231 set up
  DS3231_init(DS3231_I2C_ADDRESS);
  t.hour = 12;
  t.min = 30;
  t.sec = 0;
  t.mday = 25;
  t.mon = 12;
  t.year = 2019;

  DS3231_set(t);
  
}

void loop() {
  if (interrupted) {
    oldFall = millis();
    if (running == true) {
      // Stuff to close file, etc.
      outputFile.close();
      Serial.println("Closing file...");

      initialized = false;
      running = false;
      digitalWrite(INDICATORPIN, LOW);
      delay(100);
    } else if (running == false && initialized == false) {
      // Stuff to open file, etc.
      // create a new file
      char filename[] = "IMUT00.CSV";
      for (uint8_t i = 0; i < 100; i++) {
        filename[4] = i / 10 + '0';
        filename[5] = i % 10 + '0';
        if (!SD.exists(filename)) {
          // only open a new file if it doesn't exist
          outputFile = SD.open(filename, FILE_WRITE);
          break;  // leave the loop!
        }
      }
      outputFile.print("Real time, Time(ms),ax,ay,az,gx,gy,gz,temperature");

      outputFile.println("");
      Serial.print(filename);
      Serial.println(" successfully initialized!");
      Serial.println("Time(ms),ax,ay,az,gx,gy,gz,temperature");

      startTime = millis();
      digitalWrite(INDICATORPIN, HIGH);
      initialized = true;
      running = true;

    } else {
      Serial.println("Something went wrong. Trying to close file.");
      outputFile.close();
    }
    interrupted = false;
  }

  if (running == true && initialized == true) {
    IMU.getMotion9(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my,
                   &mz);  // get IMU values
    temperature = IMU.getTemperature();

    // Start printing to Serial
    /*
    Serial.print(millis() - startTime);
    Serial.print(",");
    Serial.print(ax);
    Serial.print(",");
    Serial.print(ay);
    Serial.print(",");
    Serial.print(az);
    Serial.print(",");
    Serial.print(gx);
    Serial.print(",");
    Serial.print(gy);
    Serial.print(",");
    Serial.print(gz);
    Serial.print(",");
    Serial.print(mx);
    Serial.print(",");
    Serial.print(my);
    Serial.print(",");
    Serial.print(mz);
    Serial.print(",");
    Serial.println(temperature);*/
    if (abs(gx)>GYROTHRESHOLD){
      Serial.print(millis() - startTime);
      Serial.println(" X over");
      digitalWrite(XINDICPIN, HIGH);
    } else {
      digitalWrite(XINDICPIN, LOW);
    }

    if (abs(gy)>GYROTHRESHOLD){
      Serial.print(millis() - startTime);
      Serial.println(" Y over");
      digitalWrite(YINDICPIN, HIGH);
    } else {
      digitalWrite(YINDICPIN, LOW);
    }

    if (abs(gz)>GYROTHRESHOLD){
      Serial.print(millis() - startTime);
      Serial.println(" Z over");
      digitalWrite(ZINDICPIN, HIGH);
    } else {
      digitalWrite(ZINDICPIN, LOW);
    }

    //print the time data
    readDS3231time(t);
    
    // Print the same information to the file
    /*  displayTime(outputFile);
    outputFile.print(millis() - startTime);
    outputFile.print(",");  
    outputFile.print(ax);
    outputFile.print(",");
    outputFile.print(ay);
    outputFile.print(",");
    outputFile.print(az);
    outputFile.print(",");
    outputFile.print(gx);
    outputFile.print(",");
    outputFile.print(gy);
    outputFile.print(",");
    outputFile.print(gz);
    outputFile.print(",");/*
    outputFile.print(mx);
    outputFile.print(",");
    outputFile.print(my);
    outputFile.print(",");
    outputFile.print(mz);
    outputFile.print(",");
    outputFile.println(temperature); */
  }
  delay(DELAYTIME);
}

void buttonPress(void) {
  Serial.println("Button falling!");
  if (millis() - oldFall > DEBOUNCETIME) {
    interrupted = true;
  } else {
    Serial.println(
        "Not enough time has passed since the last event. Debouncing.");
  }
}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val) {
  return( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val) {
  return( (val/16*10) + (val%16) );
}

void readDS3231time(struct ts time) {
  /*Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);*/
  // request seven bytes of data from DS3231 starting from register 00h

  DS3231_get(&t); //gets time from DS3231

  Serial.print("Date : ");
  Serial.print(t.mday);
  Serial.print("/ ");
  Serial.print(t.mon);
  Serial.print("/ ");
  Serial.print(t.year);
  Serial.print("\t hour: ");
  Serial.print(t.hour);
  Serial.print(": ");
  Serial.print(t.min);
  Serial.print(". ");
  Serial.println(t.sec);

 /* *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read()); */
}

/* void displayTime(File outputFile) {
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  // send it to the serial monitor
  outputFile.print(2000+year, DEC);
  outputFile.print("-");
  outputFile.print(month, DEC);
  outputFile.print("-");
  outputFile.print(dayOfMonth, DEC);
   outputFile.print(" ");
  outputFile.print(hour, DEC);
  outputFile.print(":");
  if (minute<10)
    outputFile.print("0");
  outputFile.print(minute, DEC);
  outputFile.print(":");
  if (second<10)
    outputFile.print("0");
  outputFile.print(second, DEC);
  outputFile.print(",");
} */
