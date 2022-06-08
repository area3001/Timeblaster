#include "data.h"
#include <Arduino.h>


  DataReader::DataReader()
  {
  }
  void DataReader::stopReading()
  {
  }


//Private
_data::_data()
{
    ir1_reader.stopReading();
    ir2_reader.stopReading();
    badge_reader.stopReading();
}

void _data::setup_ir_carrier()
{
  /* Setup Timer 1 to toggle the IR-LED D1 at 38khz
     This will generate a wave of approximately 38.1khz.
     http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
  */
  cli();              // Stop interrupts while we set up the timer
  TCCR1B = B00000000; // Stop Timer/Counter1 clock by setting the clock source to none.
  TCCR1A = B00000000; // Set Timer/Counter1 to normal mode.
  TCNT1 = 0;          // Set Timer/Counter1 to 0
  OCR1A = 209; // = 16000000 / (1 * 76190.47619047618) - 1 (must be <65536)
  TCCR1A = B01000100; // Set Timer/Counter1 to CTC mode. Set OC1A to toggle.
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  sei();              // allow interrupts
}


// Public
static _data &_data::getInstance()
{
  static _data data;
  return data;
}


/* Calculate the 4-bit CRC and xor it with the existing CRC. 
 * For new packages it add the CRC 
 * For existing packages it will set the CRC to 0 if the existing CRC was correct.
*/
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

void _data::transmit(DataPacket packet, DeviceType device)
{
  // 1) Disable Receiving for each device
  disable_receive(device);
  
  // 2) Clear and recalculate CRC
  packet.crc = 0;
  packet = calculateCRC(packet);
  
  // 3) Calculate output buffer(s)
  // 4) Enable transmit and wait
  // 5) Clean receiving buffers
  // 6) Enable Receiving on selected devices
}

void _data::disable_receive(DeviceType device)
{
  if (device & eInfrared) 
  {
    //set receive to false
    //clear buffer pointer
  }
  if (device & eBadge)
  {
    //set receive to false
    //clear buffer pointer
  }
}

_data &Data = Data.getInstance();
