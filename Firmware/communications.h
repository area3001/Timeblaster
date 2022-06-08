#ifndef Communications_h
#define Communications_h


namespace Communications
{
class DataReader {
    volatile uint32_t refTime = 0;
    volatile bool oldState = 0;
    volatile uint16_t rawData = 0;
    volatile uint8_t bitsRead = 0;
    volatile bool dataReady = 0;
  public:
    void handleState(bool state)
    {
      if (dataReady) return; //don't read more data until the "buffer" is empty
      if (state == oldState) return; // if the state didn't change then don't do anything. this happens if an other pin caused the interrupt
      oldState = state; // update the oldState value so we can detect the next pin change.
      if (state) return; // we are looking for a rising edge, but the signal is inverted so a falling edge is what we want.
      unsigned long time = micros(); //check time passed since boot up.

      /* Check total pulse length (rising to rising edge) allow for some deviation*/
      if (time - refTime > 12600 * 0.8 && time - refTime < 12600 / 0.8)
      {
        bitsRead = 0;
        rawData = 0;
      } 
      else if (time - refTime > 2100 * 0.8 && time - refTime < 2100 / 0.8) 
      {
        rawData = rawData >> 1; //make room for an extra bit
        rawData |= 0x8000; //set left bit high
        if (++bitsRead == 16) {
          dataReady = 1;
        }
      }
      else if (time - refTime > 1050 * 0.8 && time - refTime < 1050 / 0.8) 
      {
        rawData = rawData >> 1; //make room for an extra bit
        if (++bitsRead == 16) {
          dataReady = 1;
        }
        
      }
      refTime = time;
    }

    bool isDataReady() {
      return dataReady;
    }
    
    uint16_t getRawData() {
      uint16_t data = rawData;
      dataReady = 0;
      return data;
    }
};

DataReader ir1 = DataReader();
DataReader ir2 = DataReader();
DataReader badgeLink;


void enableIrReceive()
{
  pinMode(IR_IN1_PIN, INPUT_PULLUP);
  pinMode(IR_IN2_PIN, INPUT_PULLUP);
  PCICR |= 0b00000100;    // turn on port D
  PCMSK2 |= 0b01100000;    // turn on pins 5 & 6
}

void disableIrReceive()
{
  PCMSK2 &= ~0b01100000;    // turn on pins 5 & 6
  }

bool irDataReady() {
  return ir1.isDataReady() || ir2.isDataReady();
}

uint16_t getIrData() {
  if (ir1.isDataReady()) {
    uint16_t data = ir1.getRawData();
    ir2.getRawData();
    return data;
  }
  if (ir2.isDataReady()) {
    uint16_t data = ir2.getRawData();
    ir1.getRawData();
    return data;
  }
  return 0;
}

ISR(PCINT2_vect)
{
  bool IR1 = (PIND & 0b01000000);
  bool IR2 = (PIND & 0b00100000);
  ir1.handleState(IR1);
  ir2.handleState(IR2);
}

uint16_t add_checksum(uint16_t data) {
  bool crc[] = {0,0,0,0};
  // makes computing the checksum a litle bit faster
  bool d0 =  bitRead(data,0);
  bool d1 =  bitRead(data,1);
  bool d2 =  bitRead(data,2);
  bool d3 =  bitRead(data,3);
  bool d4 =  bitRead(data,4);
  bool d5 =  bitRead(data,5);
  bool d6 =  bitRead(data,6);
  bool d7 =  bitRead(data,7);
  bool d8 =  bitRead(data,8);
  bool d9 =  bitRead(data,9);
  bool d10 =  bitRead(data,10);
  bool d11 =  bitRead(data,11);

  crc[0] = d11 ^ d10 ^ d9 ^ d8 ^ d6 ^ d4 ^ d3 ^ d0 ^ 0;
  crc[1] = d8  ^ d7  ^ d6 ^ d5 ^ d3 ^ d1 ^ d0 ^ 1;
  crc[2] = d9  ^ d8  ^ d7 ^ d6 ^ d4 ^ d2 ^ d1 ^ 1;
  crc[3] = d10 ^ d9  ^ d8 ^ d7 ^ d5 ^ d3 ^ d2 ^ 0;

  bitWrite(data, 12, crc[0] ^ bitRead(data,12));
  bitWrite(data, 13, crc[1] ^ bitRead(data,13));
  bitWrite(data, 14, crc[2] ^ bitRead(data,14));
  bitWrite(data, 15, crc[3] ^ bitRead(data,15));

  return data;
}

bool validate_checksum(uint16_t data){
  return (add_checksum(data) & 0b1111000000000000UL) == 0;
}

class a {};
}
#endif
