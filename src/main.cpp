#include <Arduino.h>
#include <Wire.h>


#include "buttons.h"
#include "stepper.h"
#include "imu.h"
#include "encoder.h"
#include "settings.h"
#include "tft.h"


#define SDA_PIN 33
#define SCL_PIN 32

enum State {
  HOME,
  SETUP,
  PRE_ASPIRATE,
  ASPIRATING,
  DISPENSION_READY,
  PRE_DISPENSE,
  DISPENSING,
  INBETWEEN,
  PAUSE,
  COMPLETE,
  ERROR_STATE
};

enum ButtonEvent {
  NO_BUTTON,
  UP_BUTTON,
  DOWN_BUTTON,
  OK_BUTTON,
  PAUSE_BUTTON,
  RESUME_BUTTON,
  BACK_BUTTON
};

enum SetupSelection {
  SELECT_VOLUME,
  SELECT_WELLS,
  SELECT_INCREMENT,
  SELECT_START
};

State currentState = HOME;
State previousState = HOME;

PipetteSettings settings;

SetupSelection selectedSetting = SELECT_VOLUME;

unsigned long stateStartTime = 0;
unsigned long stableStartTime = 0;

const unsigned long STABLE_REQUIRED_MS = 1000;

const char *errorMessage = "";

void enterState(State newState) {
  previousState = currentState;
  currentState = newState;
  stateStartTime = millis();

  switch (newState) {
    case HOME:
      drawHomePage(settings);
      break;

    case SETUP:
      drawSetupPage(settings, selectedSetting);
      break;

    case PRE_ASPIRATE:
      drawPreAspirationPage(settings, false);
      break;

    case ASPIRATING:
      // TODO: replace with aspirate movement if needed
      drawDispensionReadyPage(settings);
      break;

    case DISPENSION_READY:
      drawDispensionReadyPage(settings);
      break;

    case PRE_DISPENSE:
      stableStartTime = 0;
      drawPreDispensePage(settings, false, false);
      break;

    case DISPENSING:
      updateCurrentDispenseVolume(settings);
      stepperMoveRelative(-settings.dispenseSteps);
      drawDispensingPage(settings);
      break;

    case INBETWEEN:
      drawInbetweenPage(settings);
      break;

    case PAUSE:
      stopStepper();
      drawPausePage(settings);
      break;

    case COMPLETE:
      drawCompletePage(settings);
      break;

    case ERROR_STATE:
      stopStepper();
      drawErrorPage(settings, errorMessage);
      break;
  }
}

ButtonEvent readButtonEvent() {
  // Replace this with your real button function.
  return NO_BUTTON;
}

void triggerError(const char *message) {
  errorMessage = message;
  enterState(ERROR_STATE);
}

void updateSetup(ButtonEvent button) {
  if (button == UP_BUTTON) {
    if (selectedSetting == SELECT_VOLUME) {
      settings.dispenseVolume_uL += 1.0f;
    } 
    else if (selectedSetting == SELECT_WELLS) {
      settings.wellCount++;
    } 
    else if (selectedSetting == SELECT_INCREMENT) {
      settings.incrementEnabled = true;
      settings.incrementPerWell_uL += 1.0f;
    }

    drawSetupPage(settings, selectedSetting);
  }

  else if (button == DOWN_BUTTON) {
    if (selectedSetting == SELECT_VOLUME) {
      settings.dispenseVolume_uL -= 1.0f;
      if (settings.dispenseVolume_uL < 0) settings.dispenseVolume_uL = 0;
    } 
    else if (selectedSetting == SELECT_WELLS) {
      settings.wellCount--;
      if (settings.wellCount < 1) settings.wellCount = 1;
    } 
    else if (selectedSetting == SELECT_INCREMENT) {
      settings.incrementPerWell_uL -= 1.0f;

      if (settings.incrementPerWell_uL <= 0) {
        settings.incrementPerWell_uL = 0;
        settings.incrementEnabled = false;
      }
    }

    drawSetupPage(settings, selectedSetting);
  }

  else if (button == OK_BUTTON) {
    if (selectedSetting == SELECT_VOLUME) {
      selectedSetting = SELECT_WELLS;
    } 
    else if (selectedSetting == SELECT_WELLS) {
      selectedSetting = SELECT_INCREMENT;
    } 
    else if (selectedSetting == SELECT_INCREMENT) {
      selectedSetting = SELECT_START;
    } 
    else if (selectedSetting == SELECT_START) {
      resetExperiment(settings);
      enterState(PRE_ASPIRATE);
      return;
    }

    drawSetupPage(settings, selectedSetting);
  }
}

void updateFSM(ButtonEvent button) {
    bool angleOkay = isValidAngle();
    bool boardStable = isEncoderStable();

    if (button == PAUSE_BUTTON && currentState != PAUSE && currentState != ERROR_STATE) {
        enterState(PAUSE);
        return;
    }

    switch (currentState) {
        case HOME:
        if (button == OK_BUTTON) {
            selectedSetting = SELECT_VOLUME;
            enterState(SETUP);
        }
        break;

        case SETUP:
        updateSetup(button);
        break;

        case PRE_ASPIRATE:
        drawPreAspirationPage(settings, angleOkay);

        if (button == BACK_BUTTON) {
            enterState(SETUP);
        }

        if (angleOkay && button == OK_BUTTON) {
            enterState(ASPIRATING);
        }
        break;

        case ASPIRATING:
        // If you add aspiration motor movement later, wait for completion here.
        enterState(DISPENSION_READY);
        break;

        case DISPENSION_READY:
        if (button == OK_BUTTON) {
            enterState(PRE_DISPENSE);
        }
        break;

        case PRE_DISPENSE:
        drawPreDispensePage(settings, angleOkay, boardStable);

        if (!angleOkay || !boardStable) {
            stableStartTime = 0;
            return;
        }

        if (stableStartTime == 0) {
            stableStartTime = millis();
        }

        if (millis() - stableStartTime >= STABLE_REQUIRED_MS) {
            enterState(DISPENSING);
        }
        break;

        case DISPENSING:
        if (stepperMoveComplete()) {
            settings.currentWell++;

            if (settings.currentWell >= settings.wellCount) {
            enterState(COMPLETE);
            } else {
            enterState(INBETWEEN);
            }
        }
        break;

        case INBETWEEN:
        if (button == OK_BUTTON) {
            enterState(PRE_DISPENSE);
        }
        break;

        case PAUSE:
        if (button == RESUME_BUTTON || button == OK_BUTTON) {
            enterState(previousState);
        }
        break;

        case COMPLETE:
        if (button == OK_BUTTON || button == BACK_BUTTON) {
            enterState(HOME);
        }
        break;

        case ERROR_STATE:
        if (button == OK_BUTTON || button == BACK_BUTTON) {
            enterState(HOME);
        }
        break;
    }
}

// void setup() {
//     Serial.begin(115200);
//     delay(1000);

//     Wire.begin(SDA_PIN, SCL_PIN);

//     setupStepper();
//     setupIMU();
//     setupEncoder();

//     setupSettings(settings);

//     enterState(HOME);
// }

// void loop() {
//     ButtonEvent button = readButtonEvent();

//     updateFSM(button);

//     updateStepper();
// }

// ---------CALIBRATION--------------

void setup() {
    Serial.begin(115200);
    delay(1000);

    setupStepper();
    stepperForwards(5000);
    //stepperBackwards(100000);
}

void loop() {
    disableStepper();
}
