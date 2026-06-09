#include <Arduino.h>
#include <Wire.h>
#include "buttons.h"
#include "stepper.h"
#include "imu.h"
#include "encoder.h"

// I2C pins
#define SDA_PIN 33
#define SCL_PIN 32

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("IMU test starting...");
  setupIMU();
}

void loop() {
  Angles angles = getAngle();

  Serial.println("Roll: ");
  Serial.println(angles.roll);

  Serial.println(" | Pitch: ");
  Serial.println(angles.pitch);

  delay(50);
}