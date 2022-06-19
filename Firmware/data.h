
#ifndef DATA_H
#define DATA_H
#include <Arduino.h>

const bool ir_send_start_pulse = true;
const bool ir_send_stop_pulse = true;
const int ir_bit_lenght = 16;
const int ir_start_high_time = 16;
const int ir_start_low_time = 8;
const int ir_zero_high_time = 1;
const int ir_zero_low_time = 1;
const int ir_one_high_time = 1;
const int ir_one_low_time = 3;
const int ir_stop_high_time = 1;
const int ir_stop_low_time = 1;
const int pulse_train_lenght = ir_send_start_pulse * 2 + ir_bit_lenght * 2 + ir_send_stop_pulse * 2;
#define IR_IN1_PIN 5 // PD PCINT21
#define IR_IN2_PIN 6 // PD PCINT22
#define BADGELINK_PIN 7

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
  void handleState(bool state);
  void reset();           // clear buffer
  bool isDataReady();     // check buffer, if valid True, if invalid ResetBuffer
  DataPacket getPacket(); // return packet and reset; Dataclass then needs to calculate CRC
};

class _data
{
private:
  _data();
  DataReader ir1_reader;
  DataReader ir2_reader;
  DataReader badge_reader;

  volatile bool transmitting;
  volatile bool transmit_badge;
  volatile bool transmit_ir;
  volatile int pulse_train[pulse_train_lenght];
  volatile int8_t pulse_pointer;

  void setup_ir_carrier();
  void setup_data_timer();
  void prepare_pulse_train(DataPacket packet);
  void enableReceive(DeviceType device);
  void disableReceive(DeviceType device);

public:
  void dataReady();
  static _data &getInstance();
  DataPacket calculateCRC(DataPacket packet);
  void transmit(DataPacket packet, DeviceType device);
  void transmit_ISR();             // function called by ISR
  void receive_ISR(uint8_t state); // function called by ISR
  void init();
};

extern _data &Data;
#endif
