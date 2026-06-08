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
  delay(500);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  setupStepper();
  setupEncoder();
}

void loop() {
  printEncoderDebug();

  delay(100);
}