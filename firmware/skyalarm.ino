//https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/ 
//https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Time/SimpleTime/SimpleTime.ino 
//https://wiki.seeedstudio.com/XIAO_ESP32C3_WiFi_Usage/ 
//https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/ 
//https://github.com/witnessmenow/arduino-sample-api-request/blob/master/ESP32/HTTP_GET_JSON/HTTP_GET_JSON.ino 
//https://github.com/pschatzmann/arduino-audio-tools/wiki/Working-with-PlatformIO

#include <Arduino.h>
#include <WiFi.h> 
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "time.h"
#include "esp_sntp.h"
#include "AudioTools.h"
#include "ESPUI.h"

const int led = D10; // indicator light (external) 
const int SIREN_CTRL = D2; 

// User specific info 
double lat = 37.4830; 
double lng = -122.2359; 

// Other configurable info 
bool notifsOn = true; 

// Notifications 
// Notifications begin 5 minutes before the event by default 
// (Also the grace period for sunrise/sunset checks) 
const int notifySecsBefore = 5 * 60; 
const int timeout = 60*1000; // time out after 1 min (not 1sec)
const long checkInterval = (15*60*1000); // check every 15 minutes 

// Element switches from 0 to 1 when notification is made for an event, and switches back to zero once the notification period for that event is over (or the event can no longer be found). 
// In this order: lunar-eclipse, solar-ecliipse, supermoon, ISS, TBD 
int eventsHappening[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 


// Network 
const char* ssid = "Wokwi-GUEST";
const char* password = ""; 

// Only need HTTPS requests
WiFiClientSecure client; 

HTTPClient http;

// Time 
unsigned long previousMillis = 0; // last time of check 
bool firstTime = true; 

const char* ntpServer = "pool.ntp.org";
//const long gmtOffset_sec = 0; // match (close enough) with UTC 
//const int daylightOffset_sec = 3600;

// Audio 
AudioInfo info(44100, 2, 16);
SineWaveGenerator<int16_t> sineWave(32000);      // subclass of SoundGenerator with max amplitude of 32000
GeneratedSoundStream<int16_t> sound(sineWave);   // Stream generated from sine wave
I2SStream out; 
StreamCopy copier(out, sound);                   // copies sound into i2s

// UI 
uint16_t onOffSwitchID = 0; 

void setup() {
  Serial.begin(115200);
  delay(10); 

  // initialize digital pin led as an output
  pinMode(led, OUTPUT);

  // Connect to wifi  
  Serial.print("\n\nConnecting to wifi network");

  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); 

  // TODO: Actually verify later 
  client.setInsecure();

  // Set configured ntp servers and constant TimeZone/daylightOffset
  configTime(0, 0, ntpServer); // UTC?? pls 

  // Audio config 
  Serial.println("Starting I2S...");
  I2SConfig config = out.defaultConfig(TX_MODE); 
  config.pin_bck = 7; 
  config.pin_ws = 6; 
  config.pin_data = 5; 
  config.copyFrom(info);
  out.begin(config); 

  //ESPUI config 
  ESPUI.setVerbosity(Verbosity::Quiet); 
  onOffSwitchID = ESPUI.addControl(ControlType::Switcher, "Notifications", "1", ControlColor::Emerald, Control::noParent, &onOffCallback);
  ESPUI.begin("Sky Alarm Settings");

  printTime();
}

void loop() {
  // Every 15 minutes, trigger a check 
  unsigned long currentMillis = millis(); 

  if ((currentMillis - previousMillis >= checkInterval) || firstTime) {
    previousMillis = currentMillis; // update last time checked 

    //Serial.print("Free heap at start: ");
    //Serial.println(ESP.getFreeHeap());
    //Serial.println(ESP.getMaxAllocHeap());
    
    // The code below happens every 15 minutes 
    Serial.println("\nChecking for new phenomena...");

    // First of all, is it after sunset + before sunrise? 
    // Grace period = how much before the event notifications begin 
    
    //bool isNight = skyIsDark(notifySecsBefore);

    //Serial.print("The sky is dark: "); 
    //Serial.println(isNight);

    // If dark, check for lunar eclipses in the next 15 minutes 
    // https://opale.imcce.fr/api/v1/phenomena/eclipses/301/2026-08-27
    
    /*
    if (isNight == true) {
      lunarEclipse();
      superMoon(); 
    }
    
    solarEclipse(); 
    */

    ISS(); 

    firstTime = false; 
  }
  
  // Copy sound to out 
  copier.copy(); 
}

void lunarEclipse() {
  Serial.println("Checking for lunar eclipse...");

  // Get the date for the API call 
  char nowISO[25]; 
  nowTimeInUTCISO8601(nowISO, sizeof(nowISO)); 
  tm nowtm = tmFromISO8601(nowISO); 
  time_t nowEpoch = nowTimeInEpoch(); 

  char formattedDate[13]; 
  standardizedDateFromtm(nowtm, formattedDate, sizeof(formattedDate)); 
  String updatedURL = "https://opale.imcce.fr/api/v1/phenomena/eclipses/301/" + String(formattedDate);

  const char* forSureEclipseURL = "https://opale.imcce.fr/api/v1/phenomena/eclipses/301/2026-08-27"; 

  http.useHTTP10(true); 

  http.begin(client, updatedURL);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == HTTP_CODE_OK) {
    // Json parsing setup and filters 
    JsonDocument filter;

    JsonObject filter_response_lunareclipse_0 = filter["response"]["lunareclipse"].add<JsonObject>();

    JsonObject filter_response_lunareclipse_0_duration = filter_response_lunareclipse_0["duration"].to<JsonObject>();
    filter_response_lunareclipse_0_duration["partial"] = true;
    filter_response_lunareclipse_0_duration["total"] = true;

    JsonObject filter_response_lunareclipse_0_events = filter_response_lunareclipse_0["events"].to<JsonObject>();
    filter_response_lunareclipse_0_events["U1"]["date"] = true;
    // Don't need ending time of ecilpse rn 
    filter_response_lunareclipse_0_events["U4"]["date"] = false;
    filter_response_lunareclipse_0["type"] = true;

    JsonDocument doc;

    //Serial.print("Free heap before deserializeJson: ");
    //Serial.println(ESP.getFreeHeap());
    //Serial.println(ESP.getMaxAllocHeap());

    // Time out after 1min instead of 1sec 
    client.setTimeout(timeout); 
    DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(15));

    //Serial.print("Free heap after deserializeJson: ");
    //Serial.println(ESP.getFreeHeap());
    //Serial.println(ESP.getMaxAllocHeap());

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    JsonObject response_lunareclipse_0 = doc["response"]["lunareclipse"][0];

    const char* response_lunareclipse_0_duration_partial = response_lunareclipse_0["duration"]["partial"];


    const char* eclipse_start_date_time = response_lunareclipse_0["events"]["U1"]["date"];

    /*
    char event_key[3]; 
    for (JsonPair response_lunareclipse_0_event : response_lunareclipse_0["events"].as<JsonObject>()) {
      const char* response_lunareclipse_0_event_key = response_lunareclipse_0_event.key().c_str(); // "U1", ...
      strncpy(event_key, response_lunareclipse_0_event_key, sizeof(response_lunareclipse_0_event_key - 1));
      event_key[sizeof(event_key) - 1] = '\0'; // null termination 
      const char* response_lunareclipse_0_event_value_date = response_lunareclipse_0_event.value()["date"];

    }
    */

    const char* response_lunareclipse_0_type = response_lunareclipse_0["type"]; // "PartialEclipse"

    if (response_lunareclipse_0.size() == 0) {
      Serial.println("No lunar eclipse today!");
      eventsHappening[0] = 0; 
    } else if (strcmp(response_lunareclipse_0_type, "PenumbralEclipse") == 0) {
      Serial.println("Just a penumbral eclipse today! It's not very visible with the naked eye."); 
      eventsHappening[0] = 0; 
    } else {
      // It's an eclipse! 
      Serial.print("There's a "); 
      Serial.print(response_lunareclipse_0_type);
      Serial.println(" today!"); 

      // Is it anywhere from -[notifySecsBefore]min to +halfway through the event? 
      time_t eclipse_start_epoch = epochFromISO8601(eclipse_start_date_time);
      int eclipse_partial_duration = secsFromDuration(response_lunareclipse_0_duration_partial);

      if (nowEpoch < eclipse_start_epoch) {
        // Eclipse hasn't started yet - only notify if [notifySecs] before 
        if ((eclipse_start_epoch - nowEpoch) <= notifySecsBefore) {
          if (eventsHappening[0] == 0) {
            // Notification for this event hasn't happened yet 
            notify(); 
            eventsHappening[0] = 1;          
          } 
        } else {
          // It's wayyy before the eclipse. Don't notify 
          eventsHappening[0] = 0; 
          Serial.println("...But not yet."); 
        }
      } else if ((nowEpoch >= eclipse_start_epoch) && (nowEpoch <= (eclipse_start_epoch + (eclipse_partial_duration / 2)))) {
        // Eclipse has started but it isn't halfway through yet 
        if (eventsHappening[0] == 0) {
          // Notification for this event hasn't happened yet 
          notify(); 
          eventsHappening[0] = 1; 
        }
      } else {
        // It's past the eclipse time. Don't notify 
        eventsHappening[0] = 0; 
        Serial.println("...But it's over or close to over.");
      }
    }

    
  } else {
    Serial.printf("HTTP Error code: %d\n", httpResponseCode); 
  }

  http.end();

}

void solarEclipse() {
  Serial.println("Checking for solar eclipse...");

  // Get the date for the API call 
  char nowISO[25]; 
  nowTimeInUTCISO8601(nowISO, sizeof(nowISO)); 
  tm nowtm = tmFromISO8601(nowISO); 
  time_t nowEpoch = nowTimeInEpoch(); 

  char formattedDate[13]; 
  standardizedDateFromtm(nowtm, formattedDate, sizeof(formattedDate)); 
  String updatedURL = "https://opale.imcce.fr/api/v1/phenomena/eclipses/10/" + String(formattedDate) + "?observer=" + String(lat) + "," + String(lng);

  const char* forSureEclipseURL = "https://opale.imcce.fr/api/v1/phenomena/eclipses/10/2029-01-14?observer=37.4830,-122.2359"; 

  http.useHTTP10(true); 

  http.begin(client, updatedURL);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == HTTP_CODE_OK) {
    // Json parsing setup and filters 
    // Stream& input;

    JsonDocument filter;

    JsonObject filter_response_data_0 = filter["response"]["data"].add<JsonObject>();

    JsonObject filter_response_data_0_duration = filter_response_data_0["duration"].to<JsonObject>();
    filter_response_data_0_duration["penumbral"] = true;
    filter_response_data_0_duration["umbral"] = true;

    JsonObject filter_response_data_0_events = filter_response_data_0["events"].to<JsonObject>();
    filter_response_data_0_events["P1"]["date"] = true;
    filter_response_data_0_events["P4"]["date"] = true;
    filter_response_data_0["type"] = true;

    JsonDocument doc;

    client.setTimeout(timeout); 
    DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    JsonObject response_data_0 = doc["response"]["data"][0];

    const char* response_data_0_duration_penumbral = response_data_0["duration"]["penumbral"];
    // response_data_0["duration"]["umbral"] is null

    for (JsonPair response_data_0_event : response_data_0["events"].as<JsonObject>()) {
      const char* response_data_0_event_key = response_data_0_event.key().c_str(); // "P1", "P4"

      const char* response_data_0_event_value_date = response_data_0_event.value()["date"];

    }

    const char* response_data_0_type = response_data_0["type"]; // "ObserverPartialEclipse"

    const char* eclipse_start_date_time = response_data_0["events"]["P1"]["date"];

    if (response_data_0.size() == 0 || response_data_0_duration_penumbral == nullptr) {
      Serial.println("No solar eclipse today!");
      eventsHappening[1] = 0; 
    } else {
      // It's an eclipse! 
      Serial.print("There's a "); 
      Serial.print(response_data_0_type);
      Serial.println(" today!"); 

      // Is it anywhere from -[notifySecsBefore]min to +halfway through the event? 
      time_t eclipse_start_epoch = epochFromISO8601(eclipse_start_date_time);
      int eclipse_partial_duration = secsFromDuration(response_data_0_duration_penumbral);

      if (nowEpoch < eclipse_start_epoch) {
        // Eclipse hasn't started yet - only notify if [notifySecs] before 
        if ((eclipse_start_epoch - nowEpoch) <= notifySecsBefore) {
          if (eventsHappening[1] == 0) {
            // Notification for this event hasn't happened yet 
            notify(); 
            eventsHappening[1] = 1;          
          } 
        } else {
          // It's wayyy before the eclipse. Don't notify 
          eventsHappening[1] = 0; 
          Serial.println("...But not yet."); 
        }
      } else if ((nowEpoch >= eclipse_start_epoch) && (nowEpoch <= (eclipse_start_epoch + (eclipse_partial_duration / 2)))) {
        // Eclipse has started but it isn't halfway through yet 
        if (eventsHappening[1] == 0) {
          // Notification for this event hasn't happened yet 
          notify(); 
          eventsHappening[1] = 1; 
        }
      } else {
        // It's past the eclipse time. Don't notify 
        eventsHappening[1] = 0; 
        Serial.println("...But it's over or close to over.");
      }
    }

    
  } else {
    Serial.printf("HTTP Error code: %d\n", httpResponseCode); 
  }

  http.end();

}

void superMoon() {

  Serial.println("Checking for supermoon...");

  // Get the date for the API call 
  char nowISO[25]; 
  nowTimeInUTCISO8601(nowISO, sizeof(nowISO)); 

  // nowISO contains seconds right now, get rid of last three characters 
  int len = strlen(nowISO); 
  nowISO[len - 3] = '\0'; 

  String updatedURL = "https://svs.gsfc.nasa.gov/api/dialamoon/" + String(nowISO);

  const char* forSureEclipseURL = "https://svs.gsfc.nasa.gov/api/dialamoon/2026-11-24T14:54"; 

  http.useHTTP10(true); 

  http.begin(client, updatedURL);
  int httpResponseCode = http.GET();
  
  if (httpResponseCode == HTTP_CODE_OK) {
    // Json parsing setup and filters 
    // Stream& input;

    JsonDocument filter;
    filter["phase"] = true;
    filter["obscuration"] = true;
    filter["distance"] = true;

    JsonDocument doc;

    client.setTimeout(timeout); 
    DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    float phase = doc["phase"]; // 99.81
    int obscuration = doc["obscuration"]; // 0
    long distance = doc["distance"]; // 360850

    if ((phase >= 99.8f) && (distance <= 367520)) {
      // It's a supermoon! 
      Serial.println("There's a supermoon right now!"); 
      // Since this is a phase of the moon and not an isolated event, not notifying x seconds before. 
      // If anything. 99.8% illumination _is_ the notification before the actual full moon (which is usually a higher illumination %)
      if (eventsHappening[2] == 0) {
        // Hasn't been notified for yet 
        notify(); 
        eventsHappening[2] = 1; 
      } else {
        // Notification for this ongoing event has already happened once 
      }
    } else {
      // No supermoon today 
      Serial.println("No supermoon right now!");
      eventsHappening[2] = 0; 
    }
    
  } else {
    Serial.printf("HTTP Error code: %d\n", httpResponseCode); 
  }

  http.end();
}

void ISS() {
  Serial.println("Checking for ISS passes...");
  String updatedURL = "https://iss-api.polluxlabs.io/iss-pass?lat=" + String(lat) + "&lon=" + String(lng) + "&visible_only=true"; 

  // This may change in the future because this is not time-specific
  const char* forSureURL = "https://iss-api.polluxlabs.io/iss-pass?lat=52.52&lon=13.40"; 

  http.begin(client, updatedURL);
  int code = http.GET();

  if (code == 200) {
    JsonDocument doc;
    deserializeJson(doc, http.getStream());
    const char* riseTime = doc["passes"][0]["rise"]["time"];
    const char* culminationTime = doc["passes"][0]["culmination"]["time"];
    const char* compassDirection = doc["passes"][0]["rise"]["compass"]; 
    const char* riseAzimuth = doc["passes"][0]["rise"]["azimuth_deg"];
    // Max elevation in degrees 
    int elevation  = doc["passes"][0]["culmination"]["elevation_deg"];

    JsonObject passes_0 = doc["passes"][0];

    if (passes_0.size() == 0) {
      // No visible passes 
      Serial.println("No visible passes!"); 
      eventsHappening[3] = 0; 
    } else {
      // Visible passes exist - but is it happening soon/now? 
      Serial.println("There's a visible pass!");
      // Check upcoming one (passes[0]) 
      char nowISO[25]; 
      nowTimeInUTCISO8601(nowISO, sizeof(nowISO)); 
      time_t nowEpoch = nowTimeInEpoch(); 
      time_t passEpoch = epochFromISO8601(riseTime); 
      time_t culminationEpoch = epochFromISO8601(culminationTime);

      if (nowEpoch < passEpoch) {
        // Pass hasn't happened yet - less than 15+ mins before it happens? 
        int threshold; 

        if (notifySecsBefore >= 15) {
          // Use the user defined setting, it's fine 
          threshold = notifySecsBefore; 
        } else {
          // The user defined notif setting is too short to reliably alert for the ISS - make it 15mins
          threshold = (15*60); 
        }

        if ((passEpoch - nowEpoch) <= threshold) {
          // It is happening soon! Notify 
          if (eventsHappening[3] == 0) {
            notify(); 
            eventsHappening[3] = 1; 
          }
        } else {
          // It is too early to notify 
          Serial.println("...But not yet."); 
          eventsHappening[3] = 0; 
        }
      } else {
        // Pass is happening or has already happened 
        // Only notify if it is at culmination time or before 
        if (nowEpoch <= culminationEpoch)  {
          // It's started but still early enough to issue a notif 
          if (eventsHappening[3] == 0) {
            notify(); 
            eventsHappening[3] = 1; 
          } 
        } else {
          // It's too late to notify :( 
          Serial.println("...But it's already happened."); 
          eventsHappening[3] = 0; 
        }
      }

    }

  } else {
    Serial.printf("HTTP error: %d\n", code);
  }
  http.end();
}

JsonDocument jsonFromRequest(const char* urlBase, const char* everythingElse) {

  // Use the ArduinoJson Assistant to calculate this:
  // StaticJsonDocument<192> doc;
  // DynamicJsonDocument is deprecated 
  JsonDocument doc; //For ESP32/ESP8266 you'll mainly use dynamic.

  // Opening connection to server (Use 80 as port if HTTP)
  if (!client.connect(urlBase, 443))
  {
    Serial.println(F("Connection failed"));
    return doc;
  }

  // give the esp a breather
  yield();

  // Send HTTP request
  client.print(F("GET "));
  // This is the second half of a request (everything that comes after the base URL)
  client.print(everythingElse); // %2C == ,
  client.println(F(" HTTP/1.1"));

  //Headers
  client.print(F("Host: "));
  client.println(urlBase);

  client.println(F("Cache-Control: no-cache"));

  if (client.println() == 0)
  {
    Serial.println(F("Failed to send request"));
    return doc;
  }
  //delay(100);
  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0)
  {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return doc;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders))
  {
    Serial.println(F("Invalid response"));
    return doc;
  }

  // This is probably not needed for most, but I had issues
  // with the Tindie api where sometimes there were random
  // characters coming back before the body of the response.
  // This will cause no harm to leave it in
  // peek() will look at the character, but not take it off the queue
  while (client.available() && client.peek() != '{')
  {
    char c = 0;
    client.readBytes(&c, 1);
    yield();
    //Serial.print(c);
    //Serial.println("BAD");
  }

  //  // While the client is still availble read each
  //  // byte and print to the serial monitor
  //  while (client.available()) {
  //    char c = 0;
  //    client.readBytes(&c, 1);
  //    Serial.print(c);
  //  }

  
  DeserializationError error = deserializeJson(doc, client);

  // if (!error)

  return doc; 

}

void notify() {
  if (notifsOn) {
    // Placeholder for when testing without actual hardware 
    digitalWrite(led, HIGH);   // turn the LED on 
    Serial.println("WEE OOO WEE OOO"); 
    Serial.println("A SPACE THING IS HAPPENING");
    delay(10000);               // wait for 10 seconds
    digitalWrite(led, LOW);    // turn the LED off
    delay(10);
  }
}

void siren() {
  digitalWrite(SIREN_CTRL, HIGH);
  sineWave.begin(info, N_B4);  
  delay(10000); 
  digitalWrite(SIREN_CTRL, LOW); 
  sineWave.end(); 
}

#pragma region Various time formatting functions 
void nowTimeInUTCISO8601(char* result, size_t resultSize) {
  struct tm timeinfo; 
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time :(");
    result[0] = '\0'; // result will be empty?? Hopefully?? When unable to obtain time 
    return; 
  }

  strftime(result, resultSize, "%Y-%m-%dT%H:%M:%S", &timeinfo);
  
  //Serial.println(&timeinfo, "%Y-%m-%dT%H:%M:%S");
  // Day of week/month/day of month/year/hour(24hr)/hour(12hr)/minute/second 
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void localTimeToUTCISO8601(char* result, size_t resultSize) {
  int tzHour = 0, tzMin = 0; 
  char sign = '+';

}

void tmToISO8601(tm time, char* result, size_t resultSize) {
  // Formats to: YYYY-MM-DDTHH:MM:SS
  strftime(result, resultSize, "%Y-%m-%dT%H:%M:%S", &time);
}

// Epoch and seconds  
time_t epochFromISO8601(const char* timestr) {
  struct tm tm_struct = {0};
  int year, month, day, hour, minute, second;

  int parsed = sscanf(timestr, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);
  
  if (parsed != 6) {
    return 0; // Parsing failed
  }

  // Adjust values to fit standard tm struct specs
  tm_struct.tm_year = year - 1900; // Years since 1900
  tm_struct.tm_mon  = month - 1;   // Months 0-11
  tm_struct.tm_mday = day;
  tm_struct.tm_hour = hour;
  tm_struct.tm_min  = minute;
  tm_struct.tm_sec  = second;
  tm_struct.tm_isdst = -1;         // Let system automatically figure out DST

  return mktime(&tm_struct);       // Converts tm struct back to UNIX epoch seconds
}
int secsFromDuration(const char* duration) {
  if (duration != nullptr && strlen(duration) >= 8) {
    char buffer[3]; 
    buffer[0] = duration[0];
    buffer[1] = duration[1];
    buffer[2] = '\0';
    int hr = atoi(buffer); 

    buffer[0] = duration[3];
    buffer[1] = duration[4];
    int min = atoi(buffer);

    buffer[0] = duration[6];
    buffer[1] = duration[7];
    int sec = atoi(buffer); 

    return ((hr * 3600) + (min * 60) + sec);
  } else {
    return -1; 
  }
}
time_t nowTimeInEpoch() {
  char utc[25]; 
  nowTimeInUTCISO8601(utc, sizeof(utc)); 
  return epochFromISO8601(utc);
}

// tm stuff 
// ONLY WORKS WITH UTC 
tm tmFromISO8601(const char* timestr) {
  struct tm tm_struct = {0};
  int year, month, day, hour, minute, second;

  int parsed = sscanf(timestr, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);
  
  if (parsed != 6) {
    return tm_struct; // Parsing failed
  }

  // Adjust values to fit standard tm struct specs
  tm_struct.tm_year = year - 1900; // Years since 1900
  tm_struct.tm_mon  = month - 1;   // Months 0-11
  tm_struct.tm_mday = day;
  tm_struct.tm_hour = hour;
  tm_struct.tm_min  = minute;
  tm_struct.tm_sec  = second;
  tm_struct.tm_isdst = -1;         // Let system automatically figure out DST

  return tm_struct;       // Converts tm struct back to UNIX epoch seconds
}
void standardizedDateFromtm(tm time, char* result, size_t resultSize) {
  char yearBuf[5]; 
  char monBuf[3]; 
  char dayBuf[3];

  // tm_year is years since 1900 
  int year = time.tm_year + 1900;
  itoa(year, yearBuf, 10); 
  //tm_mon starts from 0  
  int month = time.tm_mon + 1; 
  if (month < 10) {
    // Single-digit; add extra 0 in front 
    char tempBuf[2]; 
    itoa(month, tempBuf, 10); 
    monBuf[0] = '0'; 
    monBuf[1] = tempBuf[0]; 
    monBuf[2] = '\0'; 
  } else {
    itoa(month, monBuf, 10); 
  }
  //tm_mday is day of month 
  int day = time.tm_mday; 
  if (day < 10) {
    // Single-digit; add extra 0 in front 
    char tempBuf[2]; 
    itoa(day, tempBuf, 10);
    dayBuf[0] = '0';
    dayBuf[1] = tempBuf[0]; 
    dayBuf[2] = '\0'; 
  } else {
    itoa(day, dayBuf, 10);
  }
  
  // Combine 
  snprintf(result, resultSize, "%s-%s-%s", yearBuf, monBuf, dayBuf); 
}

double diffSecsISOTimes(const char* timeStr1, const char* timeStr2) {
  time_t epoch1 = epochFromISO8601(timeStr1);
  time_t epoch2 = epochFromISO8601(timeStr2);

  if (epoch1 == 0 || epoch2 == 0) {
    Serial.println("Error parsing one of the timestamps.");
    return 0.0;
  }

  // Calculate the raw difference in seconds
  double diff = difftime(epoch1, epoch2);

  // diff > 0: Current local time is LATER than target time 
  // diff < 0: Current local time is EARLIER than target time 
  return diff; 
}

void printTime() {
  struct tm timeinfo; 
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  Serial.println(&timeinfo, "%Y-%m-%dT%H:%M:%S");
  // Day of week/month/day of month/year/hour(24hr)/hour(12hr)/minute/second 
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

#pragma endregion 


#pragma region Time/weather checks  

bool skyIsDark(const int gracePeriod) {
  // Is it after sunset + before sunrise?  
  // Grace period: count the sky n seconds before a sunset and the n seconds after a sunrise as "dark"
  bool skyIsDark; 

  // Automatically returns sunrise/sunset data today in UTC so that it fits with everything else 
  // String for ease of concatenation  
  String secondPart = "/v2?lat=" + String(lat) + "&lng=" + String(lng) + "&tz=Etc/UTC";
  JsonDocument sunTimesJson = jsonFromRequest("api.sunrise-sunset.org", secondPart.c_str());

  const char* sunrise = sunTimesJson["sunrise"];
  const char* sunset = sunTimesJson["sunset"]; 
  char nowISO[25]; 
  nowTimeInUTCISO8601(nowISO, sizeof(nowISO));

  time_t sunriseEpoch = epochFromISO8601(sunrise);
  time_t sunsetEpoch = epochFromISO8601(sunset);  
  time_t nowEpoch = epochFromISO8601(nowISO); 

  sunriseEpoch += gracePeriod; 
  sunsetEpoch -= gracePeriod; 

  // Assuming sunrise is always earlier than sunset in response 
  if (nowEpoch < sunriseEpoch && nowEpoch < sunsetEpoch) {
    // Sun is down 
    skyIsDark = true; 
  } else if (nowEpoch > sunriseEpoch && nowEpoch > sunsetEpoch) {
    // Sun is down 
    skyIsDark = true; 
  } else { 
    // Now is between sunrise and sunset for the day - sun is up 
    skyIsDark = false; 
  }

  if (sunriseEpoch > sunsetEpoch) {
    // Sunrise is actually later than sunset in this response - reverse every outcome 
    skyIsDark = !skyIsDark; 
  }

  return skyIsDark; 
}

#pragma endregion

#pragma region ESPUI functions 

void onOffCallback(Control* sender, int value) {
  // When on/off toggle switch clicked 
  switch (value) {
    case S_ACTIVE: 
      // Turn on notifs 
      notifsOn = true; 
      break;
    case S_INACTIVE: 
      // Turn off notifs
      notifsOn = false;  
      break; 
  }
}

#pragma endregion