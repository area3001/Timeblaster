# A note on multitasking on the blaster

The blaster sometimes needs to run some tasks in parallel.
- create a 38khz carrier wave for IR out
- read and interpret IR in sent on 2 channels
- Perform bi-directional 1 wire communication with the badge
- Drive ws2812 LEDS with high accuracy.
- Play sounds
- run the game logic.

This is not quite possible on a single-core 16 Mhz (8-bit) cpu so some tricks and compromises were made.

The Arduino has 3 timers to use (see datasheet for details). Some of these timers can be programmed to toggle a pin, create PWM etc.

1) Timer 1 is shared by both IR for the carrier wave and the buzzer.   
   This means that IR out and sound can never happen at the same time.

   Solution: IR out only takes about 30 ms so in cases where we need IR out and sound we first send out IR then reconfigure the timer for sound. For the player both events seem to happen at the same time.

2)  Data is send in pulses that are multiples of 525µs. This both for IR and to the badge.   
    For this timer 2 is used. The data timer is not shared with other timers.
    This means we can send out data while sound is playing or while data is being received on an other channel.

    Note that while transmitting data on a channel (IR, wire) receiving data is disable on that channel. For IR this is done so we don't receive or own IR signal. For the wire channel this is done because this is a half duplex device.

3) driving WS2812 leds requires very precise timings. To accomplish this the neopixel library turns of interrupts before updating the LEDS.   
   This means that data communication is halted while updating the LEDS.   
   More LED updates means more packets that get lost.   
   Some things are not adviced for this reason:
   * constantly fade/heartbeat LED's
   * play animations on the LED's while we expect data to come in.
   * refresh the LEDS to often

   A packet loss detection mechanism for communication from the badge to the blaster has been created. The blaster has to ACK all packets received from the badge in less than 100 ms.

4) Reading data   
   Reading data pulses from the badge and IR is done via a trigger so we don't waste cpu cycles when no data is coming in.    
   Because data reading happens inside a pin interrupt we must keep the processing time as short as possible. Most non critical calculations like CRC validation and updating the blaster state happens in the main "thread".

5) Team and trigger switches.
   Since most time sensitive things happen in interrupts we can have a short event loop (arduino loop function) that polls the state of the trigger and team switch.

   
