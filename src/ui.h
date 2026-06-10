#ifndef UI_H
#define UI_H

#include "settings.h"

void drawHomePage(const PipetteSettings &settings);

void drawSetupPage(const PipetteSettings &settings, int selectedSetting);
void drawPreAspirationPage(const PipetteSettings &settings, bool angleOkay);
void drawDispensionReadyPage(const PipetteSettings &settings);

void drawPreDispensePage(
  const PipetteSettings &settings,
  bool angleOkay,
  bool boardStable
);

void drawDispensingPage(const PipetteSettings &settings);
void drawInbetweenPage(const PipetteSettings &settings);
void drawPausePage(const PipetteSettings &settings);
void drawCompletePage(const PipetteSettings &settings);
void drawErrorPage(const PipetteSettings &settings, const char *message);

#endif