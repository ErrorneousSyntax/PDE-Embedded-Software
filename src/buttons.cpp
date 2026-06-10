#include <Arduino.h>

#define UP_BTN    14
#define OK_BTN    12
#define DOWN_BTN  13

bool prevUP = LOW;
bool prevOK = LOW;
bool prevDOWN = LOW;

void setupButtons() {
  pinMode(UP_BTN, INPUT_PULLDOWN);
  pinMode(OK_BTN, INPUT_PULLDOWN);
  pinMode(DOWN_BTN, INPUT_PULLDOWN);
}

int handleButtonClicks() {
  bool upNow = digitalRead(UP_BTN);
  bool okNow = digitalRead(OK_BTN);
  bool downNow = digitalRead(DOWN_BTN);

  int result = 0;

  if (upNow == HIGH && prevUP == LOW) result = 1; // UP IS 1
  else if (okNow == HIGH && prevOK == LOW) result = 2; // OK IS 2
  else if (downNow == HIGH && prevDOWN == LOW) result = 3; // DOWN IS 3

  prevUP = upNow;
  prevOK = okNow;
  prevDOWN = downNow;

  delay(30);
  return result;
}