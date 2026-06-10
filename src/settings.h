#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

struct PipetteSettings {
  float dispenseVolume_uL;
  int wellCount;

  bool incrementEnabled;
  float incrementPerWell_uL;

  int currentWell;
  float currentDispenseVolume_uL;

  long dispenseSteps;
};

void setupSettings(PipetteSettings &settings);
void resetExperiment(PipetteSettings &settings);
void updateCurrentDispenseVolume(PipetteSettings &settings);
long volumeToSteps(float volume_uL);

#endif