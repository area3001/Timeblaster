//keep track of the pulse length
byte irIn1Pulse[] = {0, 0};
byte buffer1[] = {0};
byte count = 0;
byte sending = false;
byte divider = 0;

/*
outbuffer contains pulse lengths in pairs (on-time, off-time)
time is in multiples of 562.5µs
reset = 16,8
one = 1,3
zero = 1,1

todo: add pointer to the current element for sending
when (divider++ == 43) 562.5µs have passed and the value of the element is decreased by one unless it is zero.
when zero we move to the next element in the buffer until we are at the end.
*/
byte outBuffer [10];

EnableIr() {
  TIMSK1 |= (1 << OCIE1A);
}

DisableIr() {
  TIMSK1 &= ~(1 << OCIE1A);
  PORTB = PORTB & ~0b00000010;
}

void setupTimer() {
  //http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html

  // TIMER 1 for interrupt frequency 76190.47619047618 Hz:
  cli(); // stop interrupts
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1  = 0; // initialize counter value to 0
  // set compare match register for 76190.47619047618 Hz increments
  OCR1A = 209; // = 16000000 / (1 * 76190.47619047618) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12, CS11 and CS10 bits for 1 prescaler
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei(); // allow interrupts

  DisableIr();
}

void SetupIr()
{
  pinMode(IrOut1_Pin, OUTPUT);
  pinMode(IrOut2_Pin, OUTPUT);
  pinMode(IrIn1_Pin, INPUT);
  pinMode(IrIn2_Pin, INPUT);

  setupTimer();
}

IrSend(uint32_t data)
{
  outBuffer[0] = 16;
  sending = true;

}

ISR(TIMER1_COMPA_vect) {
  if (sending) PORTB = PORTB ^ 0b00000010;

  if (divider++ == 43) {
    // place ir_in and ir_out logic here.
    // keep in mind that we only have a handfull of instructions to spare
    
    divider = 0;
    outBuffer[0]--;
    if (outBuffer[0] == 0) sending = false;

    //irin
    /* wait for a reset pulse
     * if a valid signal was received. update a global var & stop the IR interrupt
     * the main thread must decided wat tho do with the data received and restart IR when ready
     * 
     * Reset pulses and invalid input clear the receive buffer
    */
  }

  return;

  byte state = digitalRead(IrIn1_Pin); //to expensive
  /*
     1 = no pulse
     0 = pulse
  */

  if (state == 0)
  {
    if (irIn1Pulse[1] == 0)
    {
      irIn1Pulse[0]++;
    } else
    {
      if (irIn1Pulse[0] == 16 && irIn1Pulse[1] == 8) {
        buffer1[0] = 0;
        count = 0;
      }
      if (irIn1Pulse[0] == 1 && irIn1Pulse[1] == 1) {
        buffer1[0] = ((buffer1[0] << 1) | 0b00000001);
        count++;
      }
      if (irIn1Pulse[0] == 1 && irIn1Pulse[1] == 3) {
        buffer1[0] = (buffer1[0] << 1);
        count++;
      }

      // move outside of interrupt
      if (count == 8) {
        if (buffer1[0] == 0b11111100) strip.setPixelColor(1, 0x000000FF);
        if (buffer1[0] == 0b11100111) strip.setPixelColor(1, 0x00FF0000);
        if (buffer1[0] == 0b00111111) strip.setPixelColor(1, 0x0000FF00);
      }


      irIn1Pulse[0] = 1;
      irIn1Pulse[1] = 0;
    }
  } else
  {
    if (irIn1Pulse[0] == 0) {}
    else
    {
      irIn1Pulse[1]++;
      if (irIn1Pulse[1] > 200)
      {
        irIn1Pulse[0] = 0;
        irIn1Pulse[1] = 0;
      }
    }
  }
}
