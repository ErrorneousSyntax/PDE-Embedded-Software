#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <Arduino.h>

void setupStepper();
void enableStepper();
void disableStepper();
void moveStepper(int steps);
void updateStepper();

#endif