#include <SPI.h>

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "GxEPD2_display_selection.h"

void setup() {
  SPI.begin(13, 12, 14, 15);
  display.init(115200); // Default 10ms reset pulse, since I have a bare panel with DESPI-C02

  displayHelloWorld();
  
  display.hibernate();
}

void loop() {
  // Runs repeatedly
}

const char HelloWorld[] = "Hello, world!";

void displayHelloWorld() {
  // Use landscape
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);

  // Center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();

  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(HelloWorld);
  }
  while (display.nextPage());
}