# Communication over the wire and IR

## Infrared
Infrared data is transmitted on 940 nm with a 38khz carrier wave.

## Protocol
IR and badgelink share the same protocol. The protocol they use is the standard JVC IR protocol. https://www.sbprojects.net/knowledge/ir/jvc.php  
This protocol is simple to implement and gives us 16 bits of data.

The implementation on the Time Blaster is compatible with the JVC protocol but has a few small differences:
 - Receiving/decoding only measure the full pulse (HLH) length. This is possible because the JVC protocol has unique pulse lengths for each type of signal (Start, 0, 1)
 - Data is not split into a Address and command area but in a data(12bit) and CRC(4bit) area.
 - The blaster does not use the repeat feature of the JVC IR protocol.


# Timings
When sending or replying to a message it is important to wait aprox. 4ms before sending a message. This is to give the other device the time to switch back from sending to receiving.

# Data 

## data packet
The Time Blaster uses 16 bit messages for all communication between blasters and also to and from the badge. see [DataFormat](DataFormat.md) for implementation details.

Depending on the source of the packet the blaster allows or rejects some commands.

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