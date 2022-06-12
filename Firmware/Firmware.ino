#define HARDWARE_VERSION 1 //todo: needs to go

#include "constants.h"
#include "pins.h"
#include "functions.h"
#include "badgelink.h"
#include "animations.h"
#include "data.h"

/* Blaster state variables */
bool can_shoot = true;
uint8_t ir_channel = 0b000;
bool healing_mode = false;
bool stealth_mode = false;
bool single_shot_mode = false;
uint8_t  game_mode = 0; //TDB
uint8_t fixed_team = 0;
bool muted = false;
bool animated = true;
uint8_t brightness = 0b11; 

void setup()
{  
  Serial.begin(115200);

  Data.init();

  DataPacket d;
  d.team = eTeamRex;
  d.trigger_state = 1;
  d.command = eCommandShoot;
  d.parameter = 0;

  Data.transmit(d, eBadge);

  Serial.println();
  while(true);

  blinkIfNoTeamSelector(); //This is only used when flashing the mc

//  Communications::init();
//  Communications::enableIrReceive();

  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  

  Serial.println("Blaster starting");
  Serial.println("Init timers");
 // setup_ir_carrier();

  Animations::setup();

  Animations::mute();
  if (triggerPressed())
  {
    Animations::mute();
  }
  while (triggerPressed())
  {
    Animations::blink_team_led();
  }
  Animations::blaster_start();



  delay(1000);
}

void loop()
{
  if (teamChanged())
  {
    Animations::team_switch(blasterTeam);
  }
  if (triggerPressed())
  {
    uint16_t team  = getHardwareTeam();
    //transmit_raw(team, true, true);
    Animations::shoot(blasterTeam);
  }

  Animations::refresh();

}

/* This is to give a visual signal that the firmware is flashed successfully when bulk uploading.
 * This has no use after flashing.
 */
void blinkIfNoTeamSelector()
{
  pinMode(R_TEAM_PIN, INPUT_PULLUP);
  pinMode(G_TEAM_PIN, INPUT_PULLUP);
  pinMode(B_TEAM_PIN, INPUT_PULLUP);
  pinMode(A0, OUTPUT);
  while (!(digitalRead(R_TEAM_PIN) | digitalRead(G_TEAM_PIN) | digitalRead(B_TEAM_PIN)))
  {
    digitalWrite(A0, HIGH);
    delay(500);
    digitalWrite(A0, LOW);
    delay(500);
  }
  pinMode(A0, INPUT_PULLUP);
}
