
#ifndef DATA_H
#define DATA_H
#include <Arduino.h>

enum TeamColor : uint8_t 
{
  eNoTeam = 0b000,
  eTeamRex = 0b001,
  eTeamGiggle = 0b010,
  eTeamBuzz = 0b100,

  eTeamYellow = eTeamRex | eTeamGiggle,
  eTeamMMagenta = eTeamRex | eTeamBuzz,
  eTeamCyan = eTeamGiggle | eTeamBuzz,

  eTeamWhite = eTeamRex | eTeamGiggle | eTeamBuzz
};

enum CommandType : uint8_t
{
  eCommandShoot = 1,
  eCommandHeal = 2,
  eCommandSetChannel = 3,
  eCommandSetFireType = 4,
  eCommandSetGameMode = 5,
  eCommandGotHit = 6,
  eCommandPlayAnimation = 7,
  eCommandTeamSwitched = 8,
  eCommandChatter = 9,
  eCommandPullTrigger = 10,
  eCommandSetFlagsA = 11,
  eCommandSetFlagsB = 12,
  eCommandReservedA = 13,
  eCommandReservedB = 14,
  eCommandBlasterAck = 15,
};

union DataPacket 
{
  uint16_t raw;
  struct 
  {
    TeamColor team : 3;
    bool trigger_state : 1;
    CommandType command : 4;
    uint8_t parameter : 4;
    uint8_t crc : 4;
  };
};

enum DeviceType
{
  eInfrared = 0b01,
  eBadge = 0b10,
  eAllDevices = 0b11,
};

class DataReader
{
  private:
    volatile uint32_t refTime;
    volatile bool oldState;
    volatile uint16_t rawData;
    volatile uint8_t bitsRead;
    volatile bool dataReady;
  public:
    DataReader();
    void stopReading();
};

class _data
{ 
  private:
    _data();
    DataReader ir1_reader;
    DataReader ir2_reader;
    DataReader badge_reader;
    void setup_ir_carrier();
  public:
    static _data &getInstance();
    DataPacket calculateCRC(DataPacket packet);
    void transmit(DataPacket packet, DeviceType device);
    void disable_receive(DeviceType device);
};

extern _data &Data;
#endif
