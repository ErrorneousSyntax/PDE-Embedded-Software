#include <Arduino.h>
#include "buttons.h"

void setup() {
  Serial.begin(115200);
  delay(1000);

  setupButtons();

  Serial.println("Button test started");
}

void loop() {
  String button = handleButtonClicks();

  if (button != "") {
    Serial.println(button);
  }
}