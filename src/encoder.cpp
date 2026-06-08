#include "encoder.h"
#include <AS5600.h>
#include <Wire.h>

// Encoder config
#define AS5600_ADDRESS 0x36
#define AS5600_COUNTS  4096.0f

AS5600 encoder;

void setupEncoder() {
  encoder.begin();

  if (encoder.isConnected()) {
    Serial.println("AS5600 connected");
  } else {
    Serial.println("AS5600 not connected");
  }
}

bool isEncoderConnected() {
  return encoder.isConnected();
}

uint16_t getEncoderRaw() {
  return encoder.readAngle();
}

float getEncoderAngleDeg() {
  return getEncoderRaw() * 360.0f / AS5600_COUNTS;
}

void printEncoderDebug() {
  Serial.print("Encoder raw: ");
  Serial.print(getEncoderRaw());

  Serial.print(" | angle: ");
  Serial.print(getEncoderAngleDeg(), 2);

  Serial.println(" deg");
}