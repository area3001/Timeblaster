#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixels(1, 10, NEO_GRB + NEO_KHZ800);

/* Sony 12-bit SIRC protocol*/
#define trigger_pin 8
//or 3

uint16_t base = 600;
uint8_t start[] = {4, 1};
uint8_t one[] = {2, 1};
uint8_t zero[] = {1, 1};
uint16_t trail = 10000;
uint8_t repeat = 3;


uint16_t transmit_times[] = {
  2400, 600,

  /*0010101 (21) == power*/
  1200, 600, //1
  600, 600,  //0
  1200, 600, //1
  600, 600,  //0
  1200, 600, //1
  600, 600,  //0
  600, 600,  //0

  // 10000 (16)   Cassette Deck / Tuner
  600, 600, //0
  600, 600, //0
  600, 600, //0
  600, 600, //0
  1200, 600 //1
};

void setup() {
  pinMode(trigger_pin, INPUT_PULLUP);
  setup_carrier();
  pixels.begin();
}

void loop() {
  auto data_length = sizeof(transmit_times) / sizeof(transmit_times[0]);

  if (digitalRead(trigger_pin) == 0) {
    pixels.setPixelColor(0, pixels.Color(50, 50, 50));
    pixels.show();
    for (int y = 0; y < repeat; y++) {
      for (int i = 0; i < data_length; i++)
      {
        auto ref = micros();
        if (i % 2 == 0)
          DDRB |= B00000010;
        else
          DDRB &= ~B00000010;
        while ((micros() - ref) < (transmit_times[i])) ;
      }
      auto ref = micros();
      while ((micros() - ref) < 10000);
    }
    delay(100);
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();
  }
}

void setup_carrier()
{
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
