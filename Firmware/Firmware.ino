#include "functions.h"
#include "animations.h"
#include "data.h"

#define R_TEAM_PIN A5
#define G_TEAM_PIN 4
#define B_TEAM_PIN 8
#define TRIGGER_PIN 3

/* Blaster state variables */
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
  Serial.begin(115200);

  Data.init();

  // while (true){
  //   if (Serial.available()){
  //     auto msg = Serial.readStringUntil("\n");
  //     if (msg.substring(0, 2) == "tx")
  //     {
  //       if (msg.substring(3,5) == "bl") {
  //         Serial.println("Transmitting to blaster");
  //       } else if ((msg.substring(3,5) == "ir") {
  //       Serial.println(msg);
  //     } else Serial.println("Unknown command.");
  //   }
  // }

  

      

  // Serial.println();
  // while (true)
  //   ;

  blinkIfNoTeamSelector(); // This is only used when flashing the mc

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

  pinMode(BADGELINK_PIN, OUTPUT);
  digitalWrite(BADGELINK_PIN, LOW);
}

void loop()
{
    if(Data.dataReady()) Serial.println("Ready.");

    // DataPacket d;
    // d.team = eTeamRex;
    // d.trigger_state = 1;
    // d.command = eCommandShoot;
    // d.parameter = 0;

    // while (true)
    //   for(int i=0;i<16;i++)
    //   {
    //     Serial.println("Ping");
    //     //d.parameter = i;
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
  while (!(digitalRead(R_TEAM_PIN) | digitalRead(G_TEAM_PIN) | digitalRead(B_TEAM_PIN)))
  {
    digitalWrite(A0, HIGH);
    delay(500);
    digitalWrite(A0, LOW);
    delay(500);
  }
  pinMode(A0, INPUT_PULLUP);
}
