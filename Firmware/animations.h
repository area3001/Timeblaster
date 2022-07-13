#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include "buzzer.h"
#include "leds.h"

namespace Animations
{
  enum AnimationNames: uint8_t {
    eAnimationBlasterStart = 1,
    eAnimationError = 2,
    eAnimationCrash = 3,
    eAnimationFireball = 4,
    eAnimationOneUp = 5,
    eAnimationCoin = 6,
    eAnimationVoice = 7,
    eAnimationWolfWhistle = 8,
    eAnimationChatter = 9,
    
    eAnimationBlinkTeamLed = 15,
  };

  uint32_t _teamToColor(uint8_t team, uint8_t level)
  {
    uint32_t color = 0;
    uint32_t rgb = level;
    // todo: allow for other colors
    if (team & 0b001)
      color |= (rgb << 16);
    if (team & 0b010)
      color |= ((rgb / 2) << 8);
    if (team & 0b100)
      color |= (rgb << 0);
    return color;
  }

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
    for (byte w = 0; w < 200; w += 10)
    {
      for (int i = 0; i < 9; i++)
      {
        Leds.setPixelColor(i, Leds.wheel(w + (i * 10)));
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

  void error()
  {
    for (int i = 0; i < 10; i++)
    {
      Buzzer.playFrequency(200);
      delay(25);
      Buzzer.playFrequency(0);
      delay(25);
    }

    Buzzer.playFrequency(0);
  }

  void crash(uint8_t team)
  {
    Leds.clear();

    uint32_t color = 0;
    // todo: allow for other colors
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

    for (int i = 1; i < 5; i++)
    {
      delay(1000);
      Leds.setPixelColor(i, 0);
      Leds.update();
    }
    delay(1000);
  }

  void voice()
  {
    // https://github.com/connornishijima/arduino-volume3/blob/master/examples/test_sounds/test_sounds.ino
    int beeps[] = {1933, 2156, 1863, 1505, 1816, 1933, 1729, 2291};

    int i = 9;
    while (i >= 0)
    {
      Buzzer.playFrequency(1050);
      delayMicroseconds(20 * 64);
      Buzzer.playFrequency(1050);
      delayMicroseconds(40 * 64);
      i--;
    }
    // Buzzer.playFrequency(0);

    delay(35);

    i = 0;
    while (i < 8)
    {
      int v = 0;
      while (v < 250)
      { // 12.5 mS fade up time
        Buzzer.playFrequency(beeps[i]);
        v += 10;
        delayMicroseconds(2 * 64);
      }
      delay(20);
      v = 250;
      while (v > 0)
      { // 12.5 mS fade down time
        Buzzer.playFrequency(beeps[i]);
        v -= 10;
        delayMicroseconds(5 * 64);
      }
      Buzzer.playFrequency(0);
      delay(25);
      i++;
    }

    int f = 2466;
    while (f < 2825)
    {
      Buzzer.playFrequency(f);
      f += 3;
      delay(1);
    }
    f = 2825;
    int v = 255;
    while (f > 2000)
    {
      Buzzer.playFrequency(f);
      f -= 6;
      v -= 1;
      delay(1);
    }
    Buzzer.playFrequency(0);
    delay(35);

    i = 10;
    while (i > 0)
    {
      Buzzer.playFrequency(1050);
      delayMicroseconds(20 * 64);
      Buzzer.playFrequency(1050);
      delayMicroseconds(40 * 64);
      i--;
    }
    Buzzer.playFrequency(0);
  }

  void fireball()
  {
    Buzzer.playNote(G, 2, 35);
    Buzzer.playNote(G, 3, 35);
    Buzzer.playNote(G, 4, 35);
    Buzzer.playFrequency(0);
  }

  void one_up()
  {
    Buzzer.playNote(E, 3, 130);
    Buzzer.playNote(G, 3, 130);
    Buzzer.playNote(E, 4, 130);
    Buzzer.playNote(C, 4, 130);
    Buzzer.playNote(D, 4, 130);
    Buzzer.playNote(G, 4, 125);
    Buzzer.playFrequency(0);
  }

  void coin()
  {
    Buzzer.playNote(B, 3, 100);
    Buzzer.playNote(E, 4, 600);
    Buzzer.playFrequency(0);
    ;
  }

  void phrase1()
  {
    Leds.setDiskColor(1, 0x00FF0000);
    int k = random(1000, 2000);
    for (int i = 0; i <= random(100, 2000); i++)
    {

      Buzzer.playFrequency(k + (-i * 2));
      delay(random(.9, 2));
    }
    Leds.setDiskColor(2, 0x00000000);
    Leds.setDiskColor(1, 0x00FF0000);
    for (int i = 0; i <= random(100, 1000); i++)
    {

      Buzzer.playFrequency(k + (i * 10));
      delay(random(.9, 2));
    }
    Leds.setDiskColor(2, 0x00000000);
  }

  void phrase2()
  {
    Leds.setDiskColor(1, 0x0000FF00);
    int k = random(1000, 2000);
    for (int i = 0; i <= random(100, 2000); i++)
    {

      Buzzer.playFrequency(k + (i * 2));
      delay(random(.9, 2));
    }
    Leds.setDiskColor(1, 0x00000000);
    Leds.setDiskColor(1, 0x0000FF00);
    for (int i = 0; i <= random(100, 1000); i++)
    {

      Buzzer.playFrequency(k + (-i * 10));
      delay(random(.9, 2));
    }
    Leds.setDiskColor(2, 0x00000000);
  }

  void chatter()
  {
    int K = 2000;
    switch (random(1, 7))
    {

    case 1:
      phrase1();
      break;
    case 2:
      phrase2();
      break;
    case 3:
      phrase1();
      phrase2();
      break;
    case 4:
      phrase1();
      phrase2();
      phrase1();
      break;
    case 5:
      phrase1();
      phrase2();
      phrase1();
      phrase2();
      phrase1();
      break;
    case 6:
      phrase2();
      phrase1();
      phrase2();
      break;
    }
    Leds.setDiskColor(2, 0x000000FF);
    for (int i = 0; i <= random(3, 9); i++)
    {

      Buzzer.playFrequency(K + random(-1700, 2000));
      delay(random(70, 170));
      Buzzer.playFrequency(0);
      delay(random(0, 30));
    }
    Leds.setDiskColor(2, 0x00000000);
    Buzzer.playFrequency(0);
  }

  void wolfWhistle()
  {
    // https://github.com/connornishijima/arduino-volume3/blob/master/examples/test_sounds/test_sounds.ino
    int f = 122; // starting frequency
    while (f < 4000)
    {
      Buzzer.playFrequency(f);
      f += 25;
      delay(1);
    }
    Buzzer.playFrequency(0);
    delay(100); // wait a moment
    f = 122;    // starting frequency
    while (f < 3000)
    { // slide up to 3000Hz
      Buzzer.playFrequency(f);
      f += 25;
      delay(2);
    }
    while (f > 125)
    { // slide down to 125Hz
      Buzzer.playFrequency(f);
      f -= 25;
      delay(2);
    }
    Buzzer.playFrequency(0); // end tone production
  }

  /* END of Animations */

  void set_team_status(uint8_t team, bool can_shoot)
  {
    Leds.setPixelColor(0, _teamToColor(team, 255));
    if (can_shoot)
      Leds.setDiskColor(1, _teamToColor(team, 25));
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
    delay(100); // helps debouncing
  };

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

  void refresh()
  {
    Leds.update();
  }

  void stealth(bool stealth_mode)
  {
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
    if (r == 0)
      Leds.setPixelColor(0, _teamToColor(base_team, 255));
    else if (r < 4)
      Leds.setPixelColor(0, 0);
    else
      Leds.setPixelColor(0, _teamToColor(team, 255));
    Leds.update();
    delay(random(50));
  }

}

#endif
