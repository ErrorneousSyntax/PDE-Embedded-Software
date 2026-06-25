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
class PipetteTFT : public Adafruit_ST7735 {
 public:
  using Adafruit_ST7735::Adafruit_ST7735;

  void setPanelOffset(int8_t column, int8_t row) {
    setColRowStart(column, row);
  }
};

// Using software SPI, because I discovered limit switch pin was default hardware SPI
PipetteTFT tft = PipetteTFT(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

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

// TFT pages to render
enum TftPage {
  PAGE_NONE,
  PAGE_HOME,
  PAGE_SETUP,
  PAGE_ASPIRATION_PLAN,
  PAGE_HOMING,
  PAGE_PRE_ASPIRATION_TILT,
  PAGE_PRE_ASPIRATION_PRESS,
  PAGE_ASPIRATING,
  PAGE_ASPIRATION_COMPLETE,
  PAGE_PRE_DISPENSE_TILT,
  PAGE_PRE_DISPENSE_PRESS,
  PAGE_DISPENSING,
  PAGE_MOVE_TO_WELL,
  PAGE_PAUSE,
  PAGE_IMU_TEST,
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

void drawTextBox(
  const char *text,
  int x,
  int y,
  int w,
  int h,
  uint16_t fill,
  uint16_t border,
  uint16_t textColour,
  uint8_t textSize = 1
) {
  tft.fillRoundRect(x, y, w, h, 4, fill);
  tft.drawRoundRect(x, y, w, h, 4, border);

  tft.setFont();
  tft.setTextSize(textSize);
  tft.setTextColor(textColour);

  int16_t textX;
  int16_t textY;
  uint16_t textW;
  uint16_t textH;
  tft.getTextBounds(text, 0, 0, &textX, &textY, &textW, &textH);

  int cursorX = x + (w - textW) / 2;
  int cursorY = y + (h - textH) / 2 - textY;
  tft.setCursor(cursorX, cursorY);
  tft.print(text);
}

void drawTitleBox(const char *title, uint16_t colour) {
  drawTextBox(title, 12, 10, 104, 22, colour, colour, WHITE, 1);
}

void drawBodyBox(const char *text, int y, uint16_t borderColour, uint8_t textSize = 1) {
  drawTextBox(text, 14, y, 100, 22, OFF_WHITE, borderColour, BLACK, textSize);
}

void setupTFT() {
  // Keep the black-tab colour order, but apply this panel's 2x1 RAM offset.
  // GREENTAB fixes the offset but changes the colour order on this display.
  tft.initR(INITR_BLACKTAB);
  tft.setPanelOffset(2, 1);
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

  drawAspirationPlanPage(settings, 2);
  delay(1200);

  drawHomingPage();
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

  drawIMUTestPage(settings, 0, 0, true, false);
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

  drawTextBox("SETUP", 24, 72, 80, 30, OFF_WHITE, TEAL, BLACK, 2);
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
  const int firstRowY = 38;
  const int rowGap = 23;

  int y = firstRowY + row * rowGap;
  bool selected = row == selectedSetting;

  // Clear the complete row area so the previous selection frame disappears.
  tft.fillRect(rowX - 3, y - 3, rowW + 6, rowH + 6, WHITE);

  if (selected && editing) {
    tft.fillRoundRect(rowX - 3, y - 3, rowW + 6, rowH + 6, 6, TEAL);
  } else if (selected) {
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
      tft.print("Volume uL");
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
    drawTitleBox("SETUP", TEAL);

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

void drawPreAspirationPressPage() {
  if (!beginPage(PAGE_PRE_ASPIRATION_PRESS, TEAL)) {
    return;
  }

  drawTitleBox("ASPIRATE", TEAL);
  drawBodyBox("PRESS OK", 58, TEAL, 1);
  drawBodyBox("TO START", 90, TEAL, 1);
}

void drawPreAspirationTiltPage(const PipetteSettings &settings, bool angleOkay) {
  if (!beginPage(PAGE_PRE_ASPIRATION_TILT, angleOkay ? TEAL : CORAL_RED)) {
    return;
  }

  uint16_t colour = angleOkay ? TEAL : CORAL_RED;

  drawTitleBox("ASPIRATE", colour);
  drawBodyBox("TILT TO", 58, colour, 1);
  drawTextBox("90", 24, 92, 80, 34, OFF_WHITE, colour, BLACK, 2);
}

void drawAspirationPlanPage(const PipetteSettings &settings, int aspirationCount) {
  if (!beginPage(PAGE_ASPIRATION_PLAN, CORAL_RED)) {
    return;
  }

  char text[24];

  drawTitleBox("ASPIRATE", CORAL_RED);

  snprintf(text, sizeof(text), "%d FILLS", aspirationCount);
  drawBodyBox(text, 48, CORAL_RED, 1);

  snprintf(text, sizeof(text), "MAX %.0f uL", maxAspirationVolume_uL());
  drawBodyBox(text, 76, CORAL_RED, 1);

  drawBodyBox("REFILL LOW", 104, CORAL_RED, 1);
  drawBodyBox("OK START", 132, CORAL_RED, 1);
}

void drawHomingPage() {
  if (!beginPage(PAGE_HOMING, TEAL)) {
    return;
  }

  drawTitleBox("HOMING", TEAL);
  drawBodyBox("FINDING", 60, TEAL, 1);
  drawBodyBox("HOME", 92, TEAL, 2);
}

void drawAspiratingPage() {
  if (!beginPage(PAGE_ASPIRATING, TEAL)) {
    return;
  }

  drawTitleBox("ASPIRATE", TEAL);
  drawBodyBox("RUNNING", 60, TEAL, 1);
  drawTextBox("...", 34, 94, 60, 28, OFF_WHITE, TEAL, BLACK, 2);
}

void drawAspirationCompletedPage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_ASPIRATION_COMPLETE, TEAL)) {
    return;
  }

  drawTitleBox("ASPIRATE", TEAL);
  drawBodyBox("COMPLETE", 70, TEAL, 1);
}

void drawPreDispenseTiltPage(const PipetteSettings &settings, bool angleOkay, bool boardStable) {
  if (!beginPage(PAGE_PRE_DISPENSE_TILT, CORAL_RED)) {
    return;
  }

  drawTitleBox("PRE-DISPENSE", CORAL_RED);
  drawBodyBox("TILT TO", 58, CORAL_RED, 1);
  drawTextBox("45", 34, 92, 60, 34, OFF_WHITE, CORAL_RED, BLACK, 2);
}

void drawPreDispensePressPage() {
  if (!beginPage(PAGE_PRE_DISPENSE_PRESS, TEAL)) {
    return;
  }

  drawTitleBox("DISPENSE", TEAL);
  drawBodyBox("PRESS OK", 58, TEAL, 1);
  drawBodyBox("TO START", 90, TEAL, 1);
}

void drawDispensingPage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_DISPENSING, TEAL)) {
    return;
  }

  char text[24];

  drawTitleBox("DISPENSING", TEAL);

  snprintf(
    text,
    sizeof(text),
    "WELL %d/%d",
    settings.currentWell + 1,
    settings.wellCount
  );
  drawBodyBox(text, 60, TEAL, 1);

  snprintf(text, sizeof(text), "%.0f uL", settings.currentDispenseVolume_uL);
  drawBodyBox(text, 92, TEAL, 2);
}

void drawInbetweenPage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_MOVE_TO_WELL, TEAL)) {
    return;
  }

  char text[24];

  drawTitleBox("MOVE", TEAL);

  snprintf(
    text,
    sizeof(text),
    "WELL %d/%d",
    settings.currentWell + 1,
    settings.wellCount
  );
  drawBodyBox(text, 60, TEAL, 1);
  drawBodyBox("TILT TO", 88, TEAL, 1);
  drawTextBox("90", 34, 116, 60, 28, OFF_WHITE, TEAL, BLACK, 2);
}

void drawPausePage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_PAUSE, TEAL)) {
    return;
  }

  drawTitleBox("PAUSED", TEAL);
  drawBodyBox("TILT TO", 64, TEAL, 1);
  drawTextBox("45", 34, 92, 60, 28, WHITE, TEAL, BLACK, 2);
  drawBodyBox("DOWN QUIT", 126, TEAL, 1);
}

void drawIMUTestPage(
  const PipetteSettings &settings,
  float roll,
  float pitch,
  bool valid,
  bool passed
) {
  bool newPage = beginPage(PAGE_IMU_TEST, valid ? TEAL : CORAL_RED);

  if (newPage) {
    drawTitleBox("IMU TEST", valid ? TEAL : CORAL_RED);
    drawBodyBox("HOLD LEVEL", 48, valid ? TEAL : CORAL_RED, 1);
  }

  char text[24];
  uint16_t statusColour = valid ? TEAL : CORAL_RED;

  tft.fillRect(10, 76, 108, 78, WHITE);

  snprintf(text, sizeof(text), "ROLL %.0f", roll);
  drawBodyBox(text, 78, statusColour, 1);

  snprintf(text, sizeof(text), "PITCH %.0f", pitch);
  drawBodyBox(text, 106, statusColour, 1);

  if (passed) {
    drawBodyBox("PASSED", 134, TEAL, 1);
  } else if (valid) {
    drawBodyBox("STEADY", 134, TEAL, 1);
  } else {
    drawBodyBox("LEVEL", 134, CORAL_RED, 1);
  }
}

void drawDispenseCompletedPage(const PipetteSettings &settings) {
  if (!beginPage(PAGE_COMPLETE, TEAL)) {
    return;
  }

  drawTitleBox("DISPENSE", TEAL);
  drawBodyBox("COMPLETE", 70, TEAL, 1);
}

void drawErrorPage(const PipetteSettings &settings, const char *message) {
  if (!beginPage(PAGE_ERROR, CORAL_RED)) {
    return;
  }

  drawTitleBox("ERROR", CORAL_RED);
  drawBodyBox(message, 70, CORAL_RED, 1);
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