#include <SPI.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "GxEPD2_display_selection.h"

// WiFi Credentials
const char* SSID = "ssid";
const char* PASS = "pass";
// NTP server
const char* NTP_SERVER = "pool.ntp.org";
// MVG Global Station IDs
const char* HASENBERGL_ID = "de:09162:790";
const char* FELDMOCHING_ID = "de:09162:320";
const char* SCHEIDPLATZ_ID = "de:09162:400";
// Use globalId=... for starting station and offset=30 for 30min in the future
const char* MVG_URL = "https://www.mvg.de/api/bgw-pt/v3/departures";

struct Departure {
  const char* label;
  const char* stationId;
  const char* shortStationName;
  const char* destination;
  const char* shortDestinationName;
  long long times[3];
  uint8_t count = 0; // Acts as state to know how many times were added

  /*
  * Used for visualizing on serial monitor.
  */
  String toString() const {
    String result = "";

    result += label;
    result += " @ ";
    result += stationId;
    result += " -> ";
    result += destination;
    result += ": ";

    for (int i=0; i < count; i++) {
      result += String(times[i]);
      if (i < count - 1) result += ", ";
    }

    return result;
  }

  void addDepartureTime(long long time) {
    if (count < 3) {
      times[count++] = time;

      // Simple sort
      for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
          if (times[j] < times[i]) {
            long long temp = times[i];
            times[i] = times[j];
            times[j] = temp;
          }
        }
      }
    }
  }

  /*
  * Used to render the information on the EPD.
  */
  String toDisplayString() const {
    String result = String(label) + " " + shortStationName + " > " + shortDestinationName + ": ";
    for (int i = 0; i < count; i++) {
      result += String(times[i]);
      if (i < count - 1) result += ",";
    }
    return result;
  }
};

/*
* Runs once on boot.
*/
void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin(13, 12, 14, 15); // Send data explicitly to pins 13, 12 and 14 since EPD is listening on those over SPI 
  display.init(115200); // Default 10ms reset pulse, since I have a bare panel with DESPI-C02

  connectToWiFi();
  syncWithNTP();
  
  Departure u2 = getDeparturesFor("U2", HASENBERGL_ID, "Hbgl", "Messestadt Ost", "Messtd.O.", 0);
  Departure s1 = getDeparturesFor("S1", FELDMOCHING_ID, "Feldm.", "Freising", "Freis.", 0);
  Departure u3 = getDeparturesFor("U3", SCHEIDPLATZ_ID, "Scheidplz", "Fürstenried West", "Fuerst.W", 0);

  // Render to E-Paper
  drawToDisplay(u2, s1, u3);
  // Put the display to sleep to prevent voltage stress on the e-ink microcapsules
  display.hibernate();

  // Turn off WiFi to save power before sleeping
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // Sleep for 60sec (calculated in microseconds)
  uint64_t sleepTime = 60 * 1000000ULL; 
  Serial.println("Going to deep sleep for 60sec...");
  esp_sleep_enable_timer_wakeup(sleepTime);
  esp_deep_sleep_start();
}

/*
* Runs repeatedly.
*/
void loop() {}

/*
* Initializes a connection to a WLAN network given its SSID and password.
*/
void connectToWiFi() {
  Serial.print("\nConnecting to WiFi");
  WiFi.begin(SSID, PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
}

/*
* Syncs the time with an NTP server.
* Updates the ESP32 current time.
*/
void syncWithNTP() {
  configTime(0, 0, NTP_SERVER);
  Serial.println("\nWaiting for time sync...");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nTime synchronized!");
}

/*
* Returns the current time in ms.
*/
long long getCurrentTimeMs() {
  return (long long)time(nullptr) * 1000;
}

/*
* Builds the URL & sends HTTP GET for specific station and offset.
* Returns raw JSON string.
*/
String fetchDepartures(const char* stationId, const uint16_t offset) {
  WiFiClientSecure client;
  HTTPClient http;
  
  client.setInsecure(); // Disable certificate validation

  // ESP32 has enough memory
  String url = String(MVG_URL) + "?globalId=" + String(stationId) + "&offset=" + String(offset);
  http.begin(client, url);
  
  int httpResponseCode = http.GET();
  String payload = "{}"; 
  
  if (httpResponseCode > 0) {
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

/*
* Fetches the next three departures given a label (U2, S1, ect.), a station (Hasenbergl, ect.), 
* and a destination (e.g. Messestadt Ost, etc.)
* Returns a newly created Departure object.
*/
Departure getDeparturesFor(
      const char* label,
      const char* stationId,
      const char* shortStationName,
      const char* destination,
      const char* shortDestinationName,
      const uint16_t offset) {
  Departure resultDeparture = {
    label, 
    stationId, 
    shortStationName, 
    destination, 
    shortDestinationName, 
    {0, 0, 0},
    0
  };

  String rawDepartures = fetchDepartures(stationId, offset);

  DynamicJsonDocument doc(12288); // Reserve enough bytes for JSON response
  DeserializationError error = deserializeJson(doc, rawDepartures.c_str());

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }

  JsonArray array = doc.as<JsonArray>();
  for (JsonObject obj : array) {
    if (!obj["label"] || !obj["destination"]) continue;

    long long departureTime = obj["realtimeDepartureTime"]; // Time in ms with possible delay
    if (strcmp(label, obj["label"]) == 0 && 
        strcmp(destination, obj["destination"]) == 0) {
      // There is an entry for our label (U2 etc.) and destination
      uint8_t departureInMin = (departureTime - getCurrentTimeMs()) / 60000; // 60000ms are in 1min
      // Only look at departures less than 1h away
      if (departureInMin < 60) {
        // Add departure time as diff in mins.
        resultDeparture.addDepartureTime(departureInMin);
      }
    }
  }

  return resultDeparture;
}

/*
* Draws the formatted departures to the Waveshare e-paper.
*/
void drawToDisplay(const Departure& d1, const Departure& d2, const Departure& d3) {
  display.setRotation(1); // 1 = Landscape
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    int16_t startX = 5;
    int16_t startY = 20; 
    int16_t lineSpacing = 35;

    // Line 1
    display.setCursor(startX, startY);
    display.print(d1.toDisplayString());

    // Line 2
    display.setCursor(startX, startY + lineSpacing);
    display.print(d2.toDisplayString());

    // Line 3
    display.setCursor(startX, startY + (lineSpacing * 2));
    display.print(d3.toDisplayString());

  } while (display.nextPage());
}




