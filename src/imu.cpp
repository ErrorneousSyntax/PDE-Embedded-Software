#include "imu.h"

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

float roll = 0;
float pitch = 0;

float elapsedTime;
unsigned long currentTime;
unsigned long previousTime;

void setupIMU() {
    Wire.begin(33, 32);

    delay(100);

    mpu.initialize();

    Serial.println(
        mpu.testConnection()
        ? "MPU6050 connected"
        : "MPU6050 failed"
    );

    currentTime = millis();
}

Angles getAngle() {
    int16_t ax, ay, az;

    mpu.getAcceleration(&ax, &ay, &az);

    float AccX = ax / 16384.0f;
    float AccY = ay / 16384.0f;
    float AccZ = az / 16384.0f;

    float mag = sqrt(
        AccX * AccX +
        AccY * AccY +
        AccZ * AccZ
    );

    if (mag == 0) {
        return {0, 0};
    }

    AccX /= mag;
    AccY /= mag;
    AccZ /= mag;

    // Upside-down mount: use -AccZ as the reference axis
    float pitchAngle = atan2(AccX, -AccZ) * 180.0f / PI;
    float rollAngle  = atan2(AccY, -AccZ) * 180.0f / PI;

    return {
        round(rollAngle),
        round(pitchAngle)
    };
}

bool isValidAngle() {
    Angles angles = getAngle();

    float roll = angles.roll;
    float pitch = angles.pitch;

    return (
        abs(roll) < 10 &&
        abs(pitch) < 10
    );
}
bool isIMUStable() {
  static bool firstRun = true;
  static float lastRoll = 0.0f;
  static float lastPitch = 0.0f;
  static unsigned long stableStartTime = 0;

  const float STABLE_TOLERANCE_DEG = 2.0f;
  const unsigned long REQUIRED_STABLE_TIME_MS = 1000;

  Angles angles = getAngle();

  if (!isValidAngle()) {
    firstRun = true;
    stableStartTime = 0;
    return false;
  }

  if (firstRun) {
    lastRoll = angles.roll;
    lastPitch = angles.pitch;
    stableStartTime = millis();
    firstRun = false;
    return false;
  }

  float rollDiff = abs(angles.roll - lastRoll);
  float pitchDiff = abs(angles.pitch - lastPitch);

  if (rollDiff <= STABLE_TOLERANCE_DEG && pitchDiff <= STABLE_TOLERANCE_DEG) {
    if (millis() - stableStartTime >= REQUIRED_STABLE_TIME_MS) {
      return true;
    }
  } else {
    stableStartTime = millis();
  }

  lastRoll = angles.roll;
  lastPitch = angles.pitch;

  return false;
}

void testIMU() {
    Angles angles = getAngle();

    Serial.print("Roll: ");
    Serial.print(angles.roll);

    Serial.print(" | Pitch: ");
    Serial.print(angles.pitch);

    Serial.print(" | Valid angle: ");
    Serial.print(isValidAngle() ? "YES" : "NO");

    Serial.print(" | Stable: ");
    Serial.println(isIMUStable() ? "YES" : "NO");

    delay(100);
}