// #include <Arduino.h>

// #include "buttons.h"
// #include "imu.h"
// #include "settings.h"
// #include "stepper.h"
// #include "tft.h"

// enum State {
//   HOME,
//   SETUP,
//   PRE_ASPIRATE,
//   ASPIRATING,
//   ASPIRATION_COMPLETE,
//   MOVE_TO_WELL,
//   READY_TO_DISPENSE,
//   DISPENSING,
//   PAUSE,
//   COMPLETE,
//   ERROR_STATE
// };

// enum SetupItem {
//   SET_VOLUME,
//   SET_WELLS,
//   SET_INCREMENT_ENABLED,
//   SET_INCREMENT_AMOUNT,
//   SET_START,
//   SETUP_ITEM_COUNT
// };

// State currentState = HOME;
// PipetteSettings settings;
// SetupItem selectedSetting = SET_VOLUME;
// bool editingSetting = false;

// unsigned long stableStartTime = 0;

// const unsigned long STABLE_REQUIRED_MS = 1000;
// const float ASPIRATE_TARGET_ANGLE = 0;
// const float DISPENSE_TARGET_ANGLE = 45.0f;
// const float ANGLE_THRESHOLD = 10.0f;
// const unsigned long IMU_DEBUG_INTERVAL_MS = 250;

// bool angleWithinThreshold(float angle, float target) {
//   // The face-down IMU reports opposite signs depending on tilt direction.
//   // Pipette operation only depends on the amount of tilt, not its sign.
//   return abs(abs(angle) - target) <= ANGLE_THRESHOLD;
// }

// bool stableForOneSecond(bool conditionMet) {
//   if (!conditionMet) {
//     stableStartTime = 0;
//     return false;
//   }

//   if (stableStartTime == 0) {
//     stableStartTime = millis();
//     return false;
//   }

//   return millis() - stableStartTime >= STABLE_REQUIRED_MS;
// }

// void printIMUDebug(const Angles &angles) {
//   static unsigned long lastPrintTime = 0;
//   unsigned long now = millis();

//   if (now - lastPrintTime < IMU_DEBUG_INTERVAL_MS) {
//     return;
//   }

//   lastPrintTime = now;

//   bool targetValid = false;
//   float targetAngle = 0.0f;

//   if (currentState == PRE_ASPIRATE) {
//     targetAngle = ASPIRATE_TARGET_ANGLE;
//     targetValid = (angleWithinThreshold(angles.roll, targetAngle) && angleWithinThreshold(angles.pitch, 0));
//   } else if (
//     currentState == READY_TO_DISPENSE ||
//     currentState == DISPENSING ||
//     currentState == PAUSE
//   ) {
//     targetAngle = DISPENSE_TARGET_ANGLE;
//     targetValid = (angleWithinThreshold(angles.roll, targetAngle) && angleWithinThreshold(angles.pitch, 0));
//   }

//   Serial.print("IMU roll=");
//   Serial.print(angles.roll, 1);
//   Serial.print(" pitch=");
//   Serial.print(angles.pitch, 1);
//   Serial.print(" faceUpValid=");
//   Serial.print(isValidAngle(angles) ? "YES" : "NO");
//   Serial.print(" state=");
//   Serial.print(static_cast<int>(currentState));

//   if (targetAngle > 0.0f) {
//     Serial.print(" target=");
//     Serial.print(targetAngle, 0);
//     Serial.print(" tilt=");
//     Serial.print(abs(angles.pitch), 1);
//     Serial.print(" targetValid=");
//     Serial.print(targetValid ? "YES" : "NO");
//   }

//   Serial.println();
// }

// void drawCurrentState() {
//   switch (currentState) {
//     case HOME:
//       drawHomePage(settings);
//       break;
//     case SETUP:
//       drawSetupPage(settings, selectedSetting, editingSetting);
//       break;
//     case PRE_ASPIRATE:
//       drawPreAspirationTiltPage(settings, false);
//       break;
//     case ASPIRATING:
//       drawAspiratingPage();
//       break;
//     case ASPIRATION_COMPLETE:
//       drawAspirationCompletedPage(settings);
//       break;
//     case MOVE_TO_WELL:
//       drawInbetweenPage(settings);
//       break;
//     case READY_TO_DISPENSE:
//       drawPreDispenseTiltPage(settings, false, false);
//       break;
//     case DISPENSING:
//       drawDispensingPage(settings);
//       break;
//     case PAUSE:
//       drawPausePage(settings);
//       break;
//     case COMPLETE:
//       drawCompletePage(settings);
//       break;
//     case ERROR_STATE:
//       drawErrorPage(settings, "System error");
//       break;
//   }
// }

// void enterState(State newState) {
//   currentState = newState;
//   stableStartTime = 0;
//   drawCurrentState();

//   Serial.print("State: ");
//   Serial.println(static_cast<int>(currentState));
// }

// void adjustSelectedSetting(int direction) {
//   switch (selectedSetting) {
//     case SET_VOLUME:
//       settings.dispenseVolume_uL += direction;
//       if (settings.dispenseVolume_uL < 1.0f) {
//         settings.dispenseVolume_uL = 1.0f;
//       }
//       break;

//     case SET_WELLS:
//       settings.wellCount += direction;
//       if (settings.wellCount < 1) {
//         settings.wellCount = 1;
//       }
//       break;

//     case SET_INCREMENT_ENABLED:
//       settings.incrementEnabled = !settings.incrementEnabled;
//       break;

//     case SET_INCREMENT_AMOUNT:
//       settings.incrementPerWell_uL += direction;
//       if (settings.incrementPerWell_uL < 0.0f) {
//         settings.incrementPerWell_uL = 0.0f;
//       }
//       break;

//     case SET_START:
//     case SETUP_ITEM_COUNT:
//       break;
//   }
// }

// void updateSetup(ButtonEvent button) {
//   if (button == BUTTON_UP || button == BUTTON_DOWN) {
//     if (editingSetting) {
//       int direction = button == BUTTON_UP ? 1 : -1;
//       adjustSelectedSetting(direction);
//     } else {
//       int selection = static_cast<int>(selectedSetting);
//       selection += button == BUTTON_UP ? -1 : 1;

//       if (selection < 0) {
//         selection = SETUP_ITEM_COUNT - 1;
//       } else if (selection >= SETUP_ITEM_COUNT) {
//         selection = 0;
//       }

//       selectedSetting = static_cast<SetupItem>(selection);
//     }

//     drawSetupPage(settings, selectedSetting, editingSetting);
//     return;
//   }

//   if (button != BUTTON_OK) {
//     return;
//   }

//   if (selectedSetting == SET_START) {
//     resetExperiment(settings);
//     enterState(PRE_ASPIRATE);
//     return;
//   }

//   editingSetting = !editingSetting;
//   drawSetupPage(settings, selectedSetting, editingSetting);
// }

// void setup() {
//   Serial.begin(115200);
//   delay(1000);

//   setupButtons();
//   setupStepper();
//   setupIMU();
//   setupSettings(settings);
//   setupTFT();
  

//   enterState(HOME);
//   Serial.println("Pipette MVP started");
// }

// // void loop() {
// //   ButtonEvent button = handleButtonClicks();
// //   Angles angles = getAngle();
// //   float controlAngle = angles.roll;
// //   printIMUDebug(angles);

// //   switch (currentState) {
// //     case HOME:
// //       if (button == BUTTON_OK) {
// //         selectedSetting = SET_VOLUME;
// //         editingSetting = false;
// //         enterState(SETUP);
// //       }
// //       break;

// //     case SETUP:
// //       updateSetup(button);
// //       break;

// //     case PRE_ASPIRATE:
// //       if (stableForOneSecond(
// //             angleWithinThreshold(controlAngle, ASPIRATE_TARGET_ANGLE))) {
// //         long aspirationSteps =
// //           volumeToSteps(calculateTotalAspirationVolume(settings));
// //         stepperMoveRelative(aspirationSteps);
// //         enterState(ASPIRATING);
// //       }
// //       break;

// //     case ASPIRATING:
// //       updateStepper();

// //       if (stepperMoveComplete()) {
// //         enterState(ASPIRATION_COMPLETE);
// //       }
// //       break;

// //     case ASPIRATION_COMPLETE:
// //       if (button == BUTTON_OK) {
// //         enterState(MOVE_TO_WELL);
// //       }
// //       break;

// //     case MOVE_TO_WELL:
// //       if (button == BUTTON_OK) {
// //         enterState(READY_TO_DISPENSE);
// //       }
// //       break;

// //     case READY_TO_DISPENSE:
// //       if (stableForOneSecond(
// //             angleWithinThreshold(controlAngle, DISPENSE_TARGET_ANGLE))) {
// //         updateCurrentDispenseVolume(settings);
// //         stepperMoveRelative(-settings.dispenseSteps);
// //         enterState(DISPENSING);
// //       }
// //       break;

// //     case DISPENSING:
// //       if (!angleWithinThreshold(controlAngle, DISPENSE_TARGET_ANGLE)) {
// //         // Pausing is achieved by temporarily not servicing AccelStepper.
// //         // Its target remains unchanged, so the remaining move can resume.
// //         enterState(PAUSE);
// //         break;
// //       }

// //       updateStepper();

// //       if (stepperMoveComplete()) {
// //         settings.currentWell++;

// //         if (settings.currentWell >= settings.wellCount) {
// //           enterState(COMPLETE);
// //         } else {
// //           enterState(MOVE_TO_WELL);
// //         }
// //       }
// //       break;

// //     case PAUSE:
// //       if (button == BUTTON_DOWN) {
// //         cancelStepperMove();
// //         resetExperiment(settings);
// //         enterState(HOME);
// //       } else if (button == BUTTON_OK &&
// //                  angleWithinThreshold(controlAngle, DISPENSE_TARGET_ANGLE)) {
// //         enterState(DISPENSING);
// //       }
// //       break;

// //     case COMPLETE:
// //       if (button == BUTTON_OK) {
// //         resetExperiment(settings);
// //         enterState(HOME);
// //       }
// //       break;

// //     case ERROR_STATE:
// //       if (button == BUTTON_OK) {
// //         cancelStepperMove();
// //         resetExperiment(settings);
// //         enterState(HOME);
// //       }
// //       break;
// //   }
// // }



// void loop (){
//   Serial.println("Forward");
//   moveStepper(1000);
//   delay(1000);

//   Serial.println("Backward");
//   moveStepper(-1000);
//   delay(1000);
// }


// #include <Arduino.h>
// void setup(){

// }
// void loop() {}

#include <Arduino.h>
#include <TMCStepper.h>

#define EN_PIN          15
#define STEP_PIN        18
#define DIR_PIN         19
#define UART_RX_PIN     16
#define UART_TX_PIN     17

#define R_SENSE         0.11f
#define DRIVER_ADDRESS  0b00

HardwareSerial TMCSerial(2);
TMC2209Stepper uartTestDriver(&TMCSerial, R_SENSE, DRIVER_ADDRESS);

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(EN_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  digitalWrite(EN_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);

  TMCSerial.begin(
    115200,
    SERIAL_8N1,
    UART_RX_PIN,
    UART_TX_PIN
  );

  uartTestDriver.begin();
  delay(100);

  Serial.println("=== TMC2209 UART TEST ===");

  uint8_t result = uartTestDriver.test_connection();

  Serial.print("Connection result: ");
  Serial.println(result);

  if (result == 0) {
    Serial.println("UART connection OK");
  } else if (result == 1) {
    Serial.println("No valid UART response");
  } else if (result == 2) {
    Serial.println("All-zero response");
  }

  Serial.print("IOIN: 0x");
  Serial.println(uartTestDriver.IOIN(), HEX);

  Serial.print("Version: 0x");
  Serial.println(uartTestDriver.version(), HEX);

  Serial.print("GSTAT: 0x");
  Serial.println(uartTestDriver.GSTAT(), HEX);

  Serial.print("DRV_STATUS: 0x");
  Serial.println(uartTestDriver.DRV_STATUS(), HEX);

  uint8_t before = uartTestDriver.IFCNT();

  // Perform one register write.
  uartTestDriver.toff(5);

  uint8_t after = uartTestDriver.IFCNT();

  Serial.print("IFCNT before: ");
  Serial.println(before);

  Serial.print("IFCNT after: ");
  Serial.println(after);

  if (after != before) {
    Serial.println("UART WRITE PASSED");
  } else {
    Serial.println("UART WRITE FAILED");
  }

  Serial.println("=========================");
}

void loop() {
}
