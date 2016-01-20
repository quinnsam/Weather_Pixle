/*
 * This is a minimal example, see extra-examples.cpp for a version
 * with more explantory documentation, example routines, how to
 * hook up your pixels and all of the pixel types that are supported.
 *
 */

#include "application.h"
#include "neopixel.h"
#include "SparkTime.h"


SYSTEM_MODE(AUTOMATIC);

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D0
#define PIXEL_COUNT 1
#define PIXEL_TYPE WS2812B

UDP UDPClient;
SparkTime rtc;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
int wait_counter;
static String cur_cond;
String notify;

void setup()
{
  strip.begin();
  rtc.begin(&UDPClient, "north-america.pool.ntp.org");
  rtc.setTimeZone(-8); // gmt offset
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
  Particle.subscribe("hook-response/get_weather", gotWeatherData, MY_DEVICES);
  // Lets give ourselves 10 seconds before we actually start the program.
  // That will just give us a chance to open the serial monitor before the program sends the request
  for(int i=0;i<10;i++) {
    Serial.println("waiting " + String(10-i) + " seconds before we publish");
    delay(1000);
  }
  wait_counter = 0;
  notify = "";
  //cur_cond = "null";
}
void loop()
{

  if ( rtc.now() > wait_counter) {
    wait_counter = rtc.now() + 900;
    // Let's request the weather, but no more than once every 60 seconds.
    Serial.println("Requesting Weather!");
    // publish the event that will trigger our Webhook
    Particle.publish("get_weather");
  }
  // and wait at least 60 seconds before doing it again
  //delay(600);

  display_color(cur_cond);
  //display_color("Partly Cloudy");
  //Serial.println(cur_cond);
}

// This function will get called when weather data comes in
void gotWeatherData(const char *name, const char *data) {
  String str = String(data);
  cur_cond = tryExtractString(str, "<weather>", "</weather>");
  Serial.println(cur_cond);
}

void display_color(String cond) {
  //Serial.println(cond);
  if (cond == "Overcast" || find_text("Cloudy", cond) != -1) {
    strip.setPixelColor(0, 160, 160, 160);
  } else if (cond == "Sunny") {
    strip.setPixelColor(0, 255, 255, 0);
  } else if (cond == "Rain") {
    strip.setPixelColor(0, 0, 0, 255);
  } else if (cond == "Cloudy") {
    strip.setPixelColor(0, 160, 160, 160);
  } else {
    if (cond != notify && cond != "null") {
      // Set to red to indicate error or non-parsed bit
      strip.setPixelColor(0, 255, 0, 0);
      Serial.println("Sending Push");
      Particle.publish("pushbullet", cond, 60, PRIVATE);
      notify = cond;
    }
  }
  if (rtc.hour(rtc.now()) > 5 && rtc.hour(rtc.now()) < 22) {
    strip.show();
  }

}

int find_text(String needle, String haystack) {
  if (haystack.length() < 3 || needle.length() < 3) {
    return -1;
  }
  int foundpos = -1;
  for (int i = 0; (i < haystack.length() - needle.length()); i++) {
    if (haystack.substring(i,needle.length()+i) == needle) {
      foundpos = i;
    }
  }
  return foundpos;
}

// Returns any text found between a start and end string inside 'str'
// example: startfooend  -> returns foo
String tryExtractString(String str, const char* start, const char* end) {
    if (str == NULL) {
        return NULL;
    }

    int idx = str.indexOf(start);
    if (idx < 0) {
        return NULL;
    }

    int endIdx = str.indexOf(end);
    if (endIdx < 0) {
        return NULL;
    }

    return str.substring(idx + strlen(start), endIdx);
}
