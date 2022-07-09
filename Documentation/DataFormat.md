# BadgeLink Data Structure
This document describes the data structure used to cummunicate between blasters and badges over IR and wire.

Data is transmitted in Data Packets of 12 bits + 4 bits for [CRC](Communication.md#crc) .
All packets are structured in the same way for simplicity sake.

# Table of existing messages
field marked with a ❌ in the table below are not used in this action. The receiver should ignore them and the sender should keep them 0.

| Bits > <br> Action ˅ |  8,9,10,11 <br> Parameter | 4,5,6,7 <br> Command | 3 <br> Trigger | 0,1,2 <br> Team |     |
|---|---|---|---|---|---|
| Shoot | Channel ID | 0x1 | 1/0 | RGB | IR + Wire |
| Heal | Channel ID | 0b0010 | 1/0 | RGB | IR + Wire |
| Channel | Channel ID | 0b0011 | ❌ | ❌ | Wire: Badge → Blaster |
| Trigger Action | X \| Stealth \| Single Shot \| Heal/Damage | 0b0100 | ❌ | ❌ | Wire: Badge → Blaster |   
| Game Mode | Timeout\| Zombie | 0b0101 | ❌ | ❌ | Wire: Badge → Blaster |   
| Unused | ❌ | 0b0110 | ❌ | ❌ |  |   
| Animation | Animation ID | 0b0111 | ❌ | ❌ | Wire: Badge → Blaster |   
| Team Change | hardware | 0b1000 | ❌ | RGB | Wire |   
| Chatter | Time to Live | 0b1001  | ❌ | ❌  | IR + Wire |  
| Unused | ❌ | 0b1010 | ❌ | ❌ | ❌| 
| Settings | 0: mute 123: brightness | 0b1011  | ❌ | ❌ | Wire: Badge → Blaster |   
| Unused | ❌ | 0b1100 |  ❌ | ❌ | ❌ |   
| Unused | ❌ | 0b1101  | ❌ | ❌ | ❌ |   
| Unused | ❌ | 0b1110  |  ❌ |  ❌ | ❌ |   
| [Ack](#Ack) | ❌ | 0b1111  | ❌  | ❌  | Wire: Blaster → Badge |   

## Shoot/Heal
Command: 0x1 (or 0x2 for healing)
Parameter: The channel ID. see [Channels](#channels)

The shoot/heal messages are used for the most basic functionality of the blaster.   


Used:
 - blaster 2 blaster over IR
 - blaster to badge over IR
 - blaster to badge over link
 - badge to blaster over link


## (set) Channel

## (set) Trigger Action

## (set) Game Mode

## (start) Animation

## Team Change

## Chatter

## Settings

## Ack

The blaster should respond with an Ack on all valid messages send by the badge.
This should happen in less than 100ms after receiving the original message.
If a Ack is not received within 100ms the original packed is seen as a lost packet.

The blaster will also send an Ack message when powered on.

