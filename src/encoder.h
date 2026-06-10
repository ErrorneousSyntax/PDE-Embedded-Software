#pragma once
#include <Arduino.h>

void setupEncoder();

bool isEncoderConnected();

uint16_t getEncoderRaw();
float getEncoderAngleDeg();
bool isEncoderStable();

void printEncoderDebug();