#include "tft.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// TFT pins
#define TFT_CS    27
#define TFT_RST   25
#define TFT_DC    26
#define TFT_SCLK  22
#define TFT_MOSI  21

#define SCREEN_W  128
#define SCREEN_H  160
#define BORDER_T  10

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

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

void setupTFT() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0);
  initColours();
  tft.fillScreen(WHITE);
}

void testTFT() {
  PipetteSettings settings;

  drawHomePage(settings);
  delay(1200);

  drawSetupPage(settings, 0);
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

  drawDispenseCompletedPage(settings);
  delay(1200);

  drawErrorPage(settings, "Liquid ran out");
  delay(1200);
}

void drawHomePage(const PipetteSettings &settings) {
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

  tft.fillRoundRect(24, 65, 80, 36, 8, TEAL);
  tft.fillRoundRect(29, 70, 70, 26, 3, OFF_WHITE);

  tft.fillRoundRect(24, 100, 80, 36, 8, TEAL);
  tft.fillRoundRect(29, 105, 70, 26, 3, OFF_WHITE);

  tft.setFont(&FreeSans9pt7b);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);

  tft.setCursor(45, 87);
  tft.print("Start");

  tft.fillTriangle(112, 69, 107, 77, 117, 77, TEAL);
  tft.fillTriangle(112, 95, 107, 87, 117, 87, TEAL);

  tft.setCursor(40, 123);
  tft.print("Setup");
}

void drawSetupPage(const PipetteSettings &settings, int selectedSetting) {
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.setCursor(44, 28);
  tft.print("Setup");

  tft.fillRoundRect(14, 37, 100, 36, 6, TEAL);
  tft.fillRoundRect(17, 40, 94, 30, 3, OFF_WHITE);

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(18, 53);
  tft.print("No. of");
  tft.setCursor(18, 67);
  tft.print("Wells");
  tft.setCursor(86, 60);
  tft.print(settings.wellCount);

  tft.fillRoundRect(14, 75, 100, 26, 6, TEAL);
  tft.fillRoundRect(17, 78, 94, 20, 3, OFF_WHITE);
  tft.setCursor(22, 93);
  tft.print("Vol");
  tft.setCursor(86, 93);
  tft.print(settings.dispenseVolume_uL);

  tft.fillRoundRect(14, 105, 100, 26, 6, TEAL);
  tft.fillRoundRect(17, 108, 94, 20, 3, OFF_WHITE);
  tft.setCursor(22, 122);
  tft.print("Step");
  tft.setCursor(86, 122);
  tft.print(settings.incrementPerWell_uL);
}

void drawPreAspirationTiltPage(const PipetteSettings &settings, bool angleOkay) {
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, CORAL_RED);

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
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

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
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

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
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

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
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, CORAL_RED);

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
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

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
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(16, 28);
  tft.print("Dispensing");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(20, 72);
  tft.print("Dispensing");

  tft.fillCircle(30, 95, 10, TEAL);
  tft.fillCircle(64, 95, 10, TEAL);
  tft.fillCircle(98, 95, 10, TEAL);
}

void drawInbetweenPage(const PipetteSettings &settings) {
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(16, 28);
  tft.print("Dispensing");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(30, 70);
  tft.print("Move to");
  tft.setCursor(25, 86);
  tft.print("Next Well");
}

void drawPausePage(const PipetteSettings &settings) {
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(35, 43);
  tft.print("Paused");

  tft.setFont(&FreeSans9pt7b);
  tft.setCursor(30, 95);
  tft.print("Continue");
  tft.setCursor(48, 125);
  tft.print("Quit");
}

void drawDispenseCompletedPage(const PipetteSettings &settings) {
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, TEAL);

  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(BLACK);
  tft.setCursor(20, 70);
  tft.print("Dispensing");
  tft.setCursor(20, 86);
  tft.print("Completed");
}

void drawErrorPage(const PipetteSettings &settings, const char *message) {
  tft.fillScreen(WHITE);
  drawBorder(BORDER_T, CORAL_RED);

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