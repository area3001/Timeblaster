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

}
#endif
