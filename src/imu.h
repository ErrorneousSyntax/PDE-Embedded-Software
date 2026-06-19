#ifndef IMU_H
#define IMU_H

struct Angles {
    float roll;
    float pitch;
};

void setupIMU();
Angles getAngle();
bool isValidAngle(const Angles &angles);
bool isValidAngle();

bool isIMUStable();
void testIMU();
#endif
