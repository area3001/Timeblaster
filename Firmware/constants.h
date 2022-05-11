// IR/Blasterlink Settings (JVC)
const int ir_shortest_pulse_length = 525;
const bool ir_send_start_pulse = true;
const int ir_start_high_time = 16; //ùs
const int ir_start_low_time = 8;   //µs
const int ir_zero_high_time = 1;   //µs
const int ir_zero_low_time = 1;    //µs
const int ir_one_high_time = 1;    //µs
const int ir_one_low_time = 3;     //µs
const bool ir_send_stop_pulse = true;
const int ir_stop_high_time = 1; //µs
const int ir_stop_low_time = 1;  //µs
const int ir_bit_lenght = 16;
const int pulse_train_lenght = ir_send_start_pulse * 2 + ir_bit_lenght * 2 + ir_send_stop_pulse * 2;

#define IR_OUT 1
#define BADGE_OUT 2
