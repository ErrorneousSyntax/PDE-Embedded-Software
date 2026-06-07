#include <Arduino.h>

// BUYTTON PINS
#define UP_BTN  14
#define OK_BTN  12
#define DOWN_BTN  13  

bool prevUP = LOW;
bool prevOK = LOW;
bool prevDOWN = LOW;

int handleButtonClicks() {
  bool upNow = digitalRead(UP_BTN);
  bool okNow = digitalRead(OK_BTN);
  bool downNow = digitalRead(DOWN_BTN);

  int result = 0;

  if (upNow == HIGH && prevUP == LOW) {
    result = 1;
  } else if (okNow == HIGH && prevOK == LOW) {
    result = 2;
  } else if (downNow == HIGH && prevDOWN == LOW) {
    result = 3;
  }

  prevUP = upNow;
  prevOK = okNow;
  prevDOWN = downNow;

  delay(30); // super basic debouncing 

  return result;
}
