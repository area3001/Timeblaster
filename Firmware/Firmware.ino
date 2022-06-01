#define HARDWARE_VERSION 1

#include "constants.h"
#include "pins.h"
#include "functions.h"
#include "badgelink.h"
#include "animations.h"
#include "communications.h"
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

  DataPacket d2 = Data.calculateCRC(d);
  DataPacket d3 = Data.calculateCRC(d2);
  Serial.println(d.raw,BIN);  
  Serial.println(d2.raw,BIN);  
  Serial.println(d3.raw,BIN);

  Serial.println();
  while(true);

  blinkIfNoTeamSelector(); //This is only used when flashing the mc

  Communications::init();
  Communications::enableIrReceive();

  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(BADGELINK_PIN, INPUT_PULLUP);

  Serial.println("Blaster starting");
  Serial.println("Init timers");
  setup_ir_carrier();

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
  if (Communications::irDataReady()) {
    auto data = Communications::getIrData();
    if (Communications::validate_checksum(data)) 
    {
      Serial.println();
      Serial.println(data, BIN);
      Serial.println(getHardwareTeam(), BIN);
      Serial.println();
      
      if ((data & 0x0007) != getHardwareTeam()) 
      {  
        Animations::crash(data & 0x0007);
        delay(5000);
        Animations::clear();
        Animations::team_switch(blasterTeam);
      }
    }
  }

  if (teamChanged())
  {
    Animations::team_switch(blasterTeam);
  }
  if (triggerPressed())
  {
    uint16_t team  = getHardwareTeam();
    transmit_raw(team, true, true);
    Animations::shoot(blasterTeam);
  }

  Animations::refresh();

}

void transmit_raw(unsigned int data, bool ir_out, bool badgelink_out)
{
  /*
     Create a pulse train of high's and lows. send the Least Significant Bit first.
  */

  data &= 0x0FFF;
  data = Communications::add_checksum(data);
  Serial.println(data, BIN);
  
  Communications::disableIrReceive();
  byte index = 0;
  if (ir_send_start_pulse)
  {
    pulse_train[index++] = ir_start_high_time;
    pulse_train[index++] = ir_start_low_time;
  }
  for (int i = 0; i < ir_bit_lenght; i++)
  {
    if (bitRead(data, i))
    {
      pulse_train[index++] = ir_one_high_time;
      pulse_train[index++] = ir_one_low_time;
    }
    else
    {
      pulse_train[index++] = ir_zero_high_time;
      pulse_train[index++] = ir_zero_low_time;
    }
  }
  if (ir_send_stop_pulse)
  {
    pulse_train[index++] = ir_stop_high_time;
    pulse_train[index++] = ir_stop_low_time;
  }

  pulse_pointer = 0;
  pinMode(BADGELINK_PIN, OUTPUT);
  setup_ir_carrier();
  transmitting = true;
  // wait for sending to end
  while (transmitting)
    delay(10);
  pinMode(BADGELINK_PIN, INPUT_PULLUP);
  Communications::enableIrReceive();
}

void setup_ir_carrier()
{
  /* Setup Timer 1 to toggle the IR-LED D1 at 38khz
     This will generate a wave of approximately 38.1khz.
     http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
  */
  cli();              // Stop interrupts while we set up the timer
  TCCR1B = B00000000; // Stop Timer/Counter1 clock by setting the clock source to none.
  TCCR1A = B00000000; // Set Timer/Counter1 to normal mode.
  TCNT1 = 0;          // Set Timer/Counter1 to 0
  OCR1A = 209; // = 16000000 / (1 * 76190.47619047618) - 1 (must be <65536)
  TCCR1A = B01000100; // Set Timer/Counter1 to CTC mode. Set OC1A to toggle.
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  sei();              // allow interrupts
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
