#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

enum ButtonEvent {
  BUTTON_NONE,
  BUTTON_UP,
  BUTTON_OK,
  BUTTON_DOWN
};

void setupButtons();
ButtonEvent handleButtonClicks();
bool handleLimitSwitch();
void testButtons();

#endif
