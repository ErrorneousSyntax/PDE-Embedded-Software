#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

enum WellType {
  FLAT_BOTTOM,
  U_BOTTOM
};

struct PipetteSettings {
  int wellCount;
  WellType wellType;

  float workingVolume_uL;
  float liquidPerDispense_uL;

  long aspirateSteps;
  long dispenseSteps;

  int currentWell;
};

void setupSettings(PipetteSettings &settings);

long volumeToSteps(float volume_uL);

#endif