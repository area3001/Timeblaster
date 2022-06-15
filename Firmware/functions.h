#ifndef Functions_h
#define Functions_h

#define R_TEAM_PIN A5
#define G_TEAM_PIN 4
#define B_TEAM_PIN 8
#define TRIGGER_PIN 3

byte _lastBlasterTeam = 99;

/* Get the team by reading the Team switch.
   0:Rex 1:Giggle 2:Buzz */

byte _readBlasterTeam()
{
  // Pins are configured with a internal pull up register.
  // HIGH/1 means not selected/pressed
  if (digitalRead(R_TEAM_PIN) == 0)
    return 0;
  else if (digitalRead(G_TEAM_PIN) == 0)
    return 1;
  else if (digitalRead(B_TEAM_PIN) == 0)
    return 2;
}

byte getBlasterTeam()
{
  _lastBlasterTeam = _readBlasterTeam();
  return _lastBlasterTeam;
}

bool hasBlasterTeamChanged()
{
  return (_lastBlasterTeam != _readBlasterTeam());
}

#endif
