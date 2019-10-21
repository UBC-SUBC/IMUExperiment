// Software for testing accuracy and sensitivity of IMU

#include <I2Cdev.h>
#include <MPU6050.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

#define BUTTONPIN 3
#define INDICATORPIN 9
#define DELAYTIME 5
#define DEBOUNCETIME 250

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
unsigned long startTime;

boolean interrupted = false;

void buttonPress(void);

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
    delay(1000);
    Serial.println("Taking Values from the sensor");
    delay(1000);
  } else {
    Serial.println("Connection failed");
  }

  // Button stuff
  pinMode(BUTTONPIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), buttonPress, FALLING);

  // Set up running indicator light
  pinMode(INDICATORPIN, OUTPUT);
  digitalWrite(INDICATORPIN, LOW);
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
      outputFile.print("Time(ms),ax,ay,az,gx,gy,gz,temperature");

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
    Serial.println(temperature);

    // Print the same information to the file
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
    outputFile.print(",");
    outputFile.print(mx);
    outputFile.print(",");
    outputFile.print(my);
    outputFile.print(",");
    outputFile.print(mz);
    outputFile.print(",");
    outputFile.println(temperature);
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
