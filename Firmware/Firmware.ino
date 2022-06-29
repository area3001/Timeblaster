#include "functions.h"
#include "animations.h"
#include "data.h"

#define R_TEAM_PIN A5
#define G_TEAM_PIN 4
#define B_TEAM_PIN 8
#define TRIGGER_PIN 3

/* Blaster state variables */
// if false the trigger is disabled. Set by the badge (trigger field)
bool can_shoot = true;
uint8_t ir_channel = 0b000;
bool healing_mode = false;
bool stealth_mode = false;
bool single_shot_mode = false;
uint8_t game_mode = 0; // TDB
uint8_t fixed_team = 0;
uint8_t blaster_team = 0;
bool muted = false;
bool animated = true;
uint8_t brightness = 0b11;

void setup()
{

  pinMode(A0, OUTPUT);
  
    while (true)
    {
      digitalWrite(A0, HIGH);
      delay(100);
      digitalWrite(A0, LOW);
      delay(100);
    }




  Serial.begin(115200);

  Data.init();

  blinkIfNoTeamSelector(); // This is only used when flashing the mc

  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  Serial.println("Blaster starting");
  Serial.println("Init timers");
  // setup_ir_carrier();

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
}

void loop()
{
    auto packet = Data.readBadge();
    if (packet.raw > 0) {
      Serial.print(packet.team);
      Serial.print(" ");
      Serial.print(packet.command);
      Serial.print(" ");
      Serial.println(packet.parameter);
      //Data.transmit(ir_packet, eAllDevices);
    }

    // DataPacket d;
    // d.team = eTeamRex;
    // d.trigger_state = 1;
    // d.command = eCommandShoot;
    // d.parameter = 0;

    // while (true)
    //   for(int i=0;i<16;i++)
    //   {
    //     Serial.println("Ping");
    //     d.parameter = i;
    //     Data.transmit(d, eInfrared);
    //     delay(1000);
    //   }

  // if (teamChanged())
  // {
  //   Animations::team_switch(blaster_team);
  // }
  // if (triggerPressed())
  // {
  //   uint16_t team = getHardwareTeam();
  //   // transmit_raw(team, true, true);
  //   Animations::shoot(blaster_team);
  // }

  // Animations::refresh();
}

bool triggerPressed()
{
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  return can_shoot && !digitalRead(TRIGGER_PIN);
}

bool teamChanged()
{
  if (fixed_team)
  {
    if (fixed_team != blaster_team)
    {
      blaster_team = fixed_team;
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
  pinMode(R_TEAM_PIN, INPUT_PULLUP);
  pinMode(G_TEAM_PIN, INPUT_PULLUP);
  pinMode(B_TEAM_PIN, INPUT_PULLUP);

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
  if (digitalRead(R_TEAM_PIN) & digitalRead(G_TEAM_PIN) & digitalRead(B_TEAM_PIN))
  {
    Serial.println("Enter Flash Test Mode");
    while (digitalRead(R_TEAM_PIN) & digitalRead(G_TEAM_PIN) & digitalRead(B_TEAM_PIN))
    {
      digitalWrite(A0, HIGH);
      delay(100);
      digitalWrite(A0, LOW);
      delay(500);
    }
    Serial.println("Exit Flash Test Mode");
  }
  pinMode(A0, INPUT_PULLUP);
}
