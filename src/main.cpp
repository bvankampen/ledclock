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
#define MAX_COL 30

#define SDA 4
#define SCL 5

// #define WIFI

#define CLOCK_ON 8
#define CLOCK_OFF 23

#define NTP_UPDATE_INTERVAL 3600

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

u_int32_t lastRefresh = 0;

#ifdef WIFI
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
#endif

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
RTC_DS3231 rtc;

char lastTime[7];

u_int16_t buffer[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// u_int16_t printNumber(u_int8_t number, u_int16_t currentCol)
// {
//   for (u_int8_t i = 0; i < 3; i++)
//   {
//     mx.setColumn(currentCol, numbers[number][i]);
//     currentCol--;
//   }
//   currentCol--;
//   return currentCol;
// }

// u_int16_t printPoints(u_int16_t currentCol)
// {
//   mx.setColumn(currentCol, points);
//   currentCol -= 2;
//   return currentCol;
// }

u_int8_t charToInt(char c)
{
  return c - 48;
}

// void printTime(char time[9])
// {
//   u_int16_t currentCol = MAX_COL - 1;
//   mx.control(0, MAX_DEVICES - 1, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
//   for (u_int8_t i = 0; i < 8; i++)
//   {
//     if (time[i] == ':')
//     {
//       currentCol = printPoints(currentCol);
//     }
//     else
//     {
//       currentCol = printNumber(charToInt(time[i]), currentCol);
//     }
//   }
//   mx.control(0, MAX_DEVICES - 1, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
// }

void fillBuffer(char *newTime)
{
  int buf = 0;
  for (int i = 0; i < 7; i++)
  {
    numbers[charToInt(newTime[i])];
    for (int j = 0; j < 3; j++)
    {
      buffer[buf] = (numbers[charToInt(lastTime[i])][j] << 8) + numbers[charToInt(newTime[i])][j];
      buf++;
    }
  }
}

void printBuffer()
{
  int col = MAX_COL;
  mx.control(0, MAX_DEVICES - 1, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (int i = 0; i < 18; i++)
  {
    if (i % 3 == 0 && i > 0)
      col--;
    if (i % 6 == 0 && i > 0) {
      mx.setColumn(col, points);
      col-=2;
    }
    mx.setColumn(col, (buffer[i] >> 8) & 0b00111110);
    col--;
  }
  mx.control(0, MAX_DEVICES - 1, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void printTimeSerial(char *c) {
  for(u_int16_t i=0;i < 6; i++) {
    Serial.print(c[i]);
  }
  Serial.println();
}

void printTime(char newTime[7])
{
  //printTimeSerial(newTime);
  //fillBuffer(newTime);
  printTimeSerial(newTime);
  // for (int i = 0; i < 8; i++)
  // {
  // printBuffer();
  // }
  // 
  //strcpy(lastTime, time);
}

char *getTime()
{
  DateTime now = rtc.now();
  char f[] = "hhmmss";
  return now.toString(f);
}


bool displayEnabled()
{
  return true;
  DateTime now = rtc.now();
  if (now.hour() >= CLOCK_ON && now.hour() < CLOCK_OFF)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void setTime()
{
  #ifdef WIFI
  if (timeClient.getEpochTime() - lastRefresh > NTP_UPDATE_INTERVAL)
  {
    Serial.println("Set Time with NTP");
    while (!timeClient.isTimeSet())
    {
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
  #endif
}

// void showZeroTime()
// {
//   char zero[9];
//   sprintf(zero, "00:00:00");
//   printTime(zero);
// }

void setup()
{

  Wire.begin(SDA, SCL);
  Serial.begin(9600);
  Serial.println("Setup Routine");

  #ifdef WIFI

  Serial.println("Connect to Wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  timeClient.begin();

  #endif

  Serial.println("Setup RTC");
  while (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    delay(500);
  }

  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, LED_INTENSITY);

  //strcpy(lastTime, "000000");

  // showZeroTime();

  setTime();
}

void loop()
{
  setTime();
  if (displayEnabled())
  {
    printTime(getTime());
  }
  else
  {
    mx.clear();
  }
  delay(1000);
}