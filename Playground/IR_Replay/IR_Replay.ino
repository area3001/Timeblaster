#define IR_IN1_PIN 5 // PD PCINT21

#define buffer_size 70
volatile uint32_t time_ref = 0;
volatile uint32_t queue[buffer_size];
volatile uint8_t qpointer = 0;

void setup() {
  pinMode(IR_IN1_PIN, INPUT);
  PCICR |= 0b00000100; // turn on port D
  PCMSK2 |= 0b00100000; // turn on pin 5 
  Serial.begin(115200);
}


void loop() {
  if (qpointer == buffer_size) {
    Serial.println("Data:");
    for(int i =0; i<qpointer; i++) {
      Serial.println(queue[i]);
      }
    Serial.println("====================================================");
    delay(5000);
    qpointer = 0;
    }
}

ISR(PCINT2_vect)
{
  bool IR1 = (PIND & 0b00100000);
  uint32_t pulse_length = micros()-time_ref;
  time_ref = micros();
  if (qpointer >= buffer_size) return;
  if (time_ref == 0) return;
  if (pulse_length > 1000000L) {
    qpointer = 0;
    Serial.println("*");
    return;
  }

  queue[qpointer++] = pulse_length;
  
}
