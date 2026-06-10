#include "settings.h"

static const float STEPS_PER_UL = 10.0f; 
// TODO: calibrate this properly for your syringe / lead screw / pipette geometry

void setupSettings(PipetteSettings &settings) {
  settings.wellCount = 96;
  settings.wellType = FLAT_BOTTOM;

  settings.workingVolume_uL = 100.0f;
  settings.liquidPerDispense_uL = 10.0f;

  settings.aspirateSteps = volumeToSteps(settings.workingVolume_uL);
  settings.dispenseSteps = volumeToSteps(settings.liquidPerDispense_uL);

  settings.currentWell = 0;
}

long volumeToSteps(float volume_uL) {
  return (long)(volume_uL * STEPS_PER_UL);
}