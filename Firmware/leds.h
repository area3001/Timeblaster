#ifndef LEDS_H
#define LEDS_H

#include <Arduino.h>
//#include <Adafruit_NeoPixel.h>

class _leds {
  private:
    _leds();
    uint32_t color;
  public:
    static _leds &getInstance();
    void init();
    void setPixelColor(byte pixel, uint32_t color);
    void setDiskColor(byte disk, uint32_t color);
    void setVerticalColor(byte index, uint32_t color);
    void setHorizontalColor(byte index, uint32_t color);
    void update();
    static uint32_t wheel(byte index);
    void clear();
};

extern _leds &Leds;

/*


void setTeamLED(byte team)
{
  if (team == 0)
    strip.setPixelColor(0, strip.Color(0, 0, 0));
  else if (team == 1)
    strip.setPixelColor(0, strip.Color(0, 255, 0));
  else if (team == 2)
    strip.setPixelColor(0, strip.Color(255, 0, 0));
  else if (team == 3)
    strip.setPixelColor(0, strip.Color(255, 255, 0));
  else if (team == 4)
    strip.setPixelColor(0, strip.Color(0, 0, 255));
  else if (team == 5)
    strip.setPixelColor(0, strip.Color(0, 255, 255));
  else if (team == 6)
    strip.setPixelColor(0, strip.Color(255, 0, 255));
  else
    strip.setPixelColor(0, strip.Color(255, 255, 255));
  strip.show();
  delay(1);
}

void setLed(byte led, uint32_t color){
  strip.setPixelColor(led, color);
  }

void setLed(byte led, uint8_t r, uint8_t g, uint8_t b)
{
  // swap R & G, LED0 is a different kind
  if (led == 0)
  {
    strip.setPixelColor(led, strip.Color(g, r, b));
    return;
  }
  strip.setPixelColor(led, strip.Color(r, g, b));
}

void setRing(byte ring, uint8_t r, uint8_t g, uint8_t b)
{
  byte min = 0;
  byte max = 0;

  switch (ring)
  {
  case 1:
    min = 1;
    max = 5;
    break;
  case 2:
    min = 5;
    max = 7;
    break;
  case 3:
    min = 7;
    max = 9;
  }

  for (int i = min; i < max; i++)
  {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }

  strip.show();
}

void clearAll()
{
  for (int i = 0; i < 9; i++)
    setLed(i, 0, 0, 0);
  strip.show();
}*/

#endif
