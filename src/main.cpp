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

#define RED 0   // WIFI
#define BLUE 16 // CLOCK
#define GREEN 2 // NTP

#define BUTTON_1 10
#define BUTTON_2 9

// #define MASK 0b00111110
#define MASK 0b11111111

#define SDA 4
#define SCL 5

#define WIFI

#define CLOCK_ON 8
#define CLOCK_OFF 23
#define ALWAYS_ON 0

#define NTP_UPDATE_INTERVAL 3600

#define TIME_OFFSET_SUMMER 7200
#define TIME_OFFSET_WINTER 3600

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

u_int32_t lastRefresh = 0;

#ifdef WIFI
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", TIME_OFFSET_WINTER, 60000);
#endif

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
RTC_DS3231 rtc;

bool always_light_on = false;
byte button_1_last_state = 1;

void printTimeSerial(const char *msg, byte *time)
{
  Serial.print(msg);
  Serial.print(": ");
  for (byte i = 0; i < 6; i++)
  {
    Serial.printf("%d", time[i]);
  }
  Serial.println();
}

void fillBuffer(word buffer[], byte newTime[], byte lastTime[])
{
  byte buf = 0;
  for (byte i = 0; i < 6; i++)
  {
    for (byte j = 0; j < 3; j++)
    {
      buffer[buf] = (numbers[lastTime[i]][j] << 8) + numbers[newTime[i]][j];
      buf++;
    }
  }
}

void shiftBuffer(word buffer[])
{
  for (byte i = 0; i < 18; i++)
  {
    buffer[i] = buffer[i] << 1;
  }
}

void printBuffer(word buffer[], byte newTime[], byte lastTime[], bool force)
{
  byte digit = 0;
  byte col = MAX_COL;
  mx.control(0, MAX_DEVICES - 1, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (byte i = 0; i < 18; i++)
  {
    if (i % 3 == 0 && i > 0)
    {
      col--;
      digit++;
    }

    if (i % 6 == 0 && i > 0)
    {
      mx.setColumn(col, points);
      col -= 2;
    }
    if (newTime[digit] != lastTime[digit] || force)
    {
      mx.setColumn(col, (buffer[i] >> 8) & MASK);
    }
    col--;
  }
  mx.control(0, MAX_DEVICES - 1, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

bool timeChanged(byte newTime[], byte lastTime[])
{
  for (byte i = 0; i < 6; i++)
  {
    if (newTime[i] != lastTime[i])
      return true;
  }
  return false;
}

bool displayEnabled()
{
  if (ALWAYS_ON)
    return true;
  DateTime now = rtc.now();
  if (now.hour() == CLOCK_ON && always_light_on) {
    always_light_on = false; // reset setting in the morning
  }
  if ((now.hour() >= CLOCK_ON && now.hour() < CLOCK_OFF) || always_light_on)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void printTime(word buffer[], byte newTime[], byte lastTime[], bool force)
{
  if (!timeChanged(newTime, lastTime) && !force)
    return;
  if (displayEnabled())
  {
    for (byte i = 0; i < 9; i++)
    {
      printBuffer(buffer, newTime, lastTime, force);
      shiftBuffer(buffer);
      delay(50);
    }
  }
  else
  {
    mx.clear();
  }
}

void getTime(byte newTime[])
{
  DateTime now = rtc.now();
  newTime[0] = floor(now.hour() / 10);
  newTime[1] = now.hour() - (newTime[0] * 10);
  newTime[2] = floor(now.minute() / 10);
  newTime[3] = now.minute() - (newTime[2] * 10);
  newTime[4] = floor(now.second() / 10);
  newTime[5] = now.second() - (newTime[4] * 10);
}

void setTime()
{
#ifdef WIFI
  if (timeClient.getEpochTime() - lastRefresh > NTP_UPDATE_INTERVAL)
  {
    while (!timeClient.isTimeSet())
    {
      digitalWrite(GREEN, LOW);
      Serial.println("Set Time with NTP");
      timeClient.update();
      delay(500);
      digitalWrite(GREEN, HIGH);
      delay(500);
    }
    Serial.print("Set RTC with epochtime: ");
    Serial.println(timeClient.getEpochTime());
    rtc.adjust(DateTime(timeClient.getEpochTime()));
    lastRefresh = timeClient.getEpochTime();
  }
#endif
}

void setup()
{

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);

  digitalWrite(RED, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(BLUE, LOW);

  Wire.begin(SDA, SCL);
  Serial.begin(9600);
  Serial.println("Setup Routine");

#ifdef WIFI

  Serial.println("Connect to Wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(BLUE, LOW);
    delay(250);
    Serial.print(".");
    digitalWrite(BLUE, HIGH);
    delay(250);
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

  mx.clear();

  setTime();
}

bool read_button(bool first) {
  byte button_1_state = digitalRead(BUTTON_1);
    if (button_1_state != button_1_last_state)
    {
      button_1_last_state = button_1_state;
      if (button_1_state)
      {
        if (always_light_on)
        {
          always_light_on = false;
        }
        else
        {
          always_light_on = true;
          first = true; //reset first to refresh display
        }
      }
    }
  return first;
}

void loop()
{
  static bool first = true;
  static byte lastTime[6] = {0, 0, 0, 0, 0, 0};
  static byte newTime[6] = {0, 0, 0, 0, 0, 0};
  static word buffer[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  while (true)
  {
    first = read_button(first);
    setTime();
    delay(10);
    getTime(newTime);
    fillBuffer(buffer, newTime, lastTime);
    printTime(buffer, newTime, lastTime, first);
    memcpy(lastTime, newTime, 6 * sizeof(byte));
    first = false;
  }
}