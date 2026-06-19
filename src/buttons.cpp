#include "buttons.h"

#define UP_BTN    13
#define OK_BTN    12
#define DOWN_BTN  14

#define DEBOUNCE_MS 25

struct DebouncedButton {
  uint8_t pin;
  bool rawState;
  bool stableState;
  unsigned long changedAt;
  ButtonEvent event;
};

DebouncedButton buttons[] = {
  {UP_BTN, LOW, LOW, 0, BUTTON_UP},
  {OK_BTN, LOW, LOW, 0, BUTTON_OK},
  {DOWN_BTN, LOW, LOW, 0, BUTTON_DOWN}
};

void setupButtons() {
  for (DebouncedButton &button : buttons) {
    pinMode(button.pin, INPUT_PULLDOWN);
    button.rawState = digitalRead(button.pin);
    button.stableState = button.rawState;
    button.changedAt = millis();
  }
}

ButtonEvent handleButtonClicks() {
  unsigned long now = millis();

  for (DebouncedButton &button : buttons) {
    bool reading = digitalRead(button.pin);

    if (reading != button.rawState) {
      button.rawState = reading;
      button.changedAt = now;
    }

    if (reading != button.stableState &&
        now - button.changedAt >= DEBOUNCE_MS) {
      button.stableState = reading;

      if (button.stableState == HIGH) {
        return button.event;
      }
    }
  }

  return BUTTON_NONE;
}

void testButtons() {
  ButtonEvent button = handleButtonClicks();

  if (button == BUTTON_UP) {
    Serial.println("UP clicked");
  } 
  else if (button == BUTTON_OK) {
    Serial.println("OK clicked");
  } 
  else if (button == BUTTON_DOWN) {
    Serial.println("DOWN clicked");
  }
}
