void EnableTrigger()
{
  pinMode(Trigger_Pin, INPUT_PULLUP);
  PCICR |= 0b00000001;  //PORTB
  PCMSK0 |= 0b00000001; //PCINT0
}

void DisableTrigger()
{
  PCICR &= 0b11111110;  //PORTB
  PCMSK0 &= 0b11111110; //PCINT0
}

/*
  https://thewanderingengineer.com/2014/08/11/arduino-pin-change-interrupts/
  When one of the configured pins on PORTB causes an Interrupt
  We don't get pin or state information, so a digitalRead is requried
*/
ISR(PCINT0_vect)
{
  byte trigger = digitalRead(Trigger_Pin);

  if (trigger == 0)
  {
    for (byte i = 1; i < 5; i++) strip.setPixelColor(i, WhiteRGB);
  } else
  {
    for (byte i = 1; i < 5; i++) strip.setPixelColor(i, BlackRGB);
  }
}
