#ifndef tft_h
#define tft_H

#include <Arduino.h>
#include "settings.h"

void setupTFT();
void testTFT();

void drawHomePage(const PipetteSettings &settings);
void drawSetupPage(
  const PipetteSettings &settings,
  int selectedSetting,
  bool editing
);
void drawPreAspirationTiltPage(const PipetteSettings &settings, bool angleOkay);
void drawPreAspirationPressPage();
void drawAspirationPlanPage(const PipetteSettings &settings, int aspirationCount);
void drawHomingPage();
void drawAspiratingPage();
void drawAspirationCompletedPage(const PipetteSettings &settings);
void drawPreDispenseTiltPage(const PipetteSettings &settings, bool angleOkay, bool boardStable);
void drawPreDispensePressPage();
void drawDispensingPage(const PipetteSettings &settings);
void drawInbetweenPage(const PipetteSettings &settings);
void drawPausePage(const PipetteSettings &settings);
void drawIMUTestPage(
  const PipetteSettings &settings,
  float roll,
  float pitch,
  bool valid,
  bool passed
);
void drawDispenseCompletedPage(const PipetteSettings &settings);
void drawErrorPage(const PipetteSettings &settings, const char *message);

void drawPreAspirationPage(const PipetteSettings &settings, bool angleOkay);
void drawDispensionReadyPage(const PipetteSettings &settings);
void drawPreDispensePage(const PipetteSettings &settings, bool angleOkay, bool boardStable);
void drawCompletePage(const PipetteSettings &settings);

#endif
