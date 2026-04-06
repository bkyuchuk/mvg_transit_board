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
// Germany CET/CEST
const char* TZ = "CET-1CEST,M3.5.0,M10.5.0/3";
// MVG Global Station IDs
const char* HASENBERGL_ID = "de:09162:790";
const char* FELDMOCHING_ID = "de:09162:320";
const char* SCHEIDPLATZ_ID = "de:09162:400";
// Use globalId=... for starting station and offset=30 for 30min in the future
const char* MVG_URL = "https://www.mvg.de/api/bgw-pt/v3/departures";

struct Departure {
  const char* label;
  const char* station;
  const char* destination;
  long long times[3];
  uint8_t count = 0; // Acts as state to know how many times were added

  String toString() const {
    String result = "";

    result += label;
    result += " @ ";
    result += station;
    result += " -> ";
    result += destination;
    result += ": ";

    for (int i=0; i < count; i++) {
      result += String(times[i]);
      if (i < count - 1) result += ", ";
    }

    return result;
  }
};

/*
* "Method" to Departure struct that adds a time to its array.
* Maybe structs can have methods in Arduino?
*/
void addDepartureTime(Departure &departure, long long time) {
  if (departure.count < 3) {
    departure.times[departure.count++] = time;
  }
}

/*
* Runs once on boot.
*/
void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin(13, 12, 14, 15); // Send data explicitly to pins 13, 12 and 14 since EPD is listening on those over SPI 
  display.init(115200); // Default 10ms reset pulse, since I have a bare panel with DESPI-C02

  connectToWiFi();
  
  Departure u2 = getDeparturesFor("U2", HASENBERGL_ID, "Messestadt Ost");
  Serial.println(u2.toString());
  
  // Put the display to sleep to prevent voltage stress on the e-ink microcapsules
  display.hibernate();
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
* Syncs with an NTP server to get the timezone specific local time.
* Returns the local time in ms.
*/
long long getLocalTimeMs(String timezone) {
  configTime(0, 0, NTP_SERVER);
  Serial.println("\nWaiting for time sync...");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nTime synchronized!");
  setTimezone(timezone);

  time_t seconds = mktime(&timeinfo); // Convert to epoch (sec)

  return (long long)seconds * 1000; // Convert to ms
}

void setTimezone(String timezone){
  Serial.printf("Setting Timezone to %s\n", timezone.c_str());
  setenv("TZ", timezone.c_str(), 1);
  tzset();
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
Departure getDeparturesFor(const char* label, const char* station, const char* destination) {
  Departure resultDeparture = {label, station, destination, {0, 0, 0}, 0};
  String rawDepartures = fetchDepartures(station, 0);

  DynamicJsonDocument doc(12288); // Reserve enough bytes for JSON response
  DeserializationError error = deserializeJson(doc, rawDepartures.c_str());

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }

  JsonArray array = doc.as<JsonArray>();
  for (JsonObject obj : array) {
    if (!obj["label"] || !obj["destination"]) continue;

    long long departureTime = obj["realtimeDepartureTime"]; // With possible delay
    if (strcmp(label, obj["label"]) == 0 && 
        strcmp(destination, obj["destination"]) == 0) {
          // There is an entry for our label and destination
          addDepartureTime(resultDeparture, departureTime);
        }
  }

  return resultDeparture;
}






