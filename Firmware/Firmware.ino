#include "functions.h"
#include "animations.h"
#include "data.h"

#define VERSION 20220802

#define R_TEAM_PIN A5
#define G_TEAM_PIN 4
#define B_TEAM_PIN 8
#define TRIGGER_PIN 3

enum GameMode : uint8_t
{
  eGMTimeout = 0,
  eGMZombie = 1,
  eGMSuddenDeath = 2
};

/* Blaster state variables */
// if false the trigger is disabled. Set by the badge (trigger field)
bool can_shoot = true;
uint8_t ir_channel = 0b0000; // default channel
bool healing_mode = false;
bool stealth_mode = false;
bool single_shot_mode = false;
uint8_t game_mode = eGMTimeout;

uint8_t last_active_team = -1;
uint8_t badge_team = 0;  // the team set by the badge (over rules the blaster if != 0)
uint8_t zombie_team = 0;
uint8_t hardware_team = 0;

uint8_t chatter_limit = 10;
uint8_t hit_timeout = 3;

bool muted = false;
bool animated = true;
uint8_t brightness = 0b11;

void setup()
{
  setupButtons();
  setupCommunication();
  setupRandomSeed();
  setupAnimations();
  blinkIfNoTeamSelector();
  testMutedBoot();
 
  Animations::blaster_start();

  Serial.println(" * Blaster Ready\n\n");
  sendACK();
  delay(500);
  sendACK(true);  
}

void loop()
{
  if (teamChanged())
    Animations::team_switch(activeTeam(), can_shoot);

  if (triggerPressed())
  {
    if (can_shoot)
    {
      if (healing_mode)
      {
        healingShot();
        Animations::shoot(activeTeam());
      }
      else
      {
        damageShot();
        Animations::shoot(activeTeam());
      }
    }
    else
    {
      Animations::error();
    }
    if (single_shot_mode)
    {
      single_shot_mode = false;
      can_shoot = false;
      Animations::set_team_status(activeTeam(), can_shoot);
    }
    Animations::stealth(false);
  }

  handle_badge_packet(Data.readBadge());
  handle_ir_packet(Data.readIr());

  // bug: changing teams after healer fixed blaster causes new infection; disable team change
  // bug: should not be able to change team when in zombie mode
  if (game_mode == eGMZombie && activeTeam(true) != activeTeam(false))
  {
    Animations::FlickerTeam(activeTeam(), activeTeam(true));
  }
}

void handle_badge_packet(DataPacket packet)
{
  if (packet.command == 0)
    return;
  Serial.println("Received message from badge");

  switch (packet.command)
  {
  case eCommandSetChannel:
    sendACK();
    setChannel(packet);
    Serial.println("eCommandSetChannel");
    break;
  case eCommandSetTriggerAction:
    sendACK();
    setTriggerAction(packet);
    Serial.println("eCommandSetTriggerAction");
    break;
  case eCommandSetGameMode:
    sendACK();
    setGameMode(packet);
    Serial.println("eCommandSetGameMode");
    break;
  case eCommandTeamChange:
    sendACK();
    setTeamColor(packet);
    Serial.println("eCommandTeamChange");
    break;
  case eCommandShoot:
    sendACK();
    handle_damage_received(packet);
    Serial.println("eCommandShoot");
    break;
  case eCommandHeal:
    sendACK();
    handle_healing_received(packet);
    Serial.println("eCommandHeal");
    break;
  case eCommandPlayAnimation:
    sendACK();
    handle_play_animation(packet);
    Serial.println("eCommandPlayAnimation");
    break;
  case eCommandChatter:
    sendACK();
    handle_play_chatter(packet);
    Serial.println("eCommandChatter");
    break;
  case eCommandSetHitTimeout:
    sendACK();
    handle_set_hit_timeout(packet);
    Serial.println("eCommandSetHitTimeout");
    break;
  }
}

void handle_set_hit_timeout(DataPacket packet){
  hit_timeout = packet.parameter;
}

void handle_play_chatter(DataPacket packet){
  if (packet.parameter <= 0) return;
  if (packet.parameter >= chatter_limit) return;
  //chatter_limit = packet.parameter;
  delay(random(500,1000));
  if (random(30) == 0) Animations::wolfWhistle();
  else Animations::chatter();
  packet.parameter--;
  if (packet.parameter > 0){
    delay(random(250,500));
    Data.transmit(packet, eInfrared);
  }
  Animations::set_team_status(activeTeam(),can_shoot);
}

void handle_play_animation(DataPacket packet) {
  switch (packet.parameter){
    case Animations::eAnimationBlasterStart:
      Animations::blaster_start();
      break;
    case Animations::eAnimationError:
          Animations::error();
      break;
    case Animations::eAnimationCrash:
      Animations::crash(7,5);
      break;
    case Animations::eAnimationFireball:
      Animations::fireball();
      break;
    case Animations::eAnimationOneUp:
      Animations::one_up();
      break;
    case Animations::eAnimationCoin:
      Animations::coin();
      break;
    case Animations::eAnimationVoice:
      Animations::voice();
      break;
    case Animations::eAnimationWolfWhistle:
      Animations::wolfWhistle();
      break;
    case Animations::eAnimationChatter:
      Animations::chatter();
      break;    
    case Animations::eAnimationBlinkTeamLed:
      Animations::blink_team_led();
      break;
  }
  Animations::set_team_status(activeTeam(),can_shoot);

}

void handle_ir_packet(DataPacket packet)
{
  if (packet.command == 0)
    return;
  Serial.println("Received message from IR");
  switch (packet.command)
  {
  case eCommandShoot:
    Serial.println("eCommandShoot");
    handle_damage_received(packet);   
    break;
  case eCommandHeal:
    Serial.println("eCommandHeal");
    handle_healing_received(packet);
    break;
  case eCommandChatter:
    sendACK();
    Serial.println("eCommandChatter");
    handle_play_chatter(packet);
    break;
  }
}

void handle_damage_received(DataPacket packet)
{
  if (packet.parameter == ir_channel && packet.trigger_state == 1 && packet.team != activeTeam())
  {
    Animations::stealth(false);
    // send data to badge
    packet.trigger_state = 0; // we are not fireing
    Data.transmit(packet, eBadge);
    switch (game_mode)
    {
    case eGMTimeout:
      // action here depends on game mode
      Animations::crash(packet.team, hit_timeout);
      Data.readIr();    // clear buffer
      Data.readBadge(); // clear badge (in case the badge received the same packet) //todo: improve so that we only remove shoot commands
      Animations::clear();
      Animations::team_switch(activeTeam(), can_shoot);
      break;
    case eGMZombie:
      // action here depends on game mode
      Animations::crash(packet.team, hit_timeout);
      Data.readIr();    // clear buffer
      Data.readBadge(); // clear badge (in case the badge received the same packet) //todo: improve so that we only remove shoot commands
      Animations::clear();
      zombie_team = packet.team;
      healing_mode = false; // no such thing as healing zombies
      if (packet.team == activeTeam(true))
        zombie_team = 0;
      Animations::team_switch(activeTeam(), can_shoot);
      break;
    }
  }
}

void handle_healing_received(DataPacket packet) {}

void setGameMode(DataPacket packet)
{
  Serial.print("Received eCommandSetGameMode M:");
  Serial.print(packet.parameter);
  Serial.print(" T:");
  Serial.println(packet.team);
  if (packet.parameter == eGMTimeout)
  {
    game_mode = packet.parameter;
    can_shoot = true;
    healing_mode = false;
    single_shot_mode = false;
    zombie_team = 0;
    badge_team = packet.team;
  }
  else if (packet.parameter == eGMZombie)
  {
    game_mode = packet.parameter;
    can_shoot = true;
    healing_mode = false;
    single_shot_mode = false;
    zombie_team = 0;
    badge_team = packet.team;
  }
  else if (packet.parameter == eGMSuddenDeath)
  {
    game_mode = packet.parameter;
    can_shoot = true;
    healing_mode = false;
    single_shot_mode = false;
    zombie_team = 0;
    badge_team = packet.team;
  }
}

void setTeamColor(DataPacket packet)
{
  Serial.println("Received eCommandTeamChange");
  badge_team = packet.team;
}

void setTriggerAction(DataPacket packet)
{
  Serial.println("Received eCommandSetTriggerAction");
  stealth_mode = packet.parameter & 8;
  single_shot_mode = packet.parameter & 4;
  healing_mode = packet.parameter & 2;
  can_shoot = !(packet.parameter & 1);

  Animations::stealth(stealth_mode);
  Animations::set_team_status(activeTeam(), can_shoot);
}

void setChannel(DataPacket packet)
{
  Serial.println("Received eCommandSetChannel");
  ir_channel = packet.parameter;
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

/*
  Configure all switch inputs as input_pullup
*/
void setupButtons(){
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(R_TEAM_PIN, INPUT_PULLUP);
  pinMode(G_TEAM_PIN, INPUT_PULLUP);
  pinMode(B_TEAM_PIN, INPUT_PULLUP);
}

void setupCommunication(){
  Serial.begin(115200);
  Serial.println();
  Serial.println("___ _ _  _ ____    ___  _    ____ ____ ___ ____ ____");
  Serial.println(" |  | |\\/| |___    |__] |    |__| [__   |  |___ |__/ ");
  Serial.println(" |  | |  | |___    |__] |___ |  | ___]  |  |___ |  \\______\\/_");
  Serial.println("                                                          /\\");
  Serial.println();                                                
  Serial.println(" * Serial Ready.");
  Serial.print(" * Firmware version: ");
  Serial.println(VERSION);
  Data.init();
  Serial.println(" * Communication Ready.");
}

void setupRandomSeed(){
  auto analogValue = analogRead(A4);
  randomSeed(analogValue);
  Serial.print(" * Random seed: ");
  Serial.println(analogValue);
}

void setupAnimations(){
  Animations::setup();
  Serial.println(" * Animations ready.");
}

/* This is to give a visual signal that the firmware is flashed successfully when bulk uploading.
 * This has no use after flashing.
 */
void blinkIfNoTeamSelector()
{
  //Don't use gethardwareteam() here as it ignores team 0
  pinMode(R_TEAM_PIN, INPUT_PULLUP);
  pinMode(G_TEAM_PIN, INPUT_PULLUP);
  pinMode(B_TEAM_PIN, INPUT_PULLUP);
  pinMode(A0, OUTPUT);
  if (digitalRead(R_TEAM_PIN) && digitalRead(G_TEAM_PIN) && digitalRead(B_TEAM_PIN))  {
    Serial.println(" * No team switch detected, entering Post Flash Mode");
    while (digitalRead(R_TEAM_PIN) && digitalRead(G_TEAM_PIN) && digitalRead(B_TEAM_PIN))    {
      digitalWrite(A0, HIGH);
      delay(100);
      digitalWrite(A0, LOW);
      delay(500);
    }
    Serial.println(" * Exit Flash Test Mode");
  } else{
    Serial.println(" * Team switch detected");
  }
  pinMode(A0, INPUT);
}

void testMutedBoot(){
  if (triggerPressed())  {
    Serial.println(" * Trigger pressed, starting in muted mode.");
    Animations::mute();
    Serial.print(" * Waiting for trigger to be released ");
    while (triggerPressed()) {
      Animations::blink_team_led();
      Serial.print(".");
    }
    Serial.println();
  }
}

void sendACK()
{
  sendACK(false);
}

void sendACK(bool blasterReady)
{
  DataPacket d;
  d.team = 0;
  d.trigger_state = 0;
  d.command = eCommandBlasterAck;
  d.parameter = 0;
  if (blasterReady) {
    d.parameter |= 1;
  }
  Serial.println("Sending ACK to badge.");
  Data.transmit(d, eBadge);
}

bool triggerPressed()
{
  return !digitalRead(TRIGGER_PIN);
}

bool teamChanged()
{
  if (last_active_team != activeTeam())
  {
    last_active_team = activeTeam();
    return true;
  }
  return false;
}

// Get the team from the hardware switch.
uint8_t getHardwareTeam() {
  uint8_t r = (1 - digitalRead(R_TEAM_PIN)) * 1;
  uint8_t g = (1 - digitalRead(G_TEAM_PIN)) * 2;
  uint8_t b = (1 - digitalRead(B_TEAM_PIN)) * 4;

  uint8_t team = r + g + b;
  if (team != hardware_team && team != 0){
    hardware_team = team;
    DataPacket d;
    d.team = hardware_team;
    d.trigger_state = 0;
    d.command = eCommandTeamChange;
    d.parameter = 0;
    Serial.print("Sending Changed HW Team (=");
    Serial.print(hardware_team);
    Serial.println(") to badge.");
    Data.transmit(d, eBadge);
  }

  return hardware_team;
}

uint8_t activeTeam(bool ignore_zombie_team)
{
  if (zombie_team && !ignore_zombie_team)
    return zombie_team;
  if (badge_team)
    return badge_team;
  auto hardwareTeam = getHardwareTeam();
  if (hardwareTeam == 0)
    return last_active_team;
}
uint8_t activeTeam() { return activeTeam(false); }
