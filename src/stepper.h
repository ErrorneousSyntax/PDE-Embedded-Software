#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <Arduino.h>

void setupStepper();
void enableStepper();
void disableStepper();
void moveStepper(int steps);
void updateStepper();
void stepperForwards(int steps);
void stepperBackwards(int steps);
bool homeStepper();
bool aspirateFromHome();

void stepperMoveRelative(long steps);
bool stepperMoveComplete();
void stopStepper();
void cancelStepperMove();
void buttonMoveStepper(int button);

#endif
