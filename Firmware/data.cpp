#include "data.h"
#include <Arduino.h>

//Private
_data::_data()
{
  // nop
}

// Public
static _data &_data::getInstance()
{
  static _data data;
  return data;
}


DataPacket _data::calculateCRC(DataPacket packet)
{
  bool crc[] = {0,0,0,0};
  // makes computing the checksum a litle bit faster
  bool d0 =  bitRead(packet.raw,0);
  bool d1 =  bitRead(packet.raw,1);
  bool d2 =  bitRead(packet.raw,2);
  bool d3 =  bitRead(packet.raw,3);
  bool d4 =  bitRead(packet.raw,4);
  bool d5 =  bitRead(packet.raw,5);
  bool d6 =  bitRead(packet.raw,6);
  bool d7 =  bitRead(packet.raw,7);
  bool d8 =  bitRead(packet.raw,8);
  bool d9 =  bitRead(packet.raw,9);
  bool d10 =  bitRead(packet.raw,10);
  bool d11 =  bitRead(packet.raw,11);

  crc[0] = d11 ^ d10 ^ d9 ^ d8 ^ d6 ^ d4 ^ d3 ^ d0 ^ 0;
  crc[1] = d8  ^ d7  ^ d6 ^ d5 ^ d3 ^ d1 ^ d0 ^ 1;
  crc[2] = d9  ^ d8  ^ d7 ^ d6 ^ d4 ^ d2 ^ d1 ^ 1;
  crc[3] = d10 ^ d9  ^ d8 ^ d7 ^ d5 ^ d3 ^ d2 ^ 0;

  bitWrite(packet.crc, 0, crc[0] ^ bitRead(packet.crc,0));
  bitWrite(packet.crc, 1, crc[1] ^ bitRead(packet.crc,1));
  bitWrite(packet.crc, 2, crc[2] ^ bitRead(packet.crc,2));
  bitWrite(packet.crc, 3, crc[3] ^ bitRead(packet.crc,3));
  
  return packet;
}

_data &Data = Data.getInstance();
