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

    
    uint16_t getRawData() {
      uint16_t data = rawData;
      dataReady = 0;
      return data;
    }
};
}
#endif
