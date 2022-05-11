#ifndef Pins_h
#define Pins_h

#include <Arduino.h>

// https://camo.githubusercontent.com/c55beef2f138da61fe671a1e4a307ff4ffbc318d/68747470733a2f2f692e696d6775722e636f6d2f715849456368542e6a7067
#if HARDWARE_VERSION == 0
/* Original blaster version with USB B type port
   Sound is disabled on this board because the buzzer pin can't do hardware PWM.
   You can piggyback on the IR carrierwave ISR but that produces a limited range and has distortions when transmitting data
*/
#define RXD_PIN 0 // Marked with T> on the blaster
#define TXD_PIN 1 // Marked with R> on the blaster
#define R_TEAM_PIN 2
#define G_TEAM_PIN 3
#define B_TEAM_PIN 4
#define IR_IN1_PIN 5
#define IR_IN2_PIN 6
#define BADGELINK_PIN 7
#define TRIGGER_PIN 8
#define IR_OUT_PIN 9
#define WS2812_PIN 10
#define IR_OUT2_PIN 11

// Sound uses pin 10 on the final blaster release. This pin is used by the RGB led's on this version.
#elif HARDWARE_VERSION == 1
/* Some pins have the incorrect label on the board */
#define RXD_PIN 0 // Marked with T> on the blaster
#define TXD_PIN 1 // Marked with R> on the blaster
#define R_TEAM_PIN A5
#define G_TEAM_PIN 4
#define B_TEAM_PIN 8
#define IR_IN1_PIN 5
#define IR_IN2_PIN 6
#define BADGELINK_PIN 7
#define TRIGGER_PIN 3
#define IR_OUT_PIN 9
#define WS2812_PIN 2
#define IR_OUT2_PIN 11
#elif HARDWARE_VERSION == 2
/* Friedcamp 2022 version. use this */
#define RXD_PIN 0 // Marked with T> on the blaster
#define TXD_PIN 1 // Marked with R> on the blaster
#define R_TEAM_PIN A5
#define G_TEAM_PIN 4
#define B_TEAM_PIN 8
#define IR_IN1_PIN 5  //PD PCINT21
#define IR_IN2_PIN 6  //PD PCINT22
#define BADGELINK_PIN 7
#define TRIGGER_PIN 3
#define IR_OUT_PIN 9
#define WS2812_PIN 2
#define IR_OUT2_PIN 11
#else
#error Invalid HARDWARE_VERSION
#endif

typedef enum TeamName
{
  eNoTeam = 0,
  eTeamRex = 1,
  eTeamGiggle = 2,
  eTeamBuzz = 4,
  eTeamMagenta = 5,
  eTeamYellow = 3,
  eTeamAzure = 6,
  eTeamWhite = 7
};

TeamName blasterTeam = eNoTeam;
TeamName badgeTeam = eNoTeam;
bool teamLocked = false;

TeamName getHardwareTeam()
{
  pinMode(R_TEAM_PIN, INPUT_PULLUP);
  pinMode(G_TEAM_PIN, INPUT_PULLUP);
  pinMode(B_TEAM_PIN, INPUT_PULLUP);

  byte r = (1 - digitalRead(R_TEAM_PIN)) * 1;
  byte g = (1 - digitalRead(G_TEAM_PIN)) * 2;
  byte b = (1 - digitalRead(B_TEAM_PIN)) * 4;

  return r + g + b;
}

bool teamChanged()
{
  if (teamLocked)
  {
    if (badgeTeam != blasterTeam)
    {
      blasterTeam = badgeTeam;
      return true;
    }
    return false;
  }
  else
  {
    TeamName hardwareTeam = getHardwareTeam();
    if (hardwareTeam != 0 && blasterTeam != hardwareTeam)
    {
      blasterTeam = hardwareTeam;
      return true;
    }
    return false;
  }
}

void lockTeam(TeamName newTeam)
{
  badgeTeam = newTeam;
  teamLocked = true;
}

void unlockTeam()
{
  teamLocked = false;
}

bool triggerLocked = false;

bool triggerPressed()
{
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  return !triggerLocked && !digitalRead(TRIGGER_PIN);
}

#endif
