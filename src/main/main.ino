#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

/**************************************************************
 * Monitors Waggin' Wheels device and attached dog for:
 *  - tipover detection: triggered when accelerometer pitch or roll is > 45 degrees or all axis falling
 *
 *  - (TODO) dog temperature: healthy range for dog is between 99.5f (37.5c) and 102.5F (39.2c)
 *
 *  - (TODO) dog heart rate:
 *      Puppies - 120 to 160 beats per minute (BPM).
 *      Small - adult dogs 100 to 140 BPM.
 *      Large - adult dogs 60 to 100 BPM.
 *
 *  - (TODO) dog stress level (via heart rate variability)
 *
 *  - (TODO) dog respiratory rate
 *
 * BOM and connections
 *  - ESP32 microcontroller
 *  - ADXL345 accelerometer (for tipover protection)
 *    | ADXL345 | ESP32 | WIRE COLOR (SUGGESTED)
      | 3.3v    | 3.3v  | RED
      | SQA     | D21   | BLUE
      | SCL     | D22   | YELLOW
      | GND     | GND   | BLACK
 *  - passive buzzer
 *    | BUZZER  | ESP32 | COLOR (SUGGESTED)
      | +       | D2    | GREEN
      | -       | GND   | BLACK
 **************************************************************/

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
#define BUZZER_PIN 2

const int pitchStep = 25;
/*****************************************
  typical range of human hearing is 2k-4k
 *****************************************/
const int maxPitch = 2500;
const int minPitch = 1500;
const int evaluationCycleDelay = 100; // Adjust the delay as needed

void setup() {
  Serial.begin(9600);
  if (!accel.begin()) {
    Serial.println("No ADXL345 detected ... Check your wiring!");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G);
  pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {
  enum pitchDirection {Up = 1, Down = -1};

  static int currentPitch = minPitch;
  static pitchDirection currentPitchDirection = Up;

  sensors_event_t event;
  accel.getEvent(&event);

  // Convert the raw data to g's
  float xg = event.acceleration.x / 9.81;
  float yg = event.acceleration.y / 9.81;
  float zg = event.acceleration.z / 9.81;

  // Calculate the tilt angles
  float roll = atan2(yg, zg) * 180.0 / PI;
  float pitch = atan2(-xg, sqrt(yg * yg + zg * zg)) * 180.0 / PI;

  bool isFalling = false, isTilted = false;

  // Check for free fall (all axes close to 0 g)
  if (abs(xg) < 0.2 && abs(yg) < 0.2 && abs(zg) < 0.2) {
    Serial.println("Free fall detected!");
    // Trigger alert for the owner
    isFalling = true;
  }

  // Check for significant tilt indicating a fall
  if (abs(roll) > 45 || abs(pitch) > 45) {
    Serial.println("Tilt detected! Possible fall.");
    // Trigger alert for the owner
    isTilted = true;
  }

  if(isTilted || isFalling)
  {
    tone(BUZZER_PIN, currentPitch);
    currentPitch = currentPitch + (pitchStep * currentPitchDirection);
    if(currentPitch < minPitch)
    {
      currentPitch = minPitch;
      currentPitchDirection = Up;
    }

    if(currentPitch > maxPitch)
    {
      currentPitch = maxPitch;
      currentPitchDirection = Down;
    }
  }
  else
  {
    noTone(BUZZER_PIN);
  }

  delay(evaluationCycleDelay); 
}
