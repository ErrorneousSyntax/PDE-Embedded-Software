#include <Arduino.h>
#include <Wire.h>
#include "buttons.h"
#include "stepper.h"
#include "imu.h"
#include "encoder.h"

// I2C pins
#define SDA_PIN 33
#define SCL_PIN 32

// FSM 
enum State {
  HOME,
  PRE_ASPIRATE,
  ASPIRATING,
  PRE_DISPENSE,
  DISPENSING,
  INBETWEEN,
  ERROR_STATE,
  PAUSE
};

State currentState = HOME;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("IMU test starting...");
  setupStepper();
  setupIMU();
}

void loop() {
    testStepperBackAndForth();
}