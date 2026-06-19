#include "stepper.h"
#include <AccelStepper.h>
#include <TMCStepper.h>

// Stepper pins
#define STEP_PIN 18
#define DIR_PIN  19
#define EN_PIN   15
#define RX_PIN   16
#define TX_PIN   17

// Driver config
#define MICROSTEPS    8
#define STEPS_PER_REV   200
#define MAX_SPEED 10000
#define ACCELERATION 5000
#define R_SENSE       0.11f

TMC2208Stepper driver(&Serial2, R_SENSE);
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

void setupStepper() {
  pinMode(EN_PIN, OUTPUT);

  digitalWrite(EN_PIN, HIGH); // disable driver while configuring

  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  driver.begin();
  driver.toff(4);
  driver.rms_current(350);
  driver.microsteps(MICROSTEPS);
  driver.pwm_autoscale(true);
  driver.pwm_autograd(true);
  driver.en_spreadCycle(false);

  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCELERATION);

  digitalWrite(EN_PIN, LOW); // enable driver
}

void enableStepper() {
  digitalWrite(EN_PIN, LOW);
}

void disableStepper() {
  digitalWrite(EN_PIN, HIGH);
}

void moveStepper(int steps) {
  stepper.move(steps);

  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

void updateStepper() {
  stepper.run();
  
}

void stepperForwards(int steps) {
  moveStepper(steps);
  delay(500);
}

void stepperBackwards(int steps){
  moveStepper(-steps);
  delay(500);
}

void stepperMoveRelative(long steps) {
  enableStepper();
  stepper.move(steps);
}

bool stepperMoveComplete() {
  return stepper.distanceToGo() == 0;
}

void stopStepper() {
  stepper.stop();
}

void cancelStepperMove() {
  stepper.setCurrentPosition(stepper.currentPosition());
}

void buttonMoveStepper(int button){
    if (button == 1) {
    moveStepper(1000);
  } 
  else if (button == 3) {
    moveStepper(-1000);
  }
}
