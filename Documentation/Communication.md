# Communication over the wire and IR

## Infrared
Infrared data is transmitted on 940 nm with a 38khz carrier wave.

## Protocol
IR and badgelink share the same protocol. The protocol they use is the standard JVC IR protocol. https://www.sbprojects.net/knowledge/ir/jvc.php  
This protocol is simple to implement and gives us 16 bits of data.

The implementation on the Time Blaster is compatible with the JVC protocol but has a few small differences:
 - Receiving/decoding only measure the full pulse (HLH) length. This is possible because the JVC protocol has unique pulse lengths for each type of signal (Start, 0, 1)
 - Data is not split into a Address and command area but in a data(12bit) and CRC(4bit) area.

# Data 

## data packet
The Time Blaster uses this 16 bit packet for all communication between blasters and also to and from the badge.

| Bit        | Name        | Use   |
| ---------: |-------------| -----|
| 0 | Team Color Red   | Set/get blaster team color |
| 1 | Team Color Green |  |
| 2 | Team Color Blue  |  |
| 3 | Trigger Status | Indicate if a shot was fired <br> also used for disabling the trigger |
| 4 | Command_0 | Send commands like animations |
| 5 | Command_1 |  |
| 6 | Command_2 |  |
| 7 | Command_3 |  |
| 8 | Parameter_0 | 4 parmeter bits for the command given |
| 9 | Parameter_1 |  |
| 10 | Parameter_2 |  |
| 11 | Parameter_3 |  |
| 12 | CRC_0 | Checksum for data validation |
| 13 | CRC_1 |  |
| 14 | CRC_2 |  |
| 15 | CRC_3 |  |

Depending on the source of the packet the blaster allows or rejects some commands.

## Team Color

### Badge to Blaster
000 : team is set by the hardware switch.  
All other: team is set by this.

### Blaster to Badge
Indicates the current selected team. The can be the team selected by switch or the team set by the badge in an earlier packet.  
value 000 is not allowed.

### Blaster to Blaster
Carries the team color of the blaster that fired.   
value 000 is not allowed.

## Trigger Status

### Badge to Blaster
0: Trigger on the blaster is disabled. Team LED is turned off  
1: Trigger on the blaster is enabled. Team LED is turned on

### Blaster to Badge
1: Blaster just fired a pulse
0: All other cases

### Blaster to Blaster
Always 1

## Command and Parameters

To be completed.

The blaster can send and receive commands. Each command can have a 4 bit parameter.

Some examples:
* Shoot
* Heal
* Set IR Channel 
* Set Fire Type (Damage|Healing)
* Set Game mode (Timeout when hit, zombie mode, slave(badge controlled))
* Got Hit
* Play Animation
* Team Switch Changed
* Chatter (Blasters start a short "discussion" about the meaning of life)
* Pull trigger (force blaster to shoot)
* Blaster ready

## CRC
This is the calculation used to get the check.

d0..d11 are the bits in position 0..11  
the 4 crc bits will be added so crc_0 will be bit_12, crc_1 => bit_13 etc.

```c++
  crc[0] = d11 ^ d10 ^ d9 ^ d8 ^ d6 ^ d4 ^ d3 ^ d0;  
  crc[1] = d8  ^ d7  ^ d6 ^ d5 ^ d3 ^ d1 ^ d0 ^ 1;  
  crc[2] = d9  ^ d8  ^ d7 ^ d6 ^ d4 ^ d2 ^ d1 ^ 1;  
  crc[3] = d10 ^ d9  ^ d8 ^ d7 ^ d5 ^ d3 ^ d2;  
```

Applying the same function to a packet with a CRC the CRC should become 0 if no corruption was detected.