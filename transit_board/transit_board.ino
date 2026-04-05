#include <SPI.h>

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "GxEPD2_display_selection.h"

void setup() {
  SPI.begin(13, 12, 14, 15);
  display.init(115200); // Default 10ms reset pulse, since I have a bare panel with DESPI-C02

  resetDisplay();
  
  // Put the display to sleep to prevent voltage stress on the e-ink microcapsules
  display.hibernate();
}

void loop() {
  // Runs repeatedly
}

const char HelloWorld[] = "Hello, world!";

void displayHelloWorld() {
  display.setRotation(1); // Use landscape
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

void resetDisplay() {
  // Clear the buffer and make it white
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  
  // Push the white buffer to the screen (false means full refresh, not partial)
  display.display(false);
}