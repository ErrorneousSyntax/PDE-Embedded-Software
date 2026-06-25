#include <Arduino.h>

#include "buttons.h"
#include "imu.h"
#include "settings.h"
#include "stepper.h"
#include "tft.h"

enum State {
  HOME,
  SETUP,
  ASPIRATION_PLAN,
  HOMING,
  PRE_ASPIRATE,
  ASPIRATING,
  ASPIRATION_COMPLETE,
  MOVE_TO_WELL,
  READY_TO_DISPENSE,
  DISPENSING,
  PAUSE,
  COMPLETE,
  ERROR_STATE
};

enum SetupItem {
  SET_VOLUME,
  SET_WELLS,
  SET_INCREMENT_ENABLED,
  SET_INCREMENT_AMOUNT,
  SET_START,
  SETUP_ITEM_COUNT
};

State currentState = HOME;
PipetteSettings settings;
SetupItem selectedSetting = SET_VOLUME;
bool editingSetting = false;
bool aspirationMoveStarted = false;
bool homingStarted = false;
bool homingComplete = false;

unsigned long stableStartTime = 0;
unsigned long lastIMUReadTime = 0;
ButtonEvent latestButton = BUTTON_NONE;
Angles latestAngles = {0.0f, 0.0f};

const unsigned long STABLE_REQUIRED_MS = 1000;
const unsigned long IMU_READ_INTERVAL_MS = 50;
const float ASPIRATE_TARGET_ROLL = -5.0f;
const float ASPIRATE_TARGET_PITCH = -19.0f;
const float BETWEEN_WELLS_TARGET_ROLL = 5.0f;
const float DISPENSE_TARGET_ROLL = 45.0f;
const float ANGLE_THRESHOLD = 10.0f;

bool angleWithinThreshold(float angle, float target) {
  return abs(angle - target) <= ANGLE_THRESHOLD;
}

bool aspirationAngleOkay() {
  return (
    angleWithinThreshold(latestAngles.roll, ASPIRATE_TARGET_ROLL) &&
    angleWithinThreshold(latestAngles.pitch, ASPIRATE_TARGET_PITCH)
  );
}

bool dispenseAngleOkay() {
  return angleWithinThreshold(abs(latestAngles.roll), DISPENSE_TARGET_ROLL);
}

bool betweenWellsAngleOkay() {
  return angleWithinThreshold(abs(latestAngles.roll), BETWEEN_WELLS_TARGET_ROLL);
}

bool stableForOneSecond(bool conditionMet) {
  if (!conditionMet) {
    stableStartTime = 0;
    return false;
  }

  if (stableStartTime == 0) {
    stableStartTime = millis();
    return false;
  }

  return millis() - stableStartTime >= STABLE_REQUIRED_MS;
}

void drawCurrentState() {
  switch (currentState) {
    case HOME:
      drawHomePage(settings);
      break;
    case SETUP:
      drawSetupPage(settings, selectedSetting, editingSetting);
      break;
    case ASPIRATION_PLAN:
      drawAspirationPlanPage(settings, calculateAspirationCount(settings));
      break;
    case HOMING:
      drawHomingPage();
      break;
    case PRE_ASPIRATE:
      drawPreAspirationPressPage();
      break;
    case ASPIRATING:
      drawAspiratingPage();
      break;
    case ASPIRATION_COMPLETE:
      drawAspirationCompletedPage(settings);
      break;
    case MOVE_TO_WELL:
      drawInbetweenPage(settings);
      break;
    case READY_TO_DISPENSE:
      drawPreDispenseTiltPage(settings, false, false);
      break;
    case DISPENSING:
      drawDispensingPage(settings);
      break;
    case PAUSE:
      drawPausePage(settings);
      break;
    case COMPLETE:
      drawCompletePage(settings);
      break;
    case ERROR_STATE:
      drawErrorPage(settings, "System error");
      break;
  }
}

void enterState(State newState) {
  currentState = newState;
  stableStartTime = 0;

  if (newState == ASPIRATING) {
    aspirationMoveStarted = false;
  }

  if (newState == HOMING) {
    homingStarted = false;
    homingComplete = false;
  }

  drawCurrentState();
}

void adjustSelectedSetting(int direction) {
  switch (selectedSetting) {
    case SET_VOLUME:
      settings.dispenseVolume_uL += direction;
      if (settings.dispenseVolume_uL < 1.0f) {
        settings.dispenseVolume_uL = 1.0f;
      }
      break;

    case SET_WELLS:
      settings.wellCount += direction;
      if (settings.wellCount < 1) {
        settings.wellCount = 1;
      }
      break;

    case SET_INCREMENT_ENABLED:
      settings.incrementEnabled = !settings.incrementEnabled;
      break;

    case SET_INCREMENT_AMOUNT:
      settings.incrementPerWell_uL += direction;
      if (settings.incrementPerWell_uL < 0.0f) {
        settings.incrementPerWell_uL = 0.0f;
      }
      break;

    case SET_START:
    case SETUP_ITEM_COUNT:
      break;
  }
}

void updateSetup(ButtonEvent button) {
  if (button == BUTTON_UP || button == BUTTON_DOWN) {
    if (editingSetting) {
      int direction = button == BUTTON_UP ? 1 : -1;
      adjustSelectedSetting(direction);
    } else {
      int selection = static_cast<int>(selectedSetting);
      selection += button == BUTTON_UP ? -1 : 1;

      if (selection < 0) {
        selection = SETUP_ITEM_COUNT - 1;
      } else if (selection >= SETUP_ITEM_COUNT) {
        selection = 0;
      }

      selectedSetting = static_cast<SetupItem>(selection);
    }

    drawSetupPage(settings, selectedSetting, editingSetting);
    return;
  }

  if (button != BUTTON_OK) {
    return;
  }

  if (selectedSetting == SET_START) {
    resetExperiment(settings);
    if (calculateAspirationCount(settings) > 1) {
      enterState(ASPIRATION_PLAN);
      return;
    }

    enterState(HOMING);
    return;
  }

  editingSetting = !editingSetting;
  drawSetupPage(settings, selectedSetting, editingSetting);
}

void handleHome() {
  if (latestButton == BUTTON_OK) {
    selectedSetting = SET_VOLUME;
    editingSetting = false;
    enterState(SETUP);
  }
}

void handleSetup() {
  updateSetup(latestButton);
}

void handleAspirationPlan() {
  if (latestButton == BUTTON_OK) {
    enterState(HOMING);
  }
}

void handleHoming() {
  if (homingStarted) {
    return;
  }

  homingStarted = true;

  if (homeStepper()) {
    homingComplete = true;
    enterState(PRE_ASPIRATE);
  } else {
    homingComplete = false;
    enterState(ERROR_STATE);
  }
}

void handlePreAspirate() {
  if (!homingComplete) {
    enterState(HOMING);
    return;
  }

  if (!stableForOneSecond(aspirationAngleOkay())) {
    drawPreAspirationTiltPage(settings, aspirationAngleOkay());
    return;
  }

  drawPreAspirationPressPage();

  if (latestButton != BUTTON_OK) {
    return;
  }

  enterState(ASPIRATING);
}

void handleAspirating() {
  if (!homingComplete) {
    enterState(ERROR_STATE);
    return;
  }

  if (!aspirationMoveStarted) {
    long aspirationSteps = aspirationVolumeToSteps(calculateTotalAspirationVolume(settings));
    stepperMoveRelative(-aspirationSteps);
    aspirationMoveStarted = true;
  }

  updateStepper();

  if (stepperMoveComplete()) {
    enterState(ASPIRATION_COMPLETE);
  }
}

void handleAspirationComplete() {
  enterState(MOVE_TO_WELL);
}

void handleMoveToWell() {
  drawInbetweenPage(settings);

  if (stableForOneSecond(betweenWellsAngleOkay())) {
    enterState(READY_TO_DISPENSE);
  }
}

void handleReadyToDispense() {
  drawPreDispenseTiltPage(settings, dispenseAngleOkay(), true);

  if (!stableForOneSecond(dispenseAngleOkay())) {
    return;
  }

  updateCurrentDispenseVolume(settings);
  stepperMoveRelative(settings.dispenseSteps);
  enterState(DISPENSING);
}

void handleDispensing() {
  if (!dispenseAngleOkay()) {
    enterState(PAUSE);
    return;
  }

  updateStepper();

  if (!stepperMoveComplete()) {
    return;
  }

  settings.currentWell++;

  if (settings.currentWell >= settings.wellCount) {
    enterState(COMPLETE);
  } else {
    enterState(MOVE_TO_WELL);
  }
}

void handlePause() {
  if (latestButton == BUTTON_DOWN) {
    cancelStepperMove();
    resetExperiment(settings);
    enterState(HOME);
  } else if (stableForOneSecond(dispenseAngleOkay())) {
    enterState(DISPENSING);
  }
}

void handleComplete() {
  if (latestButton == BUTTON_OK) {
    resetExperiment(settings);
    enterState(HOME);
  }
}

void handleError() {
  if (latestButton == BUTTON_OK) {
    cancelStepperMove();
    resetExperiment(settings);
    enterState(HOME);
  }
}

void updateState() {
  switch (currentState) {
    case HOME:
      handleHome();
      break;

    case SETUP:
      handleSetup();
      break;

    case ASPIRATION_PLAN:
      handleAspirationPlan();
      break;

    case HOMING:
      handleHoming();
      break;

    case PRE_ASPIRATE:
      handlePreAspirate();
      break;

    case ASPIRATING:
      handleAspirating();
      break;

    case ASPIRATION_COMPLETE:
      handleAspirationComplete();
      break;

    case MOVE_TO_WELL:
      handleMoveToWell();
      break;

    case READY_TO_DISPENSE:
      handleReadyToDispense();
      break;

    case DISPENSING:
      handleDispensing();
      break;

    case PAUSE:
      handlePause();
      break;

    case COMPLETE:
      handleComplete();
      break;

    case ERROR_STATE:
      handleError();
      break;
  }
}

bool motorIsMoving() {
  return currentState == ASPIRATING || currentState == DISPENSING;
}

bool imuReadDue() {
  return millis() - lastIMUReadTime >= IMU_READ_INTERVAL_MS;
}

void updateIMUIfNeeded() {
  if (!imuReadDue()) {
    return;
  }

  lastIMUReadTime = millis();
  latestAngles = getAngle();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  setupButtons();
  setupStepper();
  setupIMU();
  setupSettings(settings);
  setupTFT();

  enterState(HOME);
}

void loop() {
  latestButton = handleButtonClicks();

  if (motorIsMoving()) {
    updateStepper();
    updateIMUIfNeeded();
    updateStepper();
  } else {
    updateIMUIfNeeded();
  }

  updateState();
}
