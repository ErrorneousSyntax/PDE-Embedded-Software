#include <Arduino.h>
#include "buttons.h"
#include "stepper.h"
#include "imu.h"

void setup() {
    Serial.begin(115200);
    setupIMU();
}

void loop() {
    Angles angles = getAngle();

    Serial.print("Roll: ");
    Serial.print(angles.roll);

    Serial.print(" Pitch: ");
    Serial.println(angles.pitch);

    delay(50);
}