#include "buzzer.h"
#include <Arduino.h>

/*
  This library is hardcoded to use pin 10.
  Ony pins 3,5,6,9,10 ana 11 could potentially be configured to produce sound this way.
  look up Timers in the atmega 328p datasheet to find out how.
*/
#define BUZZER_PIN 10

/* Private methods */
_buzzer::_buzzer()
{
  pinMode(BUZZER_PIN, OUTPUT);
}

void _buzzer::sound_on()
{
  if (sound_active)
    return;
  cli();
  TCCR1A = 0;                                        // set entire TCCR1A register to 0
  TCCR1B = 0;                                        // same for TCCR1B
  TCNT1 = 0;                                         // initialize counter value to 0
  OCR1A = 16000000 / (2 * 10000) - 1;                // set compare match register
  TCCR1B |= (1 << WGM12);                            // turn on CTC mode
  TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10); // Set CS12, CS11 and CS10 bits for 8 prescaler
  TCCR1A |= (1 << COM1B0);                           // Toggle pin 10
  TCCR1A &= ~(1 << COM1A0);                          // disable IR out
  sei();
  sound_active = true;
}

void _buzzer::sound_off()
{
  if (!sound_active)
    return;
  while (TCNT1 != 0)
    ; // Wait for the counter to overflow. This prevents a clicking sound when changing frequencies.
  cli();
  TCCR1A &= ~(1 << COM1B0); // disable toggling of pin 10 (BUZZER)
  sei();
  sound_active = false;
}

/* Pulbic methods */

/*  Create a singleton instance of the _buzzer class.
    The singleton prevents multiple instances which could mess up the timer configuration.
*/
static _buzzer &_buzzer::getInstance()
{
  static _buzzer buzzer;
  return buzzer;
}

void _buzzer::playFrequency(int frequency)
{
  if (muted)
    return sound_off();

  // disable sound when out of audible range
  if (frequency < 20 || frequency > 200000)
    return sound_off();

  sound_on();

  if (frequency <= 4000)
  {
    unsigned int reg_value = 16000000L / 2 / 8 / frequency - 1;
    if (TCNT1 >= reg_value)
      TCNT1 = 0; // reset the timer counter in case it's bigger than the match value
    OCR1A = reg_value;
    TCCR1B |= (1 << CS11);
    TCCR1B &= ~(1 << CS10);
  }
  else
  {
    unsigned int reg_value = 16000000L / 2 / frequency - 1;
    if (TCNT1 >= reg_value)
      TCNT1 = 0; // reset the timer counter in case it's bigger than the match value
    OCR1A = reg_value;
    TCCR1B |= (1 << CS10);
    TCCR1B &= ~(1 << CS11);
  }
}

void _buzzer::playFrequency(int frequency, int duration)
{
  playFrequency(frequency);
  delay(duration);
  sound_off();
}

void _buzzer::playNote(Note note, int octave, int duration)
{
  uint32_t _note = note * (1L << octave) / 100;
  playFrequency(_note, duration);
}

void _buzzer::mute()
{
  muted = true;
}

void _buzzer::unmute()
{
  muted = false;
}

_buzzer &Buzzer = Buzzer.getInstance();
