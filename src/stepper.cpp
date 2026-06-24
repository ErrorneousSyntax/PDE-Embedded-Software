#include "stepper.h"
#include "buttons.h"
#include <AccelStepper.h>

// Stepper pins
#define STEP_PIN 18
#define DIR_PIN  19
#define EN_PIN   15

// Standalone STEP/DIR config.
// Current is set by the physical VREF potentiometer on the TMC module.
#define STEPS_PER_REV 200
#define MAX_SPEED 4000
#define ACCELERATION 3000
#define TEST_STEPS 1600
#define HOMING_SPEED 800
#define HOMING_SLOW_SPEED 250
#define HOMING_BACKOFF_STEPS 300
#define HOMING_MAX_STEPS 20000

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

void testDriver() {
  Serial.println();
  Serial.println("=== STEP/DIR stepper test ===");
  Serial.println("UART is not used in this version.");
  Serial.println("Set motor current with the driver's VREF potentiometer.");

  Serial.print("STEP pin: GPIO");
  Serial.println(STEP_PIN);
  Serial.print("DIR pin: GPIO");
  Serial.println(DIR_PIN);
  Serial.print("EN pin: GPIO");
  Serial.println(EN_PIN);
  Serial.print("Enable pin level: ");
  Serial.println(digitalRead(EN_PIN) == LOW ? "LOW (enabled)" : "HIGH (disabled)");

  Serial.print("Moving forward ");
  Serial.print(TEST_STEPS);
  Serial.println(" steps...");
  moveStepper(TEST_STEPS);
  delay(500);

  Serial.print("Moving backward ");
  Serial.print(TEST_STEPS);
  Serial.println(" steps...");
  moveStepper(-TEST_STEPS);

  Serial.println("STEP/DIR test complete.");
  Serial.println("=========================");
  Serial.println();
}

void setupStepper() {
  pinMode(EN_PIN, OUTPUT);

  digitalWrite(EN_PIN, HIGH); // disable driver while setting up pins

  stepper.setMaxSpeed(MAX_SPEED);
  stepper.setAcceleration(ACCELERATION);

  digitalWrite(EN_PIN, LOW); // enable driver
  delay(100);
  testDriver();
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

bool homeStepper() {
  Serial.println();
  Serial.println("=== Stepper homing ===");
  Serial.println("Moving toward limit switch on GPIO17...");

  enableStepper();

  // If the switch is already pressed, move away first so homing can re-approach cleanly.
  if (handleLimitSwitch()) {
    Serial.println("Limit already pressed; backing off...");
    stepper.setSpeed(HOMING_SLOW_SPEED);

    long startPosition = stepper.currentPosition();
    while (handleLimitSwitch()) {
      stepper.runSpeed();

      long travelled = stepper.currentPosition() - startPosition;
      if (travelled < 0) {
        travelled = -travelled;
      }

      if (travelled >= HOMING_MAX_STEPS) {
        Serial.println("Homing failed: switch stayed pressed while backing off.");
        return false;
      }
    }

    delay(100);
  }

  stepper.setSpeed(-HOMING_SPEED);
  long startPosition = stepper.currentPosition();

  while (!handleLimitSwitch()) {
    stepper.runSpeed();

    long travelled = stepper.currentPosition() - startPosition;
    if (travelled < 0) {
      travelled = -travelled;
    }

    if (travelled >= HOMING_MAX_STEPS) {
      Serial.println("Homing failed: limit switch not reached.");
      return false;
    }
  }

  Serial.println("Limit switch hit.");

  // Back off the switch, then approach slowly for a cleaner zero.
  moveStepper(HOMING_BACKOFF_STEPS);
  delay(100);

  Serial.println("Re-approaching slowly...");
  stepper.setSpeed(-HOMING_SLOW_SPEED);

  startPosition = stepper.currentPosition();
  while (!handleLimitSwitch()) {
    stepper.runSpeed();

    long travelled = stepper.currentPosition() - startPosition;
    if (travelled < 0) {
      travelled = -travelled;
    }

    if (travelled >= HOMING_MAX_STEPS) {
      Serial.println("Homing failed: limit switch not reached on slow approach.");
      return false;
    }
  }

  stepper.setCurrentPosition(0);
  Serial.println("Homing complete. Current position set to 0.");
  Serial.println("======================");
  Serial.println();
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
