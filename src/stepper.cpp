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

TMC2209Stepper driver(&Serial2, R_SENSE, 0b00);
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

void testDriver() {
  Serial.println();
  Serial.println("=== TMC2208 driver test ===");

  uint8_t connection = driver.test_connection();

  Serial.print("UART connection: ");
  switch (connection) {
    case 0:
      Serial.println("OK");
      break;
    case 1:
      Serial.println("FAILED (no UART response / loose connection)");
      break;
    case 2:
      Serial.println("FAILED (all-zero response / likely no driver power)");
      break;
    default:
      Serial.print("FAILED (unknown result ");
      Serial.print(connection);
      Serial.println(")");
      break;
  }

  uint8_t gstat = driver.GSTAT();
  uint32_t driverStatus = driver.DRV_STATUS();
  uint32_t inputStatus = driver.IOIN();
  uint8_t version = driver.version();

  Serial.print("GSTAT: 0x");
  Serial.println(gstat, HEX);
  Serial.print("DRV_STATUS: 0x");
  Serial.println(driverStatus, HEX);
  Serial.print("IOIN: 0x");
  Serial.println(inputStatus, HEX);
  Serial.print("TMC version: 0x");
  Serial.println(version, HEX);

  uint8_t ifCountBefore = driver.IFCNT();
  driver.toff(4);  // Known-safe write used to verify UART communication.
  uint8_t ifCountAfter = driver.IFCNT();

  Serial.print("IFCNT before: ");
  Serial.println(ifCountBefore);
  Serial.print("IFCNT after: ");
  Serial.println(ifCountAfter);
  Serial.print("UART write test: ");
  Serial.println(
    ifCountAfter == static_cast<uint8_t>(ifCountBefore + 1)
      ? "PASSED"
      : "FAILED"
  );

  Serial.print("Enable pin level: ");
  Serial.println(digitalRead(EN_PIN) == LOW ? "LOW (enabled)" : "HIGH (disabled)");
  Serial.println("===========================");
  Serial.println();
}

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
