/*
  Firmware for the Fri3d Camp Timeblaster 2020 Rev. 00
*/
#include <Adafruit_NeoPixel.h> //https://github.com/adafruit/Adafruit_NeoPixel
#include "Constants.h"
Adafruit_NeoPixel strip(WS2812_Leds, WS2812_Pin, NEO_RGB + NEO_KHZ800);
#include "Ir.h"
#include "Trigger.h"

/*
  1-wire memory
  3: MAC
  3: shot by
  1: [enable blaster, ......]
*/

#define MUTE

void setup()
{
  pinMode(TeamSelect_R_Pin, INPUT_PULLUP);
  pinMode(TeamSelect_G_Pin, INPUT_PULLUP);
  pinMode(TeamSelect_B_Pin, INPUT_PULLUP);
  pinMode(Buzzer_Pin, OUTPUT);

  strip.begin();

  powerOnTest();

  Serial.begin(250000);
  EnableTrigger();
  SetupIr();
  EnableIr();
}

void loop()
{
  byte selectedTeam;
  selectedTeam = readTeamSelector();
  setTeamLED(selectedTeam);
  //strip.show();
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
}

byte readTeamSelector()
{
  if (!digitalRead(TeamSelect_R_Pin)) return Rex;
  if (!digitalRead(TeamSelect_G_Pin)) return Giggle;
  return Buzz;
}

void powerOnTest() {
  uint32_t colors[] = {
    strip.Color(128, 0, 0),
    strip.Color(0, 128, 0),
    strip.Color(0, 0, 128),
    strip.Color(128, 128, 0),
    strip.Color(0, 128, 128),
    strip.Color(128, 0, 128),
  };


  strip.begin();
  for (uint8_t offset = 0; offset < 12; offset++)
    for (uint8_t pixel = 0; pixel < 9; pixel++)
    {
#ifndef MUTE
      if (offset == 0) {
        if (pixel == 0) tone(Buzzer_Pin, 400);
        if (pixel == 2) tone(Buzzer_Pin, 600);
        if (pixel == 4) tone(Buzzer_Pin, 1000);
        if (pixel == 6) noTone(Buzzer_Pin);
      }
#endif
      strip.setPixelColor(pixel, colors[(pixel + offset) % 6]);
      strip.show();
      delay(50);
    }
  for (uint8_t pixel = 9; pixel > 0; pixel--)
  {
    strip.setPixelColor(pixel - 1, strip.Color(0, 0, 0));
  }
  strip.show();
}
