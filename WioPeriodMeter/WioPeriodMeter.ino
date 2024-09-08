/* WioPeriodMeter.ino
// sketch to display the period of pulses on D1 (pin 15) of the Seeed Wio Terminal
// to show line time of the Nikon A1 and AX controllers on pin 2 of the SYNC connector
// author: Kees van der Oord <instruments-support.eu@nikon.com>
// URL: https://github.com/Kees-van-der-Oord/WioPeriodMeter
*/

// version: 1.0, 11-8-2024, creation
// version: 2.0, 8-9-2024 add modes: period, low pulse width, high pulse width; change mode with left top button; fixed wrong display periods > 10s

#include <Seeed_FS.h>
#include <FlashAsEEPROM.h>
#include "string.h"
#include "TFT_eSPI.h"
#include "free_fonts.h"

constexpr int16_t screen_width = 320;
constexpr int16_t screen_height = 240;
TFT_eSPI tft;

enum modes { MODE_PERIOD, MODE_HIGH, MODE_LOW, MODE_LAST };
unsigned char mode = MODE_LOW;

#define PIN_IN D1
volatile uint32_t ticks = 0;
uint32_t last_tick = 0;
long last_state = 0;
volatile float period = 0.0;
#define AVERAGE 20.

void onPinInInterruptPeriod()
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

void onPinInInterruptLow()
{
  ++ticks;
  int new_state = digitalRead(PIN_IN);
  uint32_t now = micros();
  if((last_tick != 0) || (period != 0))
  {
    if(!last_state)
    {
      float elapsed = now - last_tick;
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
  }
  last_tick = now;
  last_state = new_state;
}

void onPinInInterruptHigh()
{
  ++ticks;
  int new_state = digitalRead(PIN_IN);
  uint32_t now = micros();
  if((last_tick != 0) || (period != 0))
  {
    if(last_state)
    {
      float elapsed = now - last_tick;
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
  }
  last_tick = now;
  last_state = new_state;
}

void initialize()
{
  tft.setFreeFont(NULL);
  tft.setTextSize(0);
  
  detachInterrupt(digitalPinToInterrupt(PIN_IN));  
  switch(mode)
  {
  case MODE_PERIOD:
    attachInterrupt(digitalPinToInterrupt(PIN_IN), onPinInInterruptPeriod, FALLING);
    break;
  case MODE_HIGH:
    attachInterrupt(digitalPinToInterrupt(PIN_IN), onPinInInterruptHigh, CHANGE);
    break;
  case MODE_LOW:
    attachInterrupt(digitalPinToInterrupt(PIN_IN), onPinInInterruptLow, CHANGE);
    break;
  }

  tft.fillScreen(TFT_BLACK);
  tft.drawString("mode",10,5,2);
  tft.drawString("github.com/Kees-van-der-Oord/WioPeriodMeter",10,30,2);
  switch(mode)
  {
    case MODE_PERIOD: 
      tft.drawString("Wio Period Meter",50,65,4);
      tft.drawString("period on D1 (pin 15)",80,100,2);
      break;
    case MODE_HIGH: 
      tft.drawString("Wio High Pulse Width Meter",5,65,4); 
      tft.drawString("high pulse width on D1 (pin 15)",60,100,2);
      break;
    case MODE_LOW: 
      tft.drawString("Wio Low Pulse Width Meter",10,65,4); 
      tft.drawString("low pulse width on D1 (pin 15)",60,100,2);
      break;
  }
  tft.drawString("D0 D1 3V3 GND",210,200,2);
  tft.drawString("13 15",209,220,2);
  tft.drawString("BAUDRATE 115200",10,200,2);
  tft.setFreeFont(FMB24);
}

#define WioPeriodMeterSignature 0x42
void eeprom_init()
{
  byte signature = EEPROM.read(0);
  if(signature != WioPeriodMeterSignature)
  {
    EEPROM.write(0, WioPeriodMeterSignature);
    EEPROM.write(1, mode);
    EEPROM.commit();
  }
  else
  {
    mode = EEPROM.read(1) % MODE_LAST;
  }
}

void setup() 
{
  Serial.begin(115200);
    
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  pinMode(PIN_IN,INPUT);

  tft.init();
  tft.setRotation(3);

  eeprom_init();

  initialize();
}

#define BUTTON_PRESSED 0

void handle_buttons()
{
  static int      last_state = !BUTTON_PRESSED;
  static uint32_t press_tick = 0;
  int state = digitalRead(WIO_KEY_C);
  if(state == BUTTON_PRESSED)
  {
    uint32_t now = millis();
    if(last_state == !BUTTON_PRESSED)
    {
      press_tick = now;
    }
    else if((now - press_tick) > 500)
    {
      mode = (mode + 1) % MODE_LAST;
      EEPROM.write(1, mode);
      EEPROM.commit();
      initialize();
      state = !BUTTON_PRESSED;
    }
  }
  last_state = state;
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
  handle_buttons();

  uint32_t elapsed = millis() - last_update;
  if(elapsed > 200)
  {
    last_update = millis();

    // format the period
    char buf[64];
    float p = period;
    // ! the sprintf function does not round float numbers nor pad with spaces !
         if(p < 1000)       { p = uint32_t(p         +0.5)/1000.; sprintf(buf,"%.3f ms ",p); }
    else if(p < 10000)      { p = uint32_t(p/     10.+0.5)/100.;  sprintf(buf,"%.2f ms ",p);}
    else if(p < 100000)     { p = uint32_t(p/    100.+0.5)/10.;   sprintf(buf,"%.1f ms ",p);}
    else if(p < 1000000)    { p = uint32_t(p/   1000.+0.5)/1.;    sprintf(buf,"%.0f ms ",p);}
    else if(p < 10000000)   { p = uint32_t(p/  10000.+0.5)/100.;  sprintf(buf,"%.2f s  ",p);}
    else if(p < 100000000)  { p = uint32_t(p/ 100000.+0.5)/10.;   sprintf(buf,"%.1f s  ",p);}
    else                    { p = uint32_t(p/1000000.+0.5)/1.;    sprintf(buf,"%.0f s  ",p);}
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
      tft.drawString(buf,0,135);
    }

    if(ticks != last_ticks)
    {
      last_ticks = ticks;
      ++ifan;
      if(ifan > 3) ifan = 0;
      sprintf(buf,"%c ",fan[ifan]);
      tft.drawString(buf,150,200);
    }
  }
}
