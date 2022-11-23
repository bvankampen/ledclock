#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "fonts.h"
#include "wifi_settings.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 15

#define LED_INTENSITY 0
#define MAX_COL 31

#define SDA 4
#define SCL 5

#define NTP_UPDATE_INTERVAL 3600

const char *ssid     = WIFI_SSID;
const char *password = WIFI_PASSWORD;

u_int32_t lastRefresh = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
RTC_DS3231 rtc;

u_int16_t printNumber(u_int8_t number, u_int16_t currentCol) {
  for(u_int8_t i = 0;i < 3;i++) {
    mx.setColumn(currentCol, numbers[number][i]);
    currentCol--;
  }
  currentCol--;
  return currentCol;
}

u_int16_t printPoints(u_int16_t currentCol) {
  mx.setColumn(currentCol, points);
  currentCol-=2;
  return currentCol;
}

u_int8_t charToInt(char c) {
  return c - 48;
}

void printTime(char time[9]) {
  u_int16_t currentCol = MAX_COL - 1;
  mx.control(0, MAX_DEVICES-1, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (u_int8_t i =0;i < 8;i++) {
    if (time[i]==':') {
      currentCol = printPoints(currentCol);
    } else {
      currentCol = printNumber(charToInt(time[i]), currentCol);
    }
  }
  mx.control(0, MAX_DEVICES-1, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

char* getTime() {
  DateTime now = rtc.now();
  char f[] = "hh:mm:ss";
  return now.toString(f);
}

void setTime() {
  if (timeClient.getEpochTime() - lastRefresh > NTP_UPDATE_INTERVAL) {
  Serial.println("Set Time with NTP");
  while(!timeClient.isTimeSet()) {
    timeClient.update();
    delay(1000);
  }
  Serial.print("RTC Time: ");
  Serial.println(getTime());
  rtc.adjust(DateTime(timeClient.getEpochTime()));
  Serial.print("NTP Time: ");
  Serial.println(timeClient.getFormattedTime());
  lastRefresh = timeClient.getEpochTime();
  }
}




void setup()
{
  
  Wire.begin(SDA, SCL);
  Serial.begin(9600);
  Serial.println("Setup Routine");

  Serial.println("Connect to Wifi");
  WiFi.begin(ssid, password);
   while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println();

  timeClient.begin();

  Serial.println("Setup RTC");
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  

  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, LED_INTENSITY);

  setTime();

}

void loop()
{
  setTime();
  printTime(getTime());
  delay(1000);
}