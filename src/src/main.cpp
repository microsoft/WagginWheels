#include <Arduino.h>
#include <TinyWireM.h>  // Use TinyWireM for ATTiny85

#define ADXL345_ADDRESS 0x53
#define ADXL345_REG_DEVID 0x00
#define ADXL345_REG_POWER_CTL 0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0 0x32

#define BUZZER_PIN PB3
#define RED_LED_PIN PB1
#define WHITE_LED_PIN PB4

const int pitchStep = 25;
const int maxPitch = 2500;
const int minPitch = 1500;
const int evaluationCycleDelay = 100; // Adjust the delay as needed

// put function declarations here:
void writeRegister(uint8_t reg, uint8_t value);
void readRegister(uint8_t reg, uint8_t* buffer, uint8_t len);

void setup() {
   TinyWireM.begin();  // Initialize I2C communication for ATTiny85

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);

  // Initialize the sensor
  uint8_t devid;
  readRegister(ADXL345_REG_DEVID, &devid, 1);
  if (devid != 0xE5) {
    digitalWrite(RED_LED_PIN, HIGH);  // Light the red LED
    while (1);  // Infinite loop
  }

  // Set the range to +/- 4G
  writeRegister(ADXL345_REG_DATA_FORMAT, 0x01);
  // Enable measurements
  writeRegister(ADXL345_REG_POWER_CTL, 0x08);

  analogWrite(WHITE_LED_PIN, 50);  // Dimly light the white LED

}

void loop() {
  enum pitchDirection {Up = 1, Down = -1};

  static int currentPitch = minPitch;
  static pitchDirection currentPitchDirection = Up;

  uint8_t buffer;
  readRegister(ADXL345_REG_DATAX0, &buffer, 6);

  int16_t x = (buffer << 8) | buffer;
  int16_t y = (buffer << 8) | buffer;
  int16_t z = (buffer << 8) | buffer;

  // Calculate pitch and roll
  float pitch = atan2(y, sqrt(x * x + z * z)) * 180.0 / PI;
  float roll = atan2(x, sqrt(y * y + z * z)) * 180.0 / PI;

  // Sound the buzzer if pitch or roll is greater than 45 degrees
  // TODO: update this section to cycle through tone frequencies roughly within the normal range of human hearing
  
  if (abs(pitch) > 45 || abs(roll) > 45) {
    tone(BUZZER_PIN, currentPitch);  // Sound a tone based on current pitch
    currentPitch += currentPitch + (pitchStep * currentPitchDirection);
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
  } else {
    noTone(BUZZER_PIN);  // Stop the tone
  }

  delay(evaluationCycleDelay);

}

void writeRegister(uint8_t reg, uint8_t value) {
  TinyWireM.beginTransmission(ADXL345_ADDRESS);
  TinyWireM.write(reg);
  TinyWireM.write(value);
  TinyWireM.endTransmission();
}

void readRegister(uint8_t reg, uint8_t* buffer, uint8_t len) {
  TinyWireM.beginTransmission(ADXL345_ADDRESS);
  TinyWireM.write(reg);
  TinyWireM.endTransmission();
  TinyWireM.requestFrom(ADXL345_ADDRESS, len);
  for (uint8_t i = 0; i < len; i++) {
    buffer[i] = TinyWireM.read();
  }
}

