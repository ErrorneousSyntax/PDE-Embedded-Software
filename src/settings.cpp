#include "settings.h"

// Calibration from mass measurements:
//   -10000 steps -> 0.114714 g -> 114.714 uL
//   -5000 steps  -> 0.052143 g -> 52.143 uL
// Pooled conversion: 15000 steps / 166.857 uL = 89.9 steps/uL.
static const float STEPS_PER_UL = 89.9f;
static const long MAX_ASPIRATION_STEPS = 30000;

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
  return (long)(volume_uL * STEPS_PER_UL + 0.5f);
}

long aspirationVolumeToSteps(float volume_uL) {
  long steps = volumeToSteps(volume_uL);

  if (steps > MAX_ASPIRATION_STEPS) {
    return MAX_ASPIRATION_STEPS;
  }

  return steps;
}

long maxAspirationSteps() {
  return MAX_ASPIRATION_STEPS;
}

float maxAspirationVolume_uL() {
  return MAX_ASPIRATION_STEPS / STEPS_PER_UL;
}

int calculateAspirationCount(const PipetteSettings &settings) {
  long totalSteps = volumeToSteps(calculateTotalAspirationVolume(settings));

  if (totalSteps <= 0) {
    return 1;
  }

  return (int)((totalSteps + MAX_ASPIRATION_STEPS - 1) / MAX_ASPIRATION_STEPS);
}
