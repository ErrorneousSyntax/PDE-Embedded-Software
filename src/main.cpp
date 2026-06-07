#include <Arduino.h>
#include "buttons.h"
#include "stepper.h"

void setup() {
    Serial.begin(115200);
    delay(1000);

    setupStepper();
}

void loop() {
    moveStepper(2000);
    delay(1000);
}