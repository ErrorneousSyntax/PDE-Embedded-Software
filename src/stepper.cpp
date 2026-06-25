#include "stepper.h"
#include "buttons.h"
#include <AccelStepper.h>

// Stepper pins
#define STEP_PIN 18
#define DIR_PIN  19
#define EN_PIN   15

// Standalone STEP/DIR/EN config.
// Current is set by the physical VREF potentiometer on the TMC module.
// Positive movement goes toward the end-stop/limit switch.
// Negative movement backs away from the end stop and aspirates.
#define STEPS_PER_REV 200
#define MAX_SPEED 10000
#define ACCELERATION 8000
#define HOMING_SEEK_STEPS 30000
#define ASPIRATION_STEPS 20000

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

void setupStepper() {
  pinMode(EN_PIN, OUTPUT);

  digitalWrite(EN_PIN, HIGH); // disable driver while setting up pins

  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCELERATION);

  digitalWrite(EN_PIN, LOW); // enable driver
  delay(100);
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
  // Forward is toward the end-stop/limit switch.
  moveStepper(steps);
  delay(500);
}

void stepperBackwards(int steps){
  // Backward is away from the end stop, used for aspiration.
  moveStepper(-steps);
  delay(500);
}

bool homeStepper() {
  Serial.println();
  Serial.println("=== Stepper homing ===");
  Serial.print("Initial limit switch: ");
  Serial.println(handleLimitSwitch() ? "PRESSED" : "released");
  Serial.print("Seek steps: ");
  Serial.println(HOMING_SEEK_STEPS);

  enableStepper();

  Serial.println("Seeking limit switch...");
  stepper.move(HOMING_SEEK_STEPS);

  while (!handleLimitSwitch()) {
    stepper.run();

    if (stepper.distanceToGo() == 0) {
      Serial.println("Homing failed: limit switch not reached.");
      return false;
    }
  }

  Serial.println("Limit switch hit.");
  stepper.setCurrentPosition(0);
  Serial.println("Homing complete. Current position set to 0.");
  Serial.println("======================");
  Serial.println();
  return true;
}

bool aspirateFromHome() {
  if (!homeStepper()) {
    return false;
  }

  enableStepper();
  moveStepper(-ASPIRATION_STEPS);
  return true;
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
