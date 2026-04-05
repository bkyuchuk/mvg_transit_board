#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// Define the pin connections for the Waveshare ESP32 e-Paper Driver Board
static const uint8_t EPD_BUSY = 25;
static const uint8_t EPD_CS   = 15;
static const uint8_t EPD_RST  = 26;
static const uint8_t EPD_DC   = 27;
// Note: SPI SCK is GPIO 13, SPI DIN (MOSI) is GPIO 14 on this specific board.
// The library handles the SPI pins automatically based on the ESP32 hardware SPI.

// Instantiate the display. 
// NOTE: Waveshare has several versions of the 2.13" screen. 
// "GxEPD2_213_B74" is the common V3/V4 Black & White panel. 
// If your screen looks garbled, you may need to swap this to GxEPD2_213_BN (V2) or GxEPD2_213.
GxEPD2_BW<GxEPD2_213_B74, GxEPD2_213_B74::HEIGHT> display(GxEPD2_213_B74(/*CS=*/ EPD_CS, /*DC=*/ EPD_DC, /*RST=*/ EPD_RST, /*BUSY=*/ EPD_BUSY));

void setup() {
  Serial.begin(115200);
  Serial.println("Starting e-Paper clear routine...");

  // Initialize Display
  SPI.begin(13, 12, 14, 15);
  display.init(115200, true, 2, false); 
  
  // Clear the buffer and make it white
  display.setFullWindow();
  display.fillScreen(GxEPD_WHITE);
  
  // Push the white buffer to the screen (false means full refresh, not partial)
  display.display(false);
  
  // Put the display to sleep to prevent voltage stress on the e-ink microcapsules
  display.hibernate();
  
  Serial.println("Display cleared and hibernating.");
}

void loop() {
  // Nothing needed here
}