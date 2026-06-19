#include "settings.h"

static const float STEPS_PER_UL = 10.0f;
// TODO: calibrate this experimentally

void setupSettings(PipetteSettings &settings) {
  settings.dispenseVolume_uL = 10.0f;
  settings.wellCount = 8;

  settings.incrementEnabled = false;
  settings.incrementPerWell_uL = 0.0f;

  resetExperiment(settings);
}

void resetExperiment(PipetteSettings &settings) {
  settings.currentWell = 0;
  settings.currentDispenseVolume_uL = settings.dispenseVolume_uL;
  settings.dispenseSteps = volumeToSteps(settings.currentDispenseVolume_uL);
}

void updateCurrentDispenseVolume(PipetteSettings &settings) {
  if (settings.incrementEnabled) {
    settings.currentDispenseVolume_uL =
      settings.dispenseVolume_uL + 
      (settings.currentWell * settings.incrementPerWell_uL);
  } else {
    settings.currentDispenseVolume_uL = settings.dispenseVolume_uL;
  }

  settings.dispenseSteps = volumeToSteps(settings.currentDispenseVolume_uL);
}

float calculateTotalAspirationVolume(const PipetteSettings &settings) {
  float totalVolume = settings.dispenseVolume_uL * settings.wellCount;

  if (settings.incrementEnabled) {
    totalVolume += settings.incrementPerWell_uL *
      settings.wellCount * (settings.wellCount - 1) / 2.0f;
  }

  return totalVolume;
}

long volumeToSteps(float volume_uL) {
  return (long)(volume_uL * STEPS_PER_UL);
}
