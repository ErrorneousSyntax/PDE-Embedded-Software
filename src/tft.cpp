#include "tft.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include "integra_logo.h"

// TFT pins
#define TFT_CS    27
#define TFT_RST   25
#define TFT_DC    26
#define TFT_SCLK  22
#define TFT_MOSI  23

#define SCREEN_W  128
#define SCREEN_H  160
#define BORDER_T  5
// Breadboard/jumper wiring can become unreliable at higher SPI clocks.
// 12 MHz remains much faster than software SPI without edge artefacts.
#define TFT_SPI_HZ 12000000

class PipetteTFT : public Adafruit_ST7735 {
 public:
  using Adafruit_ST7735::Adafruit_ST7735;

  void setPanelOffset(int8_t column, int8_t row) {
    setColRowStart(column, row);
  }
};

// Use the ESP32 SPI peripheral. Supplying MOSI/SCLK to the display
// constructor selects much slower software SPI.
PipetteTFT tft = PipetteTFT(TFT_CS, TFT_DC, TFT_RST);

#define WHITE ST77XX_WHITE
#define BLACK ST77XX_BLACK

uint16_t DARK_CHARCOAL;
uint16_t ICE_BLUE;
uint16_t SKY_BLUE;
uint16_t TEAL;
uint16_t CORAL_RED;
uint16_t GREY;
uint16_t OFF_WHITE;
uint16_t PURE_WHITE;
uint16_t BLUSH_PINK;

enum TftPage {
  PAGE_NONE,
  PAGE_HOME,
  PAGE_SETUP,
  PAGE_PRE_ASPIRATION_TILT,
  PAGE_PRE_ASPIRATION_PRESS,
  PAGE_ASPIRATING,
  PAGE_ASPIRATION_COMPLETE,
  PAGE_PRE_DISPENSE_TILT,
  PAGE_PRE_DISPENSE_PRESS,
  PAGE_DISPENSING,
  PAGE_MOVE_TO_WELL,
  PAGE_PAUSE,
  PAGE_COMPLETE,
  PAGE_ERROR
};

TftPage activePage = PAGE_NONE;

struct SetupRenderCache {
  float volume;
  int wells;
  bool incrementEnabled;
  float incrementAmount;
  int selectedRow;
  bool editing;
};

SetupRenderCache setupCache;

void initColours() {
  DARK_CHARCOAL = tft.color565(30, 30, 30);
  ICE_BLUE      = tft.color565(213, 235, 240);
  SKY_BLUE      = tft.color565(135, 200, 215);
  TEAL          = tft.color565(38, 166, 154);
  CORAL_RED     = tft.color565(214, 80, 70);
  GREY          = tft.color565(110, 110, 110);
  OFF_WHITE     = tft.color565(248, 248, 248);
  PURE_WHITE    = tft.color565(255, 255, 255);
  BLUSH_PINK    = tft.color565(245, 185, 180);
}

void drawBorder(int thickness, uint16_t colour) {
  tft.fillRect(0, 0, SCREEN_W, thickness, colour);
  tft.fillRect(0, SCREEN_H - thickness, SCREEN_W, thickness, colour);
  tft.fillRect(0, 0, thickness, SCREEN_H, colour);
  tft.fillRect(SCREEN_W - thickness, 0, thickness, SCREEN_H, colour);
}

bool beginPage(TftPage page, uint16_t borderColour) {
  if (activePage == page) {
    return false;
  }

  activePage = page;
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, borderColour);
  return true;
}

void setupTFT() {
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  // Keep the black-tab colour order, but apply this panel's 2x1 RAM offset.
  // GREENTAB fixes the offset but changes the colour order on this display.
  tft.initR(INITR_BLACKTAB);
  tft.setPanelOffset(2, 1);
  tft.setSPISpeed(TFT_SPI_HZ);
  tft.setRotation(0);
  initColours();
  tft.fillScreen(WHITE);
  activePage = PAGE_NONE;
}

void testTFT() {
  PipetteSettings settings;
  setupSettings(settings);

  drawHomePage(settings);
  delay(1200);

  drawSetupPage(settings, 0, false);
  delay(1200);

  drawPreAspirationTiltPage(settings, false);
  delay(1200);

  drawPreAspirationPressPage();
  delay(1200);

  drawAspiratingPage();
  delay(1200);

  drawAspirationCompletedPage(settings);
  delay(1200);

  drawPreDispenseTiltPage(settings, false, false);
  delay(1200);

  drawPreDispensePressPage();
  delay(1200);

  drawDispensingPage(settings);
  delay(1200);

  drawInbetweenPage(settings);
  delay(1200);

  drawPausePage(settings);
  delay(1200);

  drawDispenseCompletedPage(settings);
  delay(1200);

  drawErrorPage(settings, "Liquid ran out");
  delay(1200);
}

void drawHomePage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_HOME, TEAL)) {
    return;
  }

  tft.drawRGBBitmap(
    (SCREEN_W - INTEGRA_LOGO_WIDTH) / 2,
    18,
    INTEGRA_LOGO_BITMAP,
    INTEGRA_LOGO_WIDTH,
    INTEGRA_LOGO_HEIGHT
  );

  tft.fillRoundRect(19, 65, 90, 38, 8, TEAL);
  tft.fillRoundRect(24, 70, 80, 28, 3, OFF_WHITE);

  tft.setFont(&FreeSans9pt7b);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.setCursor(35, 89);
  tft.print("Setup");

}

void drawSetupRow(
  const PipetteSettings &settings,
  int row,
  int selectedSetting,
  bool editing
) {
  tft.setFont();
  tft.setTextSize(1);

  const int rowX = 14;
  const int rowW = 100;
  const int rowH = 19;
  const int firstRowY = 34;
  const int rowGap = 23;

  int y = firstRowY + row * rowGap;
  bool selected = row == selectedSetting;

  // Clear the complete row area so the previous selection frame disappears.
  tft.fillRect(rowX - 3, y - 3, rowW + 6, rowH + 6, WHITE);

  if (selected && editing) {
    tft.fillRoundRect(rowX - 3, y - 3, rowW + 6, rowH + 6, 6, TEAL);
  } else if (selected) {
    // A filled outer shape plus a smaller light interior creates a solid,
    // thick teal selection frame.
    tft.fillRoundRect(rowX - 3, y - 3, rowW + 6, rowH + 6, 6, TEAL);
    tft.fillRoundRect(rowX + 1, y + 1, rowW - 2, rowH - 2, 3, OFF_WHITE);
  } else {
    tft.fillRoundRect(rowX, y, rowW, rowH, 4, OFF_WHITE);
    tft.drawRoundRect(rowX, y, rowW, rowH, 4, TEAL);
  }

  tft.setTextColor(selected && editing ? WHITE : BLACK);
  tft.setCursor(20, y + 6);

  switch (row) {
    case 0:
      tft.print("Volume");
      tft.setCursor(83, y + 6);
      tft.print(settings.dispenseVolume_uL, 0);
      break;
    case 1:
      tft.print("Wells");
      tft.setCursor(89, y + 6);
      tft.print(settings.wellCount);
      break;
    case 2:
      tft.print("Increment");
      tft.setCursor(84, y + 6);
      tft.print(settings.incrementEnabled ? "On" : "Off");
      break;
    case 3:
      tft.print("Inc uL");
      tft.setCursor(83, y + 6);
      tft.print(settings.incrementPerWell_uL, 0);
      break;
    case 4:
      tft.setCursor(49, y + 6);
      tft.print("BEGIN");
      break;
  }
}

void drawSetupPage(
  const PipetteSettings &settings,
  int selectedSetting,
  bool editing
) {
  bool newPage = beginPage(PAGE_SETUP, TEAL);

  if (newPage) {
    tft.setFont(&FreeSansBold9pt7b);
    tft.setTextSize(1);
    tft.setTextColor(BLACK);
    tft.setCursor(44, 28);
    tft.print("Setup");

    for (int row = 0; row < 5; row++) {
      drawSetupRow(settings, row, selectedSetting, editing);
    }
  } else {
    bool dirtyRows[5] = {false, false, false, false, false};

    dirtyRows[0] = setupCache.volume != settings.dispenseVolume_uL;
    dirtyRows[1] = setupCache.wells != settings.wellCount;
    dirtyRows[2] =
      setupCache.incrementEnabled != settings.incrementEnabled;
    dirtyRows[3] =
      setupCache.incrementAmount != settings.incrementPerWell_uL;

    if (setupCache.selectedRow != selectedSetting) {
      if (setupCache.selectedRow >= 0 && setupCache.selectedRow < 5) {
        dirtyRows[setupCache.selectedRow] = true;
      }
      if (selectedSetting >= 0 && selectedSetting < 5) {
        dirtyRows[selectedSetting] = true;
      }
    }

    if (setupCache.editing != editing &&
        selectedSetting >= 0 && selectedSetting < 5) {
      dirtyRows[selectedSetting] = true;
    }

    for (int row = 0; row < 5; row++) {
      if (dirtyRows[row]) {
        drawSetupRow(settings, row, selectedSetting, editing);
      }
    }
  }

  setupCache = {
    settings.dispenseVolume_uL,
    settings.wellCount,
    settings.incrementEnabled,
    settings.incrementPerWell_uL,
    selectedSetting,
    editing
  };
}

void drawPreAspirationTiltPage(const PipetteSettings &settings, bool angleOkay) {
  if (!beginPage(PAGE_PRE_ASPIRATION_TILT, CORAL_RED)) {
    return;
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(20, 28);
  tft.print("Aspirating");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(40, 70);
  tft.print("Tilt to");

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextSize(2);
  tft.setCursor(40, 105);
  tft.print("90");

  tft.setTextSize(1);
}

void drawPreAspirationPressPage() {
  if (!beginPage(PAGE_PRE_ASPIRATION_PRESS, TEAL)) {
    return;
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(22, 28);
  tft.print("Aspiration");

  tft.fillRoundRect(10, 50, 110, 50, 2, TEAL);
  tft.fillRoundRect(12, 55, 105, 40, 2, OFF_WHITE);

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(14, 70);
  tft.print("Press Select");
  tft.setCursor(20, 88);
  tft.print("to Aspirate");
}

void drawAspiratingPage() {
  if (!beginPage(PAGE_ASPIRATING, TEAL)) {
    return;
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(20, 28);
  tft.print("Aspiration");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(22, 72);
  tft.print("Aspirating");

  tft.fillCircle(30, 95, 10, TEAL);
  tft.fillCircle(64, 95, 10, TEAL);
  tft.fillCircle(98, 95, 10, TEAL);
}

void drawAspirationCompletedPage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_ASPIRATION_COMPLETE, TEAL)) {
    return;
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(22, 28);
  tft.print("Aspiration");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(25, 70);
  tft.print("Aspiration");
  tft.setCursor(20, 86);
  tft.print("Completed");
}

void drawPreDispenseTiltPage(const PipetteSettings &settings, bool angleOkay, bool boardStable) {
  if (!beginPage(PAGE_PRE_DISPENSE_TILT, CORAL_RED)) {
    return;
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(16, 28);
  tft.print("Dispensing");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(40, 70);
  tft.print("Tilt to");

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextSize(2);
  tft.setCursor(40, 105);
  tft.print("45");

  tft.setTextSize(1);
}

void drawPreDispensePressPage() {
  if (!beginPage(PAGE_PRE_DISPENSE_PRESS, TEAL)) {
    return;
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(16, 28);
  tft.print("Dispensing");

  tft.fillRoundRect(10, 50, 110, 50, 2, TEAL);
  tft.fillRoundRect(12, 55, 105, 40, 2, OFF_WHITE);

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(14, 70);
  tft.print("Press Select");
  tft.setCursor(18, 88);
  tft.print("to Dispense");
}

void drawDispensingPage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_DISPENSING, TEAL)) {
    return;
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(16, 28);
  tft.print("Dispensing");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(24, 66);
  tft.print("Well ");
  tft.print(settings.currentWell + 1);
  tft.print("/");
  tft.print(settings.wellCount);

  tft.setCursor(25, 94);
  tft.print(settings.currentDispenseVolume_uL, 0);
  tft.print(" uL");
}

void drawInbetweenPage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_MOVE_TO_WELL, TEAL)) {
    return;
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(16, 28);
  tft.print("Dispensing");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(30, 62);
  tft.print("Move to");
  tft.setCursor(22, 82);
  tft.print("Well ");
  tft.print(settings.currentWell + 1);
  tft.print("/");
  tft.print(settings.wellCount);

  tft.setCursor(22, 116);
  tft.print("OK when ready");
}

void drawPausePage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_PAUSE, TEAL)) {
    return;
  }

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(35, 43);
  tft.print("Paused");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(18, 88);
  tft.print("OK: Continue");
  tft.setCursor(18, 116);
  tft.print("Down: Quit");
}

void drawDispenseCompletedPage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_COMPLETE, TEAL)) {
    return;
  }

  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(20, 70);
  tft.print("Dispensing");
  tft.setCursor(20, 86);
  tft.print("Completed");
}

void drawErrorPage(const PipetteSettings &settings, const char *message) {
  if (!beginPage(PAGE_ERROR, CORAL_RED)) {
    return;
  }

  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(20, 70);
  tft.print("ERROR");

  tft.setCursor(10, 95);
  tft.print(message);
}

void drawPreAspirationPage(const PipetteSettings &settings, bool angleOkay) {
  if (angleOkay) {
    drawPreAspirationPressPage();
  } else {
    drawPreAspirationTiltPage(settings, angleOkay);
  }
}

void drawDispensionReadyPage(const PipetteSettings &settings) {
  drawAspirationCompletedPage(settings);
}

void drawPreDispensePage(const PipetteSettings &settings, bool angleOkay, bool boardStable) {
  if (angleOkay && boardStable) {
    drawPreDispensePressPage();
  } else {
    drawPreDispenseTiltPage(settings, angleOkay, boardStable);
  }
}

void drawCompletePage(const PipetteSettings &settings) {
  drawDispenseCompletedPage(settings);
}
