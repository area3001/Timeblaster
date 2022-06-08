#define HARDWARE_VERSION 1

#include "constants.h"
#include "pins.h"
#include "functions.h"
#include "badgelink.h"
#include "animations.h"
//#include "communications.h"
#include "data.h"

volatile int8_t pulse_pointer = 0;            // which pulse are we transmitting
volatile int pulse_train[pulse_train_lenght]; // list of pulse lenghts to send
volatile byte transmitting = false;           // are we transmitting
volatile int pulsecount = 0;
volatile int last_pulse = -1;

volatile uint8_t data = 0;

void setup()
{  
  Serial.begin(115200);

  DataPacket d;
  d.team = eTeamRex;
  d.trigger_state = 1;
  d.command = eCommandChatter;
  d.parameter = 12;

  d = Data.calculateCRC(d);

  Serial.println();
  while(true);

  blinkIfNoTeamSelector(); //This is only used when flashing the mc

//  Communications::init();
//  Communications::enableIrReceive();

  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(BADGELINK_PIN, INPUT_PULLUP);

  Serial.println("Blaster starting");
  Serial.println("Init timers");
 // setup_ir_carrier();

  Animations::setup();

  Animations::mute();
  if (triggerPressed())
  {
    Animations::mute();
  }
  while (triggerPressed())
  {
    Animations::blink_team_led();
  }
  Animations::blaster_start();



  delay(1000);
}

void loop()
{
  

  if (teamChanged())
  {
    Animations::team_switch(blasterTeam);
  }
  if (triggerPressed())
  {
    uint16_t team  = getHardwareTeam();
    //transmit_raw(team, true, true);
    Animations::shoot(blasterTeam);
  }

  Animations::refresh();

}







ISR(TIMER2_COMPA_vect)
{
  // todo: reset all data in structs when settings transmiting to true;
  if (transmitting)
  {
    /* IR TRANSMIT */
    if (pulse_pointer % 2 == 1)
    {
      DDRB &= ~B00000010;
      digitalWrite(BADGELINK_PIN, HIGH);
    }
    else
    {
      DDRB |= B00000010;
      digitalWrite(BADGELINK_PIN, LOW);
    }

    /* decrease & move to next pulse length */
    pulse_train[pulse_pointer]--; // count down

    // if we reached the end go to the next pulse
    if (pulse_train[pulse_pointer] <= 0)
      pulse_pointer++;

    // unless we allready were on the last pulse
    if (pulse_pointer >= pulse_train_lenght)
      transmitting = false;
  }
  // else
  //{ // listen when not transmitting
  //   byte ir1_pin_state = digitalRead(IR_IN1_PIN);
  //   data_in_ir1.process_state(ir1_pin_state);
  // }
}


/* This is to give a visual signal that the firmware is flashed successfully when bulk uploading.
 * This has no use after flashing.
 */
void blinkIfNoTeamSelector()
{
  pinMode(R_TEAM_PIN, INPUT_PULLUP);
  pinMode(G_TEAM_PIN, INPUT_PULLUP);
  pinMode(B_TEAM_PIN, INPUT_PULLUP);
  pinMode(A0, OUTPUT);
  while (!(digitalRead(R_TEAM_PIN) | digitalRead(G_TEAM_PIN) | digitalRead(B_TEAM_PIN)))
  {
    digitalWrite(A0, HIGH);
    delay(500);
    digitalWrite(A0, LOW);
    delay(500);
  }
  pinMode(A0, INPUT_PULLUP);
}
