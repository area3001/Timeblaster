# BadgeLink Data Structure
This document describes the data structure used to cummunicate between blasters and badges over IR and wire.

Data is transmitted in Data Packets of 12 bits + 4 bits for [CRC](Communication.md#crc) .
All packets are structured in the same way for simplicity sake.

# Table of existing messages
field marked with a ❌ in the table below are not used in this action. The receiver should ignore them and the sender should keep them 0.

| Bits > <br> Action ˅ |  8,9,10,11 <br> Parameter | 4,5,6,7 <br> Command | 3 <br> Trigger | 0,1,2 <br> Team |     |
|---|---|---|---|---|---|
| [Shoot](#shootheal) | Channel ID | 0x1 | 1/0 | RGB | IR + Wire |
| [Heal](#shootheal) | Channel ID | 0x2 | 1/0 | RGB | IR + Wire |
| [Channel](#Channel) | Channel ID | 0x3 | ❌ | ❌ | Wire: Badge → Blaster |
| [Trigger Action](#trigger-action) | (Stealth,Single Shot,Heal) | 0x4 | ❌ | ❌ | Wire: Badge → Blaster |   
| [Game Mode](#game-mode) | (Timeout,Zombie,Sudden Death) | 0x5 | ❌ | ❌ | Wire: Badge → Blaster |   
| Unused | ❌ | 0x6 | ❌ | ❌ |  |   
| [Animation](#animation) | Animation ID | 0x7 | ❌ | ❌ | Wire: Badge → Blaster |   
| [Team Change](#team-change) | (Zombi attack) | 0x8 | ❌ | RGB | Wire |   
| [Chatter](#chatter) | Time to Live | 0x9  | ❌ | ❌  | IR + Wire |  
| Unused | ❌ | 0xA | ❌ | ❌ | ❌| 
| [Settings](#settings) | (mute,brightness) | 0xB  | ❌ | ❌ | Wire: Badge → Blaster |   
| Unused | ❌ | 0xC |  ❌ | ❌ | ❌ |   
| Unused | ❌ | 0xD  | ❌ | ❌ | ❌ |   
| Unused | ❌ | 0xE  |  ❌ |  ❌ | ❌ |   
| [Ack](#Ack) | ❌ | 0xF  | ❌  | ❌  | Wire: Blaster → Badge |   


## Shoot/Heal
**Command**: 0x1 (or 0x2 for healing)   
**Parameter**: The channel ID. see [Channel](#Channel)   
**Trigger**: 1 unless notifying the badge of received shot   
**Team**: the team that generated/send the message   

The shoot/heal messages are used for the most basic functionality of the blaster, shooting.   

### Usage:
 - blaster 2 blaster over IR   
   Default action when pulling the trigger. The trigger field must be 1
 - blaster to badge over link   
   Right after receiving the message from an other blaster the receiver needs to notify the blaster of being shot. to do this it needs to modify the incoming message and remove the trigger bit. (set it to 0)
 - badge to blaster over link   
   The badge also has a IR receiver that can be shot. When the badge receives a shot it should retransmit the shot without modifying it to the blaster via wire to inform the blaster it was shot.


## Channel
**Command**: 0x3   
**Parameter**: The channel ID.   
**Trigger**: Not used   
**Team**: Not used

To support multiple parallel games without interference the blaster supports channels.   
Channels range from 0 to 15 where 0 is the default channel when starting the blaster.   
Channels can only be set by the [Channel](#channel) command from the badge.

When shooting both the sender and receiver need to be in the same channel for the shot to work.


## Trigger Action
**Command**: 0x4   
**Parameter**: 0: Stealth, 1: Single Shot, 2 Heal, 3 Disable   
**Trigger**: Not used   
**Team**: Not used

Set the blaster trigger bits. These bits define when the blaster does when pulling the trigger.

* **Bit 0**: Stealth (Default:0)   
  When set to 1 the blaster will turn off it's lights and make no sound when shooting.   
  This is reset when hit or after shooting one time.
* **Bit 1**: Single Shot (Default:0)   
  When this bit is set the blaster can only send one shot before disabling itself.
* **Bit 2**: Heal (Default:0)
  When set to 1 the blaster will send out healing shots (only to same team members)   
  When set to 0 the blaster will send out damaging shots (only to other team)
* **Bit 3**: Disable (Default:0)
  When set to 1 the trigger is disabled, however the blaster still can be shot by other teams.


## Game Mode
**Command**: 0x5  
**Parameter**: Game mode   
**Trigger**: Not used   
**Team**: Not used

Sets the same mode of the blaster.   
* Mode 0: Timeout mode. (Default)   
  When this mode is set the blaster will go in timeout mode when being shot. After a set time the blaster will auto heal and can continue the game.   
  If the blaster receives a healing shot this timeout can be shortened.
* Mode 1: Zombie mode   
  In this mode the blaster team will change to the color of the team that shot it.   
  The game is over when all players are of the same color.
* Mode 2: Sudden death mode   
  The blaster is very weak and will stop working after being shot once.   
  The winner is the last player with a working blaster

## Animation
**Command**: 0x7    
**Parameter**: Play animation  
**Trigger**: Not used   
**Team**: Not used

Tell the blaster to start one of the built in animation. Animations don't affect the state of the game.

### List of Animations
* eAnimationBlasterStart = 1,
* eAnimationError = 2,
* eAnimationCrash = 3,
* eAnimationFireball = 4,
* eAnimationOneUp = 5,
* eAnimationCoin = 6,
* eAnimationVoice = 7,
* eAnimationWolfWhistle = 8,
* eAnimationChatter = 9,
* eAnimationBlinkTeamLed = 15,


## Team Change
**Command**: 0x8     
**Parameter**: See details  
**Trigger**: Not used   
**Team**: Requested team change

This message can mean different things depending on who sends it.
* badge to blaster   
  Team: 0: release team lock (over wire)   
  Team : 1..15: lock blaster team.
  When this is set the blaster team will be locked to this team. This means that the hardware team switch is not working anymore. The blaster team can still change when playing in zombi mode.
* blaster to badge (over wire)
  Informs the badge that the hardware team switch was changed. Or when the Parameter is 1 if the team changed due to Zombie attack.

## Chatter
**Command**: 0x9   
**Parameter**: Time to live  
**Trigger**: Not used   
**Team**: Not used

*This is a gimmick and not a game mechanic*

When the badge sends this message to the blaster it initiates a blaster chatter session.   
The blaster that receives this message will start "talking" blaster language then transmit the chatter message over IR. Blasters who receive this message will also start "talking" and retransmitting.   

Every time a blaster retransmits a message is decreases the time to live field. When the TTL == 0 it will stop. A blaster will also ignore any packages with a higher TTL than the lowest TTL it has received until now.

## Settings
**Command**: 0xB  
**Parameter**: See details  
**Trigger**: Not used   
**Team**: Not used

Sets some settings on the blaster   
* Bit 0: Mute.   
  When set the blaster will be in mute mode
* Bit 1..3: Brightness (default = 7 (0b111))    
  This will change the brightness of all LEDS on the blaster.

## Ack
**Command**: 0xF   
**Parameter**: See details 
**Trigger**: Not used   
**Team**: Not used

The blaster should respond with an Ack on all valid messages send by the badge.
This should happen in less than 100ms after receiving the original message.
If a Ack is not received within 100ms the original packed is seen as a lost packet.

Parameter 0: Normal Ack (needed changes have been correctly received)
Parameter 1: Blaster Ready Ack (all needed changes have been processed)

The blaster will also send an Ack message when powered on.
