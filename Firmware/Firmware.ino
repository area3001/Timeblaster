/*
  Firmware for the Fri3d Camp Timeblaster 2020 Rev. 00
*/
#include <Adafruit_NeoPixel.h> //https://github.com/adafruit/Adafruit_NeoPixel
#include "Constants.h"

Adafruit_NeoPixel strip(WS2812_Leds, WS2812_Pin, NEO_RGB + NEO_KHZ800);

void setup() 
{
  pinMode(TeamSelect_R_Pin, INPUT_PULLUP);
  pinMode(TeamSelect_G_Pin, INPUT_PULLUP);
  pinMode(TeamSelect_B_Pin, INPUT_PULLUP);

  strip.begin();
  strip.setPixelColor(0, strip.Color(128, 128, 128));
  strip.show();
}

void loop() {
  byte selectedTeam;
  selectedTeam = readTeamSelector();
  setTeamLED(selectedTeam);
}

void setTeamLED(byte team)
{
  uint32_t teamColor;
  switch (team) {
    case Rex:
      teamColor = RedRGB;
      break;
    case Giggle:
      teamColor = GreenRGB;
      break;
    case Buzz:
      teamColor = BlueRGB;
      break;
  }
  strip.setPixelColor(0, teamColor);
  strip.show();
}

byte readTeamSelector()
{
  if (!digitalRead(TeamSelect_R_Pin)) return Rex;
  if (!digitalRead(TeamSelect_G_Pin)) return Giggle;
  return Buzz;
}
