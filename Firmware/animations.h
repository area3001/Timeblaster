#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include "buzzer.h"
#include "leds.h"

namespace Animations
{

  void setup()
  {
    Leds.init();
  }

  void mute()
  {
    Buzzer.mute();
  }
  void unmute()
  {
    Buzzer.unmute();
  }

  /* Animations */
  void blink_team_led()
  {
    Leds.setPixelColor(0, 0xffffff);
    Leds.update();
    delay(200);
    Leds.setPixelColor(0, 0x000000);
    Leds.update();
    delay(200);
  }

  void blaster_start()
  {
    
    byte colors[] = {128, 0, 0, 128, 0};
    for (byte w = 0; w < 200; w += 10) {
      for (int i = 0; i < 9; i++)
      {
        Leds.setPixelColor(i, Leds.wheel(w+(i*10)));
      }
      Leds.update();
      delay(100);
    }
    
    /*0.100 0.100.0  */

    Buzzer.playFrequency(1025, 70);
    Buzzer.playFrequency(2090, 500);
    Buzzer.playFrequency(0);
    Leds.clear();
  }

  uint32_t teamToColor(uint8_t team, uint8_t level){
     uint32_t color = 0;
     uint32_t rgb = level;
      //todo: allow for other colors
      if (team & 0b001)
        color |= (rgb << 16);
      if (team & 0b010)
        color |= ((rgb/2) << 8);
      if (team & 0b100)
        color |= (rgb << 0);
      return color;
  }

  void set_team_status(uint8_t team, bool can_shoot){
      Leds.setPixelColor(0, teamToColor(team,255));
      if (can_shoot)
        Leds.setDiskColor(1, teamToColor(team,25));
      else
        Leds.setDiskColor(1, 0);
      Leds.update();
  }

  void team_switch(uint8_t team, bool can_shoot)
  {
    set_team_status(team, can_shoot);

    Buzzer.playNote(G, 4, 35);
    Buzzer.playNote(G, 5, 35);
    Buzzer.playNote(G, 6, 35);
    Buzzer.playFrequency(0);
    delay(100); //helps debouncing
  };

  void error(){
    for(int i=0;i<10;i++){
    Buzzer.playFrequency(200);
    delay(25);
    Buzzer.playFrequency(0);
    delay(25);
    }

    
    Buzzer.playFrequency(0);
  }

  void shoot(uint8_t team)
  {
    int mod = team * 2;
    uint32_t color;

    if (team == 0b001)
      color = 0x00FF0000;
    if (team == 0b010)
      color = 0x0000FF00;
    if (team == 0b100)
      color = 0x000000FF;

    for (int i = 0; i < 2 * mod; i++)
    {
      if (i % 3 == 0)
      {
        Leds.setDiskColor(1, color);
        Leds.setDiskColor(3, 0);
      }
      else if (i % 3 == 1)
      {
        Leds.setDiskColor(2, color);
        Leds.setDiskColor(1, 0);
      }
      else if (i % 3 == 2)
      {
        Leds.setDiskColor(3, color);
        Leds.setDiskColor(2, 0);
      }

      for (int f = 5000; f > 1000; f -= 10)
      {
        Buzzer.playFrequency(f);
        delayMicroseconds(200 / mod);
      }
    }

    for (int f = 8000 / mod; f > 100 * mod; f -= 10)
    {
      Buzzer.playFrequency(f);
      delayMicroseconds(500 * mod);
    }

    Buzzer.playFrequency(0);
    Leds.setDiskColor(1, 0);
    Leds.setDiskColor(2, 0);
    Leds.setDiskColor(3, 0);
    set_team_status(team, true);
  }

  void crash(uint8_t team)
  {
    Leds.clear();

    uint32_t color = 0;
    //todo: allow for other colors
    if (team & 0b001)
      color |= 0x00FF0000;
    if (team & 0b010)
      color |= 0x0000FF00;
    if (team & 0b100)
      color |= 0x000000FF;

    Leds.setDiskColor(1, color);

    for (int i = 1000; i > 0; i -= 20)
    {
      Buzzer.playFrequency(random(500 - i / 3, 8000 - i * 9));
      delayMicroseconds(4000);
    }
    for (int i = 0; i < 1000; i++)
    {
      Buzzer.playFrequency(random(500 - i / 3, 8000 - i * 9));
      delayMicroseconds(400);
    }
    Buzzer.playFrequency(0);

    for (int i = 1; i<5; i++){
      delay(1000);
      Leds.setPixelColor(i,0);
      Leds.update();
    }
    delay(1000);
  }

  void refresh()
  {
    Leds.update();
  }

  void stealth(bool stealth_mode){
     Leds.stealth(stealth_mode);
     refresh();
  }

  void clear()
  {
    Leds.clear();
  }

  void FlickerTeam(uint8_t team, uint8_t base_team)
  {
    auto r = random(10);
    if (r == 0) Leds.setPixelColor(0,teamToColor(base_team,255));
    else if (r < 4) Leds.setPixelColor(0,0);      
    else Leds.setPixelColor(0,teamToColor(team,255));
    Leds.update();
    delay(random(50));
  }



}

#endif
