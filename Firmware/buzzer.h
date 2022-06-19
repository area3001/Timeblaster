
#ifndef BUZZER_H
#define BUZZER_H
#include <Arduino.h>

enum Note : uint16_t // 0..65535
{
  DO = 3270,
  DO_S = 3465,
  C = 3270,
  C_S = 3465,

  RE = 3671,
  RE_S = 3889,
  D = 3671,
  D_S = 3889,

  MI = 4120,
  E = 4120,

  FA = 4365,
  FA_S = 4625,
  F = 4365,
  F_S = 4625,

  SOL = 4900,
  SOL_S = 5191,
  G = 4900,
  G_S = 5191,

  LA = 5500,
  LA_S = 5827,
  A = 5500,
  A_S = 5827,

  SI = 6174,
  B = 6174
};

class _buzzer
{
private:
  _buzzer();
  bool sound_active = false;
  bool muted = false;
  void sound_on();
  void sound_off();

public:
  static _buzzer &getInstance();
  void playFrequency(int frequency);
  void playFrequency(int frequency, int duration);
  void playNote(Note note, int octave, int duration);
  void mute();
  void unmute();
};

extern _buzzer &Buzzer;

#endif
