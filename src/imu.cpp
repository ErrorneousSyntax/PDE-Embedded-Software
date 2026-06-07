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
    int16_t ax, ay, az, gx, gy, gz;

    mpu.getAcceleration(&ax, &ay, &az);

    float AccX = ax / 16384.0;
    float AccY = ay / 16384.0;
    float AccZ = az / 16384.0;

    float pitchAccel = atan2(AccX, AccZ) * 180.0 / PI;
    float rollAccel  = atan2(AccY, AccZ) * 180.0 / PI;

    previousTime = currentTime;
    currentTime = millis();

    elapsedTime = (currentTime - previousTime) / 1000.0;

    mpu.getRotation(&gx, &gy, &gz);

    float pitchGyro = gy / 131.0;
    float rollGyro  = gx / 131.0;

    float accMag = sqrt(
        AccX * AccX +
        AccY * AccY +
        AccZ * AccZ
    );

    bool accelReliable =
        abs(accMag - 1.0) < 0.15;

    float alpha =
        accelReliable ? 0.9 : 1.0;

    pitch =
        alpha * (pitch + pitchGyro * elapsedTime)
        + (1 - alpha) * pitchAccel;

    roll =
        alpha * (roll + rollGyro * elapsedTime)
        + (1 - alpha) * rollAccel;

    return {round(roll), round(pitch)};
}

bool isValidAngle(int roll, int pitch) {
    return (
        abs(roll) < 10 &&
        abs(pitch) < 10
    );
}