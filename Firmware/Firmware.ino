#include "functions.h"
#include "animations.h"
#include "data.h"

#define R_TEAM_PIN A5
#define G_TEAM_PIN 4
#define B_TEAM_PIN 8
#define TRIGGER_PIN 3

enum GameMode: uint8_t{
  eGMTimeout = 0,
  eGMZombie = 1,
  eGMSuddenDeath = 2
};

/* Blaster state variables */
// if false the trigger is disabled. Set by the badge (trigger field)
bool can_shoot = true;
uint8_t ir_channel = 0b0000;   //default channel
bool healing_mode = false;
bool stealth_mode = false;
bool single_shot_mode = false;
uint8_t game_mode = eGMZombie; //eGMTimeout

uint8_t badge_team = 0;   //the team set by the badge (over rules the blaster if != 0)
uint8_t blaster_team = 0; //the team set on the blaster
uint8_t zombie_team = 0;  //zombie team overrules all other team settings but can be reset to 0 if the badge sets or resets the team. todo

bool muted = false;
bool animated = true;
uint8_t brightness = 0b11;

void setup()
{
  setupButtons();
  Serial.begin(115200);
  Data.init();
  blinkIfNoTeamSelector(); 
  Animations::setup();

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
  blasterReady();
}

void loop()
{
  if (teamChanged())
  {
    Animations::team_switch(blaster_team);
  }

  if (triggerPressed())
  {
    damageShot();
    Animations::shoot(blaster_team);
  }

  auto badge_packet = Data.readBadge();
  if (badge_packet.command == eCommandSetChannel){
    ir_channel = badge_packet.parameter;
    blasterReady();
  }

  auto ir_packet = Data.readIr();

  if (ir_packet.command == eCommandShoot && 
        ir_packet.parameter == ir_channel && 
        ir_packet.trigger_state == 1 &&
        ir_packet.team != activeTeam()){
          handle_being_shot(ir_packet);
        }

  //bug: changing teams after healer fixed blaster causes new infection; disable team change
  //bug: should not be able to change team when in zombie mode
  if (game_mode == eGMZombie && activeTeam(true) != activeTeam(false)){
    Animations::FlickerTeam(activeTeam(), activeTeam(true));
  }
  //Animations::refresh();
}


uint8_t activeTeam(bool ignore_zombie_team)
{
  if (zombie_team && ! ignore_zombie_team) return zombie_team;
  if (badge_team) return badge_team;
  return blaster_team; //todo replace by switch readout
}
uint8_t activeTeam() {return activeTeam(false);}


void handle_being_shot(DataPacket packet){
  //send data to badge
  packet.trigger_state = 0; // we are not fireing
  Data.transmit(packet, eBadge);

  switch (game_mode)
  {
  case eGMTimeout:
    // action here depends on game mode
    Animations::crash(packet.team);
    Data.readIr(); //clear buffer
    Data.readBadge(); //clear badge (in case the badge received the same packet) //todo: improve so that we only remove shoot commands
    Animations::clear();
    Animations::team_switch(activeTeam());
    break;
  case eGMZombie:
    // action here depends on game mode
    Animations::crash(packet.team);
    Data.readIr(); //clear buffer
    Data.readBadge(); //clear badge (in case the badge received the same packet) //todo: improve so that we only remove shoot commands
    Animations::clear();
    zombie_team = packet.team;
    if (packet.team == activeTeam(true)) zombie_team = 0;
    Animations::team_switch(activeTeam());
    break;  
  default:
    break;
  }
}

void healingShot()
{
  DataPacket d;
  d.team = activeTeam();
  d.trigger_state = 1;
  d.command = eCommandHeal;
  d.parameter = ir_channel;

  Data.transmit(d, eAllDevices);

}

void damageShot()
{
  DataPacket d;
  d.team = activeTeam();
  d.trigger_state = 1;
  d.command = eCommandShoot;
  d.parameter = ir_channel;

  Data.transmit(d, eAllDevices);
}

void setupButtons()
{
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(R_TEAM_PIN, INPUT_PULLUP);
  pinMode(G_TEAM_PIN, INPUT_PULLUP);
  pinMode(B_TEAM_PIN, INPUT_PULLUP);
}

void blasterReady()
{
  DataPacket d;
  d.team = getHardwareTeam();
  d.trigger_state = 0;
  d.command = eCommandBlasterAck;
  d.parameter = 0;
  Data.transmit(d, eBadge);
}

bool triggerPressed()
{
  return can_shoot && !digitalRead(TRIGGER_PIN);
}

bool teamChanged()
{
  if (badge_team)
  {
    if (badge_team != blaster_team)
    {
      blaster_team = badge_team;
      return true;
    }
    return false;
  }
  else
  {
    uint8_t hardwareTeam = getHardwareTeam();
    if (hardwareTeam != 0 && blaster_team != hardwareTeam)
    {
      blaster_team = hardwareTeam;
      return true;
    }
    return false;
  }
}

uint8_t getHardwareTeam()
{
  byte r = (1 - digitalRead(R_TEAM_PIN)) * 1;
  byte g = (1 - digitalRead(G_TEAM_PIN)) * 2;
  byte b = (1 - digitalRead(B_TEAM_PIN)) * 4;

  return r + g + b;
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
  if (digitalRead(R_TEAM_PIN) && digitalRead(G_TEAM_PIN) && digitalRead(B_TEAM_PIN))
  {
    Serial.println("Enter Flash Test Mode");
    while (digitalRead(R_TEAM_PIN) && digitalRead(G_TEAM_PIN) && digitalRead(B_TEAM_PIN))
    {
      digitalWrite(A0, HIGH);
      delay(100);
      digitalWrite(A0, LOW);
      delay(500);
    }
    Serial.println("Exit Flash Test Mode");
  }
  pinMode(A0, INPUT);
}
