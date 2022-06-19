#include "data.h"
#include <Arduino.h>

void DataReader::handleState(bool state)
{
  if (dataReady)
    return; // don't read more data until the "buffer" is empty
  if (state == oldState)
    return;         // if the state didn't change then don't do anything. this happens if an other pin caused the interrupt
  oldState = state; // update the oldState value so we can detect the next pin change.
  if (state)
    return;                      // we are looking for a rising edge, but the signal is inverted so a falling edge is what we want.
  unsigned long time = micros(); // check time passed since boot up.

  /* Check total pulse length (rising to rising edge) allow for some deviation*/
  if (time - refTime > 12600 * 0.8 && time - refTime < 12600 / 0.8)
  {
    bitsRead = 0;
    rawData = 0;
  }
  else if (time - refTime > 2100 * 0.8 && time - refTime < 2100 / 0.8)
  {
    rawData = rawData >> 1; // make room for an extra bit
    rawData |= 0x8000;      // set left bit high
    if (++bitsRead == 16)
    {
      dataReady = 1;
    }
  }
  else if (time - refTime > 1050 * 0.8 && time - refTime < 1050 / 0.8)
  {
    rawData = rawData >> 1; // make room for an extra bit
    if (++bitsRead == 16)
    {
      dataReady = 1;
    }
  }
  refTime = time;
}
void DataReader::reset()
{
  rawData = 0;
  bitsRead = 0;
  dataReady = 0;
}
bool DataReader::isDataReady()
{
  return dataReady;
}
DataPacket DataReader::getPacket()
{
  DataPacket p;
  p.raw = rawData;
  return p;
}

/* Data Class
 */

// Private
_data::_data()
{
  ir1_reader.reset();
  ir2_reader.reset();
  badge_reader.reset();

  enableReceive(eBadge | eInfrared);
}

void _data::setup_ir_carrier()
{
  /* Setup Timer 1 to toggle the IR-LED D1 at 38khz
     This will generate a wave of approximately 38.1khz.
     http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
  */
  cli();              // Stop interrupts while we set up the timer
  TCCR1B = B00000000; // Stop Timer/Counter1 clock by setting the clock source to none.
  TCCR1A = B00000000; // Set Timer/Counter1 to normal mode.
  TCNT1 = 0;          // Set Timer/Counter1 to 0
  OCR1A = 209;        // = 16000000 / (1 * 76190.47619047618) - 1 (must be <65536)
  TCCR1A = B01000100; // Set Timer/Counter1 to CTC mode. Set OC1A to toggle.
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
  sei(); // allow interrupts
}

void _data::enableReceive(DeviceType device)
{
  PCICR |= 0b00000100; // turn on port D

  if (device & eInfrared)
  {
    pinMode(IR_IN1_PIN, INPUT_PULLUP);
    pinMode(IR_IN2_PIN, INPUT_PULLUP);
    PCMSK2 |= 0b01100000; // turn on pins 5 & 6
  }

  if (device & eBadge)
  {
    pinMode(BADGELINK_PIN, INPUT_PULLUP);
    PCMSK2 |= 0b10000000; // tun on pin 7
  }
}

void _data::disableReceive(DeviceType device)
{
  if (device & eInfrared)
  {
    PCMSK2 &= ~0b01100000; // turn off pins 5 & 6
  }
  if (device & eBadge)
  {
    PCMSK2 &= ~0b10000000; // tun off pin 7
  }
}

void _data::setup_data_timer()
{
  /* Set up timer 2 for data transfer.
      The JVC protocol has pulses that are multiples of 525 µs
      So we need a frequency of 1904.8 Hz (aproximatly)
      CPU: 16.000.000 Hz
      Prescaler: 64
      Timer freq: 16.000.000 / 64 = 250.000 Hz
      OCRA2: 250.000 / 1904.8 - 1 = 130.25 ~ 130

      This gives us an interrupt frequency of 1908.4 Hz which is good enough

      Ref: http://www.8bit-era.cz/arduino-timer-interrupts-calculator.html
   */

  // TIMER 2 for interrupt frequency 1908.4 Hz:
  cli();      // stop interrupts
  TCCR2A = 0; // set entire TCCR2A register to 0
  TCCR2B = 0; // same for TCCR2B
  TCNT2 = 0;  // initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 130;
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS22 bit for /64 prescaler
  TCCR2B |= (1 << CS22);
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei(); // allow interrupts

  // Stop timer
  // TIMSK2 &= ~(1 << OCIE2A);
}

void _data::prepare_pulse_train(DataPacket packet)
{
  auto data = packet.raw;
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
}

// Public
static _data &_data::getInstance()
{
  static _data data;
  return data;
}

void _data::init()
{
  pinMode(BADGELINK_PIN, INPUT_PULLUP);
  transmitting = false;
  transmit_badge = false;
  transmit_ir = false;
  pulse_pointer = 0;
  setup_data_timer();
}

/* Calculate the 4-bit CRC and xor it with the existing CRC.
 * For new packages it add the CRC
 * For existing packages it will set the CRC to 0 if the existing CRC was correct.
 */
DataPacket _data::calculateCRC(DataPacket packet)
{
  bool crc[] = {0, 0, 0, 0};
  // makes computing the checksum a litle bit faster
  bool d0 = bitRead(packet.raw, 0);
  bool d1 = bitRead(packet.raw, 1);
  bool d2 = bitRead(packet.raw, 2);
  bool d3 = bitRead(packet.raw, 3);
  bool d4 = bitRead(packet.raw, 4);
  bool d5 = bitRead(packet.raw, 5);
  bool d6 = bitRead(packet.raw, 6);
  bool d7 = bitRead(packet.raw, 7);
  bool d8 = bitRead(packet.raw, 8);
  bool d9 = bitRead(packet.raw, 9);
  bool d10 = bitRead(packet.raw, 10);
  bool d11 = bitRead(packet.raw, 11);

  crc[0] = d11 ^ d10 ^ d9 ^ d8 ^ d6 ^ d4 ^ d3 ^ d0 ^ 0;
  crc[1] = d8 ^ d7 ^ d6 ^ d5 ^ d3 ^ d1 ^ d0 ^ 1;
  crc[2] = d9 ^ d8 ^ d7 ^ d6 ^ d4 ^ d2 ^ d1 ^ 1;
  crc[3] = d10 ^ d9 ^ d8 ^ d7 ^ d5 ^ d3 ^ d2 ^ 0;

  bitWrite(packet.crc, 0, crc[0] ^ bitRead(packet.crc, 0));
  bitWrite(packet.crc, 1, crc[1] ^ bitRead(packet.crc, 1));
  bitWrite(packet.crc, 2, crc[2] ^ bitRead(packet.crc, 2));
  bitWrite(packet.crc, 3, crc[3] ^ bitRead(packet.crc, 3));

  return packet;
}

void _data::transmit(DataPacket packet, DeviceType device)
{
  Serial.println("TRANSMITTING PACKET");
  // 1) Disable Receiving for each device
  // disable_receive(device);

  // 2) Clear and recalculate CRC
  packet.crc = 0;
  packet = calculateCRC(packet);

  prepare_pulse_train(packet);

  if (device & eBadge)
  {
    Serial.println("TO BADGE");
    // stop RX
    // set badge send
    transmit_badge = true;
  }
  if (device & eInfrared)
  {
    Serial.println("TO IR");
    // stop RX
    // set ir send
    transmit_ir = true;
    setup_ir_carrier();
  }

  // enable transmit
  pinMode(BADGELINK_PIN, OUTPUT);
  transmitting = true;
  while (transmitting)
  {
  }
  pinMode(BADGELINK_PIN, INPUT);

  // 3) Calculate output buffer(s)
  // 4) Enable transmit and wait
  // 5) Clean receiving buffers
  // 6) Enable Receiving on selected devices
}

/* Keep this as short (in time) as possible.
 * This ISR is called 1908 times per second.
 */
void _data::transmit_ISR()
{
  if (transmitting)
  {
    if (pulse_pointer % 2 == 1) // would & 0b1 be faster?
    {
      DDRB &= ~B00000010;
      if (transmit_badge)
        digitalWrite(BADGELINK_PIN, LOW);
    }
    else
    {
      DDRB |= B00000010;
      if (transmit_badge)
        digitalWrite(BADGELINK_PIN, HIGH);
    }
    pulse_train[pulse_pointer]--; // count down

    // if we reached the end go to the next pulse
    if (pulse_train[pulse_pointer] <= 0)
      pulse_pointer++;

    // unless we allready were on the last pulse
    if (pulse_pointer >= pulse_train_lenght)
    {
      transmitting = false;
      transmit_badge = false;
      transmit_ir = false;
    }
  }
}

void _data::receive_ISR(uint8_t state)
{
  Serial.println(state, BIN);
}

_data &Data = Data.getInstance();

ISR(TIMER2_COMPA_vect)
{
  Data.transmit_ISR();
}

ISR(PCINT2_vect)
{
  bool IR1 = (PIND & 0b01000000);
  bool IR2 = (PIND & 0b00100000);
  bool Badge = (PIND & 0b10000000);
  Data.receive_ISR(PIND & 0b11100000);
}
