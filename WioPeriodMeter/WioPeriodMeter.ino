/* WioPeriodMeter.ino
// sketch to display the period of pulses on D1 (pin 15) of the Seeed Wio Terminal
// to show line time of the Nikon A1 and AX controllers on pin 2 of the SYNC connector
// author: Kees van der Oord <instruments-support.eu@nikon.com>
// URL: https://github.com/Kees-van-der-Oord/WioPeriodMeter
*/

// version: 1.0, 11-8-2024, creation

#include <Seeed_FS.h>
#include "string.h"
#include "TFT_eSPI.h"
#include "free_fonts.h"

constexpr int16_t screen_width = 320;
constexpr int16_t screen_height = 240;
TFT_eSPI tft;

#define PIN_IN D1
volatile uint32_t ticks = 0;
uint32_t last_tick = 0;
volatile float period = 0.0;
#define AVERAGE 20.

void onPinInInterrupt()
{
  ++ticks;
  uint32_t now = micros();
  float elapsed;
  if((last_tick != 0) || (period != 0))
  {
    elapsed = now - last_tick;
    if(fabs(elapsed - period) > (period/40.))
    {
      // if the new period differs more than 2.5%, reset the averaging
      period = elapsed;
    }
    else
    {
      // rolling average
      period = period * (1.0-1.0/AVERAGE) + elapsed * (1.0/AVERAGE);
    }
  }
  last_tick = now;
}

/*
#include "Adafruit_ZeroTimer.h"

Adafruit_ZeroTimer zt3 = Adafruit_ZeroTimer(3);

void TC3_Handler()
{
    Adafruit_ZeroTimer::timerHandler(3);
}

void onTimerInterrupt()
{
  static uint32_t prev_ticks = 0;
  static bool ping_pong = 0;
  char buf[64];
  uint32_t count = ticks - prev_ticks;
  if(count < 1000)
  {
    sprintf(buf,"%5d Hz  ",count);
  }
  else
  {
    sprintf(buf,"%1d.%3d kHz",count/1000,count%1000);
  }
  tft.drawString(buf,30,110);
  prev_ticks += count;
}
*/

void setup() 
{
    Serial.begin(115200);
    
    pinMode(PIN_IN,INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_IN), onPinInInterrupt, FALLING);

    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    tft.drawString("github.com/Kees-van-der-Oord/WioPeriodMeter",10,10,2);
    tft.drawString("Wio Period Meter",50,45,4);
    tft.drawString("period on D1 (pin 15)",80,80,2);
    tft.drawString("D0 D1 3V3 GND",210,200,2);
    tft.drawString("13 15",209,220,2);
    tft.drawString("BAUDRATE 115200",20,200);
    tft.setFreeFont(FMB24);

/*
    zt3.configure(TC_CLOCK_PRESCALER_DIV1024, TC_COUNTER_SIZE_16BIT, TC_WAVE_GENERATION_MATCH_FREQ);
    zt3.setCompare(0, (48000000/1024)/1); // 1 Hz
    zt3.setCallback(true, TC_CALLBACK_CC_CHANNEL0, onTimerInterrupt);
    zt3.enable(true);
*/
}

uint32_t last_update = 0;
uint32_t last_ticks = 0;
uint32_t ifan = 0;
char fan[5] = "-\\|/";
char prev_buf[64] = "";

void loop() {

  if(Serial.available())
  {
    char c = Serial.read();
    Serial.println(period);
  }

  uint32_t elapsed = millis() - last_update;
  if(elapsed > 200)
  {
    last_update = millis();
    // format the period
    char buf[64];
    float p = period;
    // the sprintf function does not round float numbers !
         if(p < 1000)       { p = uint32_t(p         +0.5)/1000.;   sprintf(buf,"%.3f ms ",p); }
    else if(p < 10000)      { p = uint32_t(p/     10.+0.5)/100.;    sprintf(buf,"%.2f ms ",p);}
    else if(p < 100000)     { p = uint32_t(p/    100.+0.5)/10.;     sprintf(buf,"%.1f ms ",p);}
    else if(p < 1000000)    { p = uint32_t(p/   1000.+0.5)/1.;      sprintf(buf,"%.0f ms ",p);}
    else if(p < 10000000)   { p = uint32_t(p/  10000.+0.5)/1000.;   sprintf(buf,"%.2f s  ",p);}
    else if(p < 100000000)  { p = uint32_t(p/ 100000.+0.5)/10000.;  sprintf(buf,"%.1f s  ",p);}
    else                    { p = uint32_t(p/1000000.+0.5)/100000.; sprintf(buf,"%.0f s  ",p);}
    // to avoid flickering, only update the display when the string changed
    if(strcmp(buf,prev_buf))
    {
      strcpy(prev_buf,buf);
      // prefix with spaces
      int len = strlen(buf);
      int pad = 11 - len;
      if(pad > 0)
      {
        memmove(buf+pad,buf,len+1);
        memset(buf,' ',pad);
      }
      tft.drawString(buf,0,120);
    }

    if(ticks != last_ticks)
    {
      last_ticks = ticks;
      ++ifan;
      if(ifan > 3) ifan = 0;
      sprintf(buf,"%c ",fan[ifan]);
      tft.drawString(buf,150,180);
    }
  }
}
